/**
 * @file bootstrap.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "bootstrap.h"
#include "fb.h"
#include "hal.h"
#include "icarius.h"
#include "kres.h"
#include "limine.h"
#include "panic.h"
#include "pfa.h"
#include "string.h"
#include "types.h"

/* EXTERNAL API */
extern c8 kernel_start[];
extern c8 kernel_end[];

/* PUBLIC API */
void bootstrap_setup(void);

/* INTERNAL API */
typedef kresult_t (*bootstrap_fn_t)(void);
kresult_t _bootstrap_sse(void);
kresult_t _bootstrap_fb(void);
kresult_t _bootstrap_mm(void);

static bootstrap_fn_t _bootstrap_handlers[] = {
    _bootstrap_fb,
    _bootstrap_mm,
    _bootstrap_sse,
};

void bootstrap_setup(void)
{
	for (usize i = 0; i < sizeof(_bootstrap_handlers) / sizeof(_bootstrap_handlers[0]); i++) {
		const bootstrap_fn_t handler = _bootstrap_handlers[i];
		const kresult_t res = handler();

		if (res.code != K_OK)
			panic();
	};
};

/* INTERNAL API */
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

kresult_t _bootstrap_sse(void) { return asm_sse_setup(); };

kresult_t _bootstrap_fb(void)
{
	if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1)
		return kresult_err(-K_ENODEV, NULL);

	struct limine_framebuffer* l_fb = fb_request.response->framebuffers[0];

	if (l_fb->bpp != 32)
		return kresult_err(-K_EINVAL, NULL);

	const fb_boot_adapter_t fb_adapter = {
	    .vaddr = l_fb->address,
	    .width = l_fb->width,
	    .height = l_fb->height,
	    .pitch = l_fb->pitch,
	    .bpp = l_fb->bpp,
	    .fmt =
		{
		    .r_shift = l_fb->red_mask_shift,
		    .g_shift = l_fb->green_mask_shift,
		    .b_shift = l_fb->blue_mask_shift,
		    .a_shift = 0,
		    .r_size = l_fb->red_mask_size,
		    .g_size = l_fb->green_mask_size,
		    .b_size = l_fb->blue_mask_size,
		    .a_size = 0,
		},
	};
	fb_setup(&fb_adapter);
	return kresult_ok(NULL);
};

kresult_t _bootstrap_mm(void)
{
	const struct limine_memmap_response* mm = memmap_request.response;
	const struct limine_hhdm_response* hhdm = hhdm_request.response;

	if (!mm)
		return kresult_err(-K_ENODEV, "Missing memory map response");

	if (!hhdm)
		return kresult_err(-K_ENODEV, "No higher half direct map response");

	// Determine maximum physical address
	struct limine_memmap_entry** entries = mm->entries;
	uptr max_phys_addr = 0;

	for (usize i = 0; i < mm->entry_count; i++) {
		const struct limine_memmap_entry* entry = entries[i];
		const uptr end_phys_addr = entry->base + entry->length;

		switch (entry->type) {
		case LIMINE_MEMMAP_USABLE:
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			if (end_phys_addr > max_phys_addr)
				max_phys_addr = end_phys_addr;
			break;
		default:
			break;
		};
	};
	const uptr max_phys_addr_al = ALIGN_UP(max_phys_addr, PAGE_SIZE);
	const usize pmm_frames = max_phys_addr_al / PAGE_SIZE;
	const usize pmm_bytes = (pmm_frames + (8 - 1)) / 8;

	// Search for the smallest amount of memory to store the bitmap himself
	const u64 pmm_bytes_al = ALIGN_UP(pmm_bytes, PAGE_SIZE);
	u64 prev_avail = UINT64_MAX;
	uptr bitmap_phys_addr = 0;

	for (usize i = 0; i < mm->entry_count; i++) {
		const struct limine_memmap_entry* entry = entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE)
			continue;

		const uptr base_al = ALIGN_UP(entry->base, PAGE_SIZE);
		const uptr end_al = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);

		if (end_al <= base_al)
			continue;

		const usize avail = end_al - base_al;
		const b8 enough_bytes = avail >= pmm_bytes_al;
		const b8 smaller_chunk = avail < prev_avail;

		if (enough_bytes && smaller_chunk) {
			bitmap_phys_addr = base_al;
			prev_avail = avail;
		};
	};

	if (!bitmap_phys_addr)
		return kresult_err(-K_ENOMEM, "No suitable place for bitmap found");

	const u8* pmm_bitmap = (u8*)(hhdm->offset + bitmap_phys_addr);
	memset((u8*)pmm_bitmap, FRAME_USED, pmm_bytes_al);

	const pmm_boot_adapter_t pmm_adapter = {
	    .bitmap_size = pmm_bytes_al,
	    .hhdm_offset = hhdm->offset,
	    .start_addr = bitmap_phys_addr,
	    .total_frames = pmm_frames,
	};

	const kresult_t res = pfa_init(&pmm_adapter, pmm_bitmap);

	if (res.code != K_OK)
		return kresult_err(-K_ENOMEM, "Pfa allocation failed");

	// Mark USABLE/BOOTLOADER_RECLAIMABLE as FRAME_FREE
	for (usize i = 0; i < mm->entry_count; i++) {
		const struct limine_memmap_entry* entry = entries[i];

		switch (entry->type) {
		case LIMINE_MEMMAP_USABLE:
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			pfa_mark(entry->base, entry->length, FRAME_FREE);
			break;
		default:
		};
	};
	// Mark KERNEL as FRAME_USED
	for (usize i = 0; i < mm->entry_count; i++) {
		const struct limine_memmap_entry* entry = entries[i];

		switch (entry->type) {
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			pfa_mark(entry->base, entry->length, FRAME_USED);
			break;
		default:
		};
	};
	// Mark first 4096 Byte Frame as FRAME_USED
	pfa_mark(0x0, 0x1000, FRAME_USED);
	// Mark Bitmap as FRAME_USED
	pfa_mark(bitmap_phys_addr, pmm_bytes_al, FRAME_USED);
	return kresult_ok(NULL);
};