extern isr_0h_handler
extern isr_14h_handler
extern isr_20h_handler
extern isr_21h_handler
extern isr_32h_handler

extern isr_default_handler
extern isr_error_handler

global asm_do_nop
global asm_do_sti
global asm_do_cli

global asm_interrupt_0h
global asm_interrupt_14h
global asm_interrupt_20h
global asm_interrupt_21h
global asm_interrupt_32h
global asm_idt_loader

global asm_interrupt_default
global asm_interrupt_error

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
; @param ebx: A pointer to the idtr_t.
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
    mov ebx, [ebp+8]      ; Access the argument 1 - idtr_t *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 

    pop ebp               ; Restore old base pointer
    
    ret

;=============================================================================
; asm_interrupt_0h
;
; Handles the Division by Zero exception (Interrupt Vector 0).
; This interrupt service routine (ISR) stub disables interrupts, saves the CPU
; state, sets the appropriate data segments, and passes a pointer to the saved
; register state to the C-level exception handler (isr_0h_handler). After the
; handler returns, the original CPU state is restored and control resumes via iret.
;
; @param None            (The CPU state is stored on the stack)
; @return None
;=============================================================================
asm_interrupt_0h:
    cli                           ; Disable interrupts to prevent nested exceptions

    push eax                      ; Save EAX
    push ecx                      ; Save ECX
    push edx                      ; Save EDX
    push ebx                      ; Save EBX

    mov eax, esp
    add eax, 16                   ; Already pushed 4x4 = 16 bytes also we need the prev esp 

    push eax                      ; Save original ESP before modifications
    push ebp                      ; Save EBP
    push esi                      ; Save ESI
    push edi                      ; Save EDI
    ; Save segment registers
    push ds
    push es
    push fs
    push gs
    ; Load kernel data segment (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; Pass pointer to saved register state to the handler
    mov eax, esp
    push eax
    call isr_0h_handler           ; Call C-level exception handler
    add esp, 4                    ; Clean up function argument
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    ; Manually restore general-purpose registers (equivalent to popad)
    pop edi
    pop esi
    pop ebp
    add esp, 4                    ; Skip original ESP (not restored)
    pop ebx
    pop edx
    pop ecx
    pop eax

    sti                           ; Re-enable interrupts
    iret                          ; Return from interrupt (restores EIP, CS, and EFLAGS)

;=============================================================================
; asm_interrupt_14h
;
; Page fault handler for interrupt 14h 
;
; @param None
;
; @return None
;=============================================================================
asm_interrupt_14h:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    mov eax, [esp+32]
    
    call isr_14h_handler          ; Call C interrupt service routine handler for interrupt 14  

    add esp, 4
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

    call isr_20h_handler          ; Call C interrupt service routine handler for interrupt 20

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
    
    call isr_21h_handler          ; Call C interrupt service routine handler for interrupt 21

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_interrupt_32h
;
; Custom handler for interrupt 32h 
;
; @param None
;
; @return None
;=============================================================================
asm_interrupt_32h:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    
    call isr_32h_handler          ; Call C interrupt service routine handler for interrupt 32  

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
    
    call isr_default_handler      ; Call a C interrupt service routine handler for interrupts that are not yet implemented

    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt