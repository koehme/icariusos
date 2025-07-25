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
    push ebp              ; Save old base pointer
    mov ebp, esp          ; Use the current stack pointer 'asm_idt_loader' as new base pointer for the caller frame (return address +4, argument 1 = +8, argument n = +4)
    mov ebx, [ebp+8]      ; Access the argument 1 - idtr_t *ptr
    lidt [ebx]            ; Load interrupt descriptor table register 
    pop ebp               ; Restore old base pointer
    ret

asm_syscall:
    pushad      
    push esp
    push eax
    call syscall_dispatch
    add esp, 8
    popad
    iretd

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

asm_isr6_wrapper:
    cli
    pushad
    push esp
    push dword 0x6
    call isr_6_handler
    add esp, 8
    popad
    sti
    iret

asm_isr8_wrapper:
    cli
    pushad
    push esp
    mov eax, [esp + 36] ; Get error code
    push eax
    call isr_8_handler
    add esp, 8
    popad
    sti
    iret

asm_isr12_wrapper:
    cli
    pushad
    push esp
    mov eax, [esp + 36] ; Get error code
    push eax
    call isr_12_handler
    add esp, 8
    popad
    sti
    iret

asm_isr13_wrapper:
    cli
    pushad         
    push esp        
    mov eax, [esp + 36]
    push eax
    ; void isr_13_handler(const uint32_t error_code, interrupt_frame_t* regs)
    call isr_13_handler
    add esp, 8      
    popad         
    sti
    iret

asm_isr14_wrapper:
    cli                        
    pushad ; Push EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI (ecx at +4,+8,+12,+16,+20,+24,+28 .. )             
    push esp    ; interrupt_frame_t
    mov eax, [esp + 36] ; Errorcode at +36
    push eax
    mov eax, cr2  
    push eax
    ; void isr_14_handler(uint32_t fault_addr, uint32_t error_code, interrupt_frame_t* regs)
    call isr_14_handler         
    add esp, 16
    popad                        
    sti                       
    iret                      
    
asm_irq0_timer:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    push esp
    call irq0_handler          
    add esp, 4
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

asm_irq1_keyboard:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    push esp
    call irq1_handler            
    add esp, 4 
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

asm_irq12_mouse:
    cli                           ; Disable interrupts to prevent nested interrupts
    pushad                        
    call irq12_handler         
    popad                         ; Restore the saved state
    sti                           ; Enable interrupts
    iret                          ; Return from interrupt

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