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

;=============================================================================
; asm_page_enable
;
; Enable the Paging Mechanism
; This assembly code segment is dedicated to activating the paging mechanism, a pivotal step in advanced memory management.
; The Control Register 0 (CR0) is a system register that determines the operating mode of the processor.
; By loading the current value of CR0 into the EAX register, setting the Paging (PG) bit (0x80000001), and writing it back to CR0,
; we transition into a mode facilitating efficient memory translation through page tables.
; This fundamental operation enhances system memory organization, laying the foundation for sophisticated memory management strategies.
;
; @param None
;
; @return None
;=============================================================================
asm_page_enable:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000001  ; Set the Paging (PG) bit
    mov cr0, eax

    pop ebp
    ret