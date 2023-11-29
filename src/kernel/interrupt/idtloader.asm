section .asm

global idt_loader

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