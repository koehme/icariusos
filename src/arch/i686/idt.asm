extern irq_14h_handler
extern isr_20h_handler
extern isr_21h_handler
extern isr_default_handler

global asm_do_nop
global asm_do_sti
global asm_do_cli
global asm_irq_14h
global asm_interrupt_20h
global asm_interrupt_21h
global asm_idt_loader
global asm_interrupt_default

asm_do_nop:
    nop
    ret

;=============================================================================
; asm_do_sti
;
; Enable interrupts by clearing the Interrupt Flag (IF) in the EFLAGS register.
;
; @param None
;
; @return None
;=============================================================================
asm_do_sti:
    sti
    ret

;=============================================================================
; asm_do_cli
;
; Disable interrupts by setting the Interrupt Flag (IF) in the EFLAGS register.
;
; @param None
;
; @return None
;=============================================================================
asm_do_cli:
    cli
    ret

;=============================================================================
; asm_idt_loader
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
asm_idt_loader:
    push ebp              ; Save old base pointer

    mov ebp, esp          ; Use the current stack pointer 'asm_idt_loader' as new base pointer for the caller frame (return address +4, argument 1 = +8, argument n = +4)
    ; Argument N
    ; ...
    ; Argument 2
    ; Argument 1
    ; Return address <--- (%esp)
    mov ebx, [ebp+8]      ; Access the argument 1 - IDT_R *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 

    pop ebp               ; Restore old base pointer
    
    ret

;=============================================================================
; asm_irq_14h
;
; ATA master driver irq
;
; @param None
;
; @return None
;=============================================================================
asm_irq_14h:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    
    call irq_14h_handler          ; Call C interrupt service routine handler for interrupt 46  

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt
    
;=============================================================================
; asm_interrupt_20h
;
; Timer handler for interrupt 20h 
;
; @param None
;
; @return None
;=============================================================================
asm_interrupt_20h:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    
    call isr_20h_handler          ; Call C interrupt service routine handler for interrupt 20h  

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_interrupt_21h
;
; Custom handler for interrupt 21h 
;
; @param None
;
; @return None
;=============================================================================
asm_interrupt_21h:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    
    call isr_21h_handler          ; Call C interrupt service routine handler for interrupt 21h  

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; interrupt_default
;
; Default handler 
;
; @param None
;
; @return None
;=============================================================================
asm_interrupt_default:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    
    call isr_default_handler      ; Call a C interrupt service routine handler for interrupts that are not yet implemented. 
                                  ; Consider this as a dummy handler.

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt