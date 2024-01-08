%define ALIGN               (1 << 0)                    ; align loaded modules on page boundaries
%define MEMINFO             (1 << 1)                    ; provide memory map
%define FLAGS               (ALIGN | MEMINFO)           ; this is the Multiboot 'flag' field
%define MAGIC               0x1BADB002                  ; 'magic number' lets bootloader find the header
%define CHECKSUM            -(MAGIC + FLAGS)            ; checksum of above, to prove we are multiboot

section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
global _start

%include "./src/kernel/arch/i386/hal/gdt.asm"
%include "./src/kernel/arch/i386/hal/pic.asm"

extern kmain

_start:
    mov ebp, stack_top

    cli
    
    lgdt [gdt_descriptor]
    call flush_gdt

    call remap_pic1
    call remap_pic2

    push eax
    push ebx

    call kmain

hltlp: 
    hlt
    jmp hltlp

section .bss
    align 16
    stack_bottom:
        resb 16384
    stack_top: