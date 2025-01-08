#include "tss.h"
#include "string.h"

tss_t tss = {
    .link = 0x0,
};

/* PUBLIC API */
void tss_init(tss_t* self, uint32_t esp0, uint16_t ss0);
void tss_load(uint16_t tss_selector);

void tss_init(tss_t* self, uint32_t esp0, uint16_t ss0)
{
	memset(self, 0, sizeof(tss_t));
	self->esp0 = esp0;
	self->ss0 = ss0;
	return;
};

void tss_load(uint16_t tss_selector)
{
	asm volatile("ltr %0" : : "r"(tss_selector));
	return;
};