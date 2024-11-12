ENTRY(_start) 

SECTIONS { 
	. = 0x00100000; 
	.multiboot ALIGN(4K) : 
	{   _multiboot_start = .;
        *(.multiboot)
        _multiboot_end = .;
	} 
	.boot ALIGN(4K) : 
	{
        _boot_start = .;
        *(.boot)
        _boot_end = .;
    }
	. += 0xC0000000; 
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000) 
	{ 
		_text_start = .;
        *(.text)
        _text_end = .; 
	} 
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000) 
	{
        _rodata_start = .;
        *(.rodata)
        _rodata_end = .;
	} 
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000) 
	{
		_data_start = .;
        *(.data)
        _data_end = .;
	} 
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000) 
	{         
		_bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
	} 
	_kernel_end = .; 
}