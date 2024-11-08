global asm_page_load
global asm_page_enable

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

;=============================================================================
; Enable the paging mechanism
; @param None
; @return None
;=============================================================================
asm_page_enable:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    pop ebp
    ret