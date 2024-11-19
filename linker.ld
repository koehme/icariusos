ENTRY(_start)

SECTIONS
{
    /* LOAD KERNEL AT 1 MiB */
    . = 0x00100000;

    /* MULTIBOOT HEADER */
    .multiboot ALIGN(4K) :
    {
        _multiboot_start = .;  /* START OF MULTIBOOT HEADER */
        *(.multiboot)          /* MULTIBOOT CONTENT */
        _multiboot_end = .;    /* END OF MULTIBOOT HEADER */
    }

    /* BOOTLOADER-SPECIFIC SECTION */
    .boot ALIGN(4K) :
    {
        _boot_start = .;  /* START OF BOOTLOADER SECTION */
        *(.boot)          /* BOOTLOADER CONTENT */
        _boot_end = .;    /* END OF BOOTLOADER SECTION */
    }

    /* SWITCH TO VIRTUAL ADDRESSES (HIGHER HALF) */
    . += 0xC0000000;

    /* KERNEL CODE SECTION */
    .text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
    {
        _text_start = .;  /* START OF .TEXT */
        *(.text)          /* KERNEL CODE */
        _text_end = .;    /* END OF .TEXT */
    }

    /* READ-ONLY DATA SECTION */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
    {
        _rodata_start = .;  /* START OF .RODATA */
        *(.rodata)          /* READ-ONLY DATA */
        _rodata_end = .;    /* END OF .RODATA */
    }

    /* INITIALIZED DATA SECTION */
    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
    {
        _data_start = .;  /* START OF .DATA */
        *(.data)          /* INITIALIZED VARIABLES */
        _data_end = .;    /* END OF .DATA */
    }

    /* UNINITIALIZED DATA (BSS) */
    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
    {
        _bss_start = .;  /* START OF .BSS */
        *(COMMON)        /* COMMON SYMBOLS */
        *(.bss)          /* UNINITIALIZED VARIABLES */
        _bss_end = .;    /* END OF .BSS */
    }

    /* END OF KERNEL */
    _kernel_end = .;
}