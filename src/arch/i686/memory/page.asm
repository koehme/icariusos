global asm_page_load

;=============================================================================
; Load the given page directory into the CR3 register to switch to the new memory mapping
; @param uint32_t *dir - Pointer to the page directory to be loaded
; @return None
;=============================================================================
asm_page_load:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    pop ebp
    ret