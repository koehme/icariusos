/**
 * @file tss.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2025-01-18
 * @see
 */

#include "tss.h"
#include "string.h"

tss_t tss = {
    .link = 0x0,
    .esp0 = 0x0, // Kernel Stack Pointer to load when changing to Kernel mode
    .ss0 = 0x0,	 // Kernel Stack Segment to load when changing to Kernel mode
    .esp1 = 0x0, // User Stack Pointer to load when changing to Ring 3 User mode
    .ss1 = 0x0,	 // User Stack Segment to load when changing to Ring 3 User mode
    .esp2 = 0x0,
    .ss2 = 0x0,
    .cr3 = 0x0,
    .eip = 0x0,
    .eflags = 0x0,
    .eax = 0x0,
    .ecx = 0x0,
    .edx = 0x0,
    .ebx = 0x0,
    .esp = 0x0,
    .ebp = 0x0,
    .esi = 0x0,
    .edi = 0x0,
    .es = 0x0,
    .cs = 0x0,
    .ss = 0x0,
    .ds = 0x0,
    .fs = 0x0,
    .gs = 0x0,
    .ldtr = 0x0,
    .iopb = 0x0,
};

/* PUBLIC API */
void tss_init(tss_t* self, uint32_t esp0, uint16_t ss0);
void tss_load(uint16_t tss_selector);

void tss_init(tss_t* self, uint32_t esp0, uint16_t ss0)
{
	// Ensure the TSS is initially zero'd
	memset(self, 0, sizeof(tss_t));
	// Set the kernel stack pointer
	self->esp0 = esp0;
	// Set the kernel stack segment
	self->ss0 = ss0;
	self->iopb = sizeof(tss_t);
	return;
};

void tss_load(uint16_t tss_selector)
{
	asm volatile("ltr %%ax" : : "a"(tss_selector));
	return;
};