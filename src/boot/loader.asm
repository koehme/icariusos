global _start

section .multiboot_header
align 8

header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; protected mode code
    dd header_end - header_start ; header length

    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

%include "./src/kernel/arch/i386/hal/gdt.asm"
%include "./src/kernel/arch/i386/hal/pic.asm"

section .text
_start:

    mov esp, stack_top

    push 0
    popf

    push ebx
    push eax

    cli
    lgdt [gdt_descriptor]
    call flush_gdt

    call remap_pic1
    call remap_pic2

.check_pm:
    mov eax, cr0    
    test eax, 1        
    jnz .run_kernel
    
.set_pm
    or eax, 1
    mov cr0, eax

.run_kernel:
    extern kmain
    call kmain

    cli

.hang: 
    hlt
    jmp .hang

section .bss
    align 16
    stack_bottom:
        resb 4096
    stack_top: