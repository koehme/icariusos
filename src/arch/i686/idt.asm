BITS 32

extern syscall_dispatch

extern isr_0_handler
extern isr_1_handler
extern isr_2_handler
extern isr_6_handler
extern isr_8_handler
extern isr_12_handler
extern isr_13_handler
extern isr_14_handler

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
global asm_isr6_wrapper
global asm_isr8_wrapper
global asm_isr12_wrapper
global asm_isr13_wrapper
global asm_isr14_wrapper
global asm_irq0_timer
global asm_irq1_keyboard
global asm_irq12_mouse
global asm_idt_loader

global asm_interrupt_default
global asm_interrupt_error

asm_do_nop:
    nop
    ret

asm_do_sti:
    sti
    ret

asm_do_cli:
    cli
    ret

asm_idt_loader:
    push dword ebp        ; Save old base pointer
    mov ebp, esp          ; Use the current stack pointer 'asm_idt_loader' as new base pointer for the caller frame (return address +4, argument 1 = +8, argument n = +4)
    mov ebx, [ebp+8]      ; Access the argument 1 - idtr_t *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 
    pop ebp               ; Restore old base pointer
    ret

asm_syscall:
    pushad      
    push dword esp
    push dword eax
    call syscall_dispatch
    add esp, 8
    popad
    iretd

asm_isr0_wrapper:
    cli
    pushad        
    push dword esp       
    push dword 0x0    
    call isr_0_handler
    add esp, 8      
    popad         
    sti
    iretd

asm_isr1_wrapper:
    cli
    pushad         
    push dword esp        
    push dword 0x1
    call isr_1_handler
    add esp, 8      
    popad         
    sti
    iretd

asm_isr2_wrapper:
    cli
    pushad         
    push dword esp        
    push dword 0x2
    call isr_2_handler
    add esp, 8      
    popad         
    sti
    iretd

asm_isr6_wrapper:
    cli
    pushad
    push dword esp
    push dword 0x6
    call isr_6_handler
    add esp, 8
    popad
    sti
    iretd

asm_isr8_wrapper:
    cli
    pushad
    push dword esp
    mov eax, [esp + 36]     ; Get error code
    push dword eax
    call isr_8_handler
    add esp, 8
    popad
    sti
    iretd

asm_isr12_wrapper:
    cli
    pushad
    push dword esp
    mov eax, [esp + 36]     ; Get error code
    push dword eax
    call isr_12_handler
    add esp, 8
    popad
    sti
    iretd

asm_isr13_wrapper:
    cli
    pushad         
    push dword esp        
    mov eax, [esp + 36]
    push dword eax
    ; void isr_13_handler(const uint32_t error_code, interrupt_frame_t* frame)
    call isr_13_handler
    add esp, 8      
    popad         
    sti
    iretd

asm_isr14_wrapper:
    cli                             ; Disable interrupts to prevent reentrancy
    pushad                          ; Save general-purpose registers: EAX, ECX, EDX, EBX, ESP (as placeholder), EBP, ESI, EDI
    push dword esp                  ; Pass current stack pointer as interrupt_frame_t* to handler
    mov eax, [esp + 36]             ; Retrieve CPU-pushed error code from stack (after pushad + push esp)
    push dword eax                  ; Push error code as second argument
    mov eax, cr2                    ; Read faulting virtual address from CR2
    push dword eax                  ; Push faulting address as first argument
    ; void isr_14_handler(uint32_t fault_addr, uint32_t error_code, interrupt_frame_t* frame)
    call isr_14_handler             
    add esp, 12                     ; Clean up pushed arguments: fault_addr, error_code, frame pointer
    popad                           ; Restore general-purpose registers
    add esp, 4                      ; Discard CPU-pushed error code (which wasn’t included in pushad)
    sti                             ; Re-enable interrupts
    iretd                           ; Return from interrupt (restores CS, EIP, EFLAGS, [optional ESP, SS])            
    
asm_irq0_timer:
    cli                             ; Disable interrupts to prevent nested interrupts
    pushad                        
    push dword esp
    call irq0_handler          
    add esp, 4
    popad                           ; Restore the saved state
    sti                             ; Enable interrupts
    iretd                           ; Return from interrupt

asm_irq1_keyboard:
    cli                             ; Disable interrupts to prevent nested interrupts
    pushad                        
    push dword esp
    call irq1_handler            
    add esp, 4 
    popad                           ; Restore the saved state
    sti                             ; Enable interrupts
    iretd                           ; Return from interrupt

asm_irq12_mouse:
    cli                             ; Disable interrupts to prevent nested interrupts
    pushad                        
    call irq12_handler         
    popad                           ; Restore the saved state
    sti                             ; Enable interrupts
    iretd                           ; Return from interrupt

asm_interrupt_default:
    cli
    pushad                          ; Save general-purpose registers
    push dword esp                  ; Pass frame pointer
    push dword 0xDEAD               ; Dummy interrupt number 
    ; void isr_default_handler(interrupt_frame_t* frame);
    call isr_default_handler
    add esp, 8                      ; Clean up arguments
    popad                           ; Restore registers
    sti
    iretd