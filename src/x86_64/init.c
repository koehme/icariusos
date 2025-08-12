/**
 * @file init.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "init.h"
#include "kerr.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t _init_limine(void);

/* INTERNAL API */
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

kresult_t _init_limine(void)
{
	if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1) {
		return kres_err(-K_ENODEV, NULL);
	};
	struct limine_framebuffer* fb = fb_request.response->framebuffers[0];

	if (fb->bpp != 32) {
		return kres_err(-K_EINVAL, NULL);
	};
	fb_setup(&(fb_info_t){
	    .vaddr = fb->address,
	    .width = fb->width,
	    .height = fb->height,
	    .pitch = fb->pitch,
	    .bpp = fb->bpp,
	    .fmt =
		{
		    .r_shift = fb->red_mask_shift,
		    .g_shift = fb->green_mask_shift,
		    .b_shift = fb->blue_mask_shift,
		    .a_shift = 0,
		    .r_size = fb->red_mask_size,
		    .g_size = fb->green_mask_size,
		    .b_size = fb->blue_mask_size,
		    .a_size = 0,
		},
	});
	return kres_ok(NULL);
};