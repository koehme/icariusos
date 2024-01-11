ENTRY(_start)

SECTIONS
{
    . = 1M;

    .rodata BLOCK(4K) : ALIGN(4K) 
	{
  		*(.multiboot_header)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}
 
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
 
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
 
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}