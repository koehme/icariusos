ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
   . = 0x0100000;

   .text : 
	{
		 . = ALIGN(8);
  		KEEP(*(.multiboot_header))
		*(.text)
	}
 
	.rodata : 
	{
		*(.rodata)
	}
 
	.data : 
	{
		*(.data)
	}
 
	.bss : 
	{
		*(COMMON)
		*(.bss)
	}
}