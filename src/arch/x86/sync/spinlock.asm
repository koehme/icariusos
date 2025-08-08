BITS 32

global asm_xchg

; uint32_t asm_xchg(volatile uint32_t* addr, const uint32_t new_val);
asm_xchg:
    push dword ebp
    mov dword ebp, esp

    mov eax, [ebp+8] ; addr -> eax
    mov edx, [ebp+12] ; new_val -> edx

    xchg edx, [eax] ; *addr = new_val

    mov eax, edx ; return old *addr

    pop ebp
    ret