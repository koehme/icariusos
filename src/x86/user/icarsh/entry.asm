global _icarsh    
extern main       

section .text
_icarsh:
    xor ebp, ebp
    xor eax, eax
    xor ebx, ebx
    call main

    mov eax, 0x1    
    xor ebx, ebx
    int 0x80
    hlt