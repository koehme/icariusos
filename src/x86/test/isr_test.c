/**
 * @file isr_test.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "isr_test.h"

void test_isr_0(void)
{
	kprintf("[TEST] Triggering Division by Zero Exception (#DE)...\n");
	int a = 5;
	int b = 0;
	int c = a / b;
	kprintf("%d\n", c);
	return;
};

void test_isr_1(void)
{
	kprintf("[TEST] Triggering Debug Exception (#DB)...\n");
	asm volatile("int $1");
	return;
};

void test_isr_2(void)
{
	kprintf("[TEST] Triggering Non-Maskable Interrupt (NMI)...\n");
	asm volatile("int $2");
	return;
};

void test_isr_6(void)
{
	kprintf("[TEST] Triggering Invalid Opcode Exception (#UD)...\n");
	asm volatile(".byte 0x0F, 0xFF"); // Invalid opcode to force #UD
	return;
};

void test_isr_8(void)
{
	kprintf("[TEST] Triggering Double Fault Exception (#DF)...\n");
	asm volatile("cli\n" // Disable interrupts
		     "mov %esp, %eax\n"
		     "mov %eax, %ss\n"	// Load an invalid stack segment (triggers #GP)
		     "mov $0x0, %esp\n" // Set ESP to 0 (causes stack overflow)
		     "pushl $0\n"	// Force a second fault (stack push fails)
		     "pushl $0\n"	// This will cause the double fault (#DF)
	);
	return;
};

void test_isr_12(void)
{
	kprintf("[TEST] Triggering Stack-Segment Fault (#SS)...\n");
	asm volatile("mov $0x23, %ax\n" // Try to load an invalid segment selector
		     "mov %ax, %ss\n"	// This will trigger a Stack-Segment Fault
	);
	return;
};

void test_isr_13(void)
{
	kprintf("[TEST] Triggering General Protection Fault (#GP)...\n");
	asm volatile("mov $0x42, %ax\n"
		     "mov %ax, %ds\n");
	return;
};

void test_isr_14(void)
{
	kprintf("[TEST] Triggering Page Fault (#PF)...\n");
	volatile char* ptr = (volatile char*)0xE0400000; // Page Dir => 897
	*ptr = 'A';
	return;
};