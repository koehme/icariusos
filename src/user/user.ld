SECTIONS {
    . = 0x00000000;
    
    .text ALIGN(4M) : { 
        *(.text) 
    }

    . = 0x00400000; 
    .bss ALIGN(4M) : { 
        _bss_start = .; 
        *(.bss) 
        . = ALIGN(4M); 
        _bss_end = .; 
    }

    . = 0x00800000; 
    _heap_start = .;
}