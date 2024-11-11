#include "pic.h"

#include "io.h"

static void pic_remap1()
{
	// Send ICW1 to Master PIC
	outb(PIC_1_CTRL, ICW_1);
	// Send ICW2: Remap IRQs 0-7 to 0x20-0x27
	outb(PIC_1_DATA, IRQ_0);
	// Send ICW3: Cascade setup (PIC2 connected to IRQ2 of PIC1)
	outb(PIC_1_DATA, 0x04);
	// Send ICW4: Set 8086 mode
	outb(PIC_1_DATA, MODE_8086);
	return;
};

static void pic_remap2()
{
	// Send ICW1 to Slave PIC
	outb(PIC_2_CTRL, ICW_1);
	// Send ICW2: Remap IRQs 8-15 to 0x28-0x2F
	outb(PIC_2_DATA, IRQ_8);
	// Send ICW3: Slave PIC is connected to IRQ2 of Master PIC
	outb(PIC_2_DATA, 0x02);
	// Send ICW4: Set 8086 mode
	outb(PIC_2_DATA, MODE_8086);
	return;
};

void pic_init()
{
	pic_remap1();
	pic_remap2();
	return;
};