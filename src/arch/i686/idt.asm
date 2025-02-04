extern isr_0h_handler
extern isr_1h_handler
extern isr_14h_handler
extern irq0_handler
extern irq1_handler
extern irq12_handler

extern isr_default_handler
extern isr_error_handler

global asm_do_nop
global asm_do_sti
global asm_do_cli

global asm_isr0_divide_exception
global asm_isr1_debug_exception
global asm_isr14_page_fault
global asm_irq0_timer
global asm_irq1_keyboard
global asm_irq12_mouse
global asm_idt_loader

global asm_interrupt_default
global asm_interrupt_error

asm_do_nop:
    nop
    ret

;=============================================================================
; asm_do_sti
; Enable interrupts by clearing the Interrupt Flag (IF) in the EFLAGS register
; @param None
; @return None
;=============================================================================
asm_do_sti:
    sti
    ret

;=============================================================================
; asm_do_cli
; Disable interrupts by setting the Interrupt Flag (IF) in the EFLAGS register
; @param None
; @return None
;=============================================================================
asm_do_cli:
    cli
    ret

;=============================================================================
; asm_idt_loader
; Responsible for loading the Interrupt Descriptor Table Register (IDTR)
; @param ebx: A pointer to the idtr_t
; @return None
;=============================================================================
asm_idt_loader:
    push ebp              ; Save old base pointer
    mov ebp, esp          ; Use the current stack pointer 'asm_idt_loader' as new base pointer for the caller frame (return address +4, argument 1 = +8, argument n = +4)
    mov ebx, [ebp+8]      ; Access the argument 1 - idtr_t *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 
    pop ebp               ; Restore old base pointer
    ret

;=============================================================================
; asm_isr0_divide_exception
; @param None
; @return None
;=============================================================================
asm_isr0_divide_exception:
    cli                           ; Disable interrupts to prevent nested exceptions

    push eax                      ; Save EAX
    push ecx                      ; Save ECX
    push edx                      ; Save EDX
    push ebx                      ; Save EBX

    mov eax, esp
    add eax, 16                   ; Adjust ESP for previous stack frame

    push eax                      ; Save original ESP before modifications
    push ebp                      ; Save EBP
    push esi                      ; Save ESI
    push edi                      ; Save EDI

    ; Save segment registers
    push ds
    push es
    push fs
    push gs

    ; Load kernel data segment (0x10) into all segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pass pointer to saved register state to the C handler
    mov eax, esp
    push eax
    call isr_0h_handler           ; Call C-level exception handler

    add esp, 4                    ; Clean up function argument

    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers (equivalent to popad)
    pop edi
    pop esi
    pop ebp
    add esp, 4                    ; Skip original ESP (not restored)
    pop ebx
    pop edx
    pop ecx
    pop eax

    sti                           ; Re-enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_isr1_debug_exception
; @param None
; @return None
;=============================================================================
asm_isr1_debug_exception:
    cli                           ; Disable interrupts to prevent nested exceptions

    push eax                      ; Save EAX
    push ecx                      ; Save ECX
    push edx                      ; Save EDX
    push ebx                      ; Save EBX

    mov eax, esp
    add eax, 16                   ; Adjust ESP for previous stack frame

    push eax                      ; Save original ESP before modifications
    push ebp                      ; Save EBP
    push esi                      ; Save ESI
    push edi                      ; Save EDI

    ; Save segment registers
    push ds
    push es
    push fs
    push gs

    ; Load kernel data segment (0x10) into all segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pass pointer to saved register state to the C handler
    mov eax, esp
    push eax
    call isr_1h_handler           ; Call C-level exception handler

    add esp, 4                    ; Clean up function argument

    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers (equivalent to popad)
    pop edi
    pop esi
    pop ebp
    add esp, 4                    ; Skip original ESP (not restored)
    pop ebx
    pop edx
    pop ecx
    pop eax

    sti                           ; Re-enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_isr14_page_fault
; Page fault handler for interrupt 14h 
; @param None
; @return None
;=============================================================================
asm_isr14_page_fault:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        ; Saving general purpose registers, as this is an interrupt to avoid side effects
    mov eax, [esp+32]
    call isr_14h_handler          ; Call C interrupt service routine handler for interrupt 14  
    add esp, 4
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt
    
;=============================================================================
; asm_irq0_timer
; @param None
; @return None
;=============================================================================
asm_irq0_timer:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    call irq0_handler          
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_irq1_keyboard
; @param None
; @return None
;=============================================================================
asm_irq1_keyboard:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    call irq1_handler             
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; asm_irq12_mouse
; @param None
; @return None
;=============================================================================
asm_irq12_mouse:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    call irq12_handler         
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

;=============================================================================
; interrupt_default
; Default handler 
; @param None
; @return None
;=============================================================================
asm_interrupt_default:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    call isr_default_handler      
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt