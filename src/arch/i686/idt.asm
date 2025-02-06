extern syscall_dispatch

extern isr_0_handler
extern isr_1_handler
extern isr_2_handler
extern isr_13_handler
extern isr_14h_handler
extern irq0_handler
extern irq1_handler
extern irq12_handler


extern isr_default_handler
extern isr_error_handler

global asm_do_nop
global asm_do_sti
global asm_do_cli

global asm_syscall
global asm_isr0_wrapper
global asm_isr1_wrapper
global asm_isr2_wrapper
global asm_isr13_wrapper
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

asm_syscall:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call syscall_dispatch

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

;=============================================================================
; asm_isr0_wrapper
; @param None
; @return None
;=============================================================================
asm_isr0_wrapper:
    cli

    pushad        

    push esp       
    push dword 0x0    

    call isr_0_handler
    add esp, 8      

    popad         

    sti
    iret
;=============================================================================
; asm_isr1_wrapper
; @param None
; @return None
;=============================================================================
asm_isr1_wrapper:
    cli

    pushad         

    push esp        
    push dword 0x1

    call isr_1_handler
    add esp, 8      

    popad         

    sti
    iret

;=============================================================================
; asm_isr2_wrapper
; @param None
; @return None
;=============================================================================
asm_isr2_wrapper:
    cli

    pushad         

    push esp        
    push dword 0x2

    call isr_2_handler
    add esp, 8      

    popad         

    sti
    iret

asm_isr13_wrapper:
    cli

    pushad         

    push esp        
    mov eax, [esp + 40]
    push eax

    call isr_13_handler
    add esp, 8      

    popad         

    sti
    iret

;=============================================================================
; asm_isr14_page_fault
; Page fault handler for interrupt 14h 
; @param None
; @return None
;=============================================================================
asm_isr14_page_fault:
    cli                        

    pushad                    
    
    mov eax, [esp+32]

    call isr_14h_handler         
    add esp, 4
    
    popad                        

    sti                       
    iret                      
    
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
    call isr_default_handler      

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