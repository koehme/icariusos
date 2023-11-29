section .asm

global idt_loader

;=============================================================================
; idt_loader
;
; Responsible for loading the Interrupt Descriptor Table Register (IDTR).
; It takes a single argument, a pointer to the Interrupt Descriptor Table (IDTR),
; and loads it into the IDT register. The IDT contains entries for interrupt
; service routines (ISRs) that handle various hardware and software interrupts.
;
; @param ebx: A pointer to the IDT_R.
;
; @return None
;=============================================================================
idt_loader:
    push ebp              ; Save old base pointer

    mov ebp, esp          ; Use the current stack pointer 'idt_loader' as new base pointer for the caller frame (return address +4, argument 1 = +8, argument n = +4)
    ; Argument N
    ; ...
    ; Argument 2
    ; Argument 1
    ; Return address <--- (%esp)
    mov ebx, [ebp+8]      ; Access the argument 1 - IDT_R *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 

    pop ebp               ; Restore old base pointer
    
    ret