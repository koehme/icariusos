global _start

section .text
bits 32

%include "./src/arch/i686/gdt.asm"
%include "./src/arch/i686/pic.asm"

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

    pop eax
    pop ebx

    cli

.hang: 
    hlt
    jmp .hang

section .bss
    align 16        ; Align the beginning of the .bss section on a 16-byte boundary to ensures that stack_bottom starts at an address that is divisible evenly by 16

    stack_bottom:
        resb 65536   ; Reserve bytes for the stack
    stack_top: