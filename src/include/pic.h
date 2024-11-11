#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define ICW_1 0x11     // Initialize PIC and expect ICW4
#define MODE_8086 0x01 // 8086/88 (MCS-80/85) mode

#define PIC_1_CTRL 0x20 // Master PIC control port
#define PIC_1_DATA 0x21 // Master PIC data port

#define PIC_2_CTRL 0xA0 // Slave PIC control port
#define PIC_2_DATA 0xA1 // Slave PIC data port

#define IRQ_0 0x20 // Offset for IRQs 0-7 (Master PIC)
#define IRQ_8 0x28 // Offset for IRQs 8-15 (Slave PIC)

void pic_init();

#endif