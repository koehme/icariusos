[BITS 32]
section .asm

global asm_page_load
global asm_page_enable

asm_page_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov cr3, eax

    pop ebp
    ret

asm_page_enable:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    pop ebp
    ret