[BITS 32]
section .asm

global asm_page_load

asm_page_load:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    pop ebp
    ret