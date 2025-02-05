global task_switch_to_userland

; void task_switch_to_userland(task_registers_t* regs)
task_switch_to_userland:
    mov ebx, [esp + 4]    ; var ebx = &regs;

    push dword [ebx + 44] ; put regs->ss onto stack for ring 3
    push dword [ebx + 40] ; put regs->esp onto stack for ring 3

    pushf ; push lower 16 bits of EFLAGS
    
    pop eax
    or eax, 0x200 ; activate interrupt flag bit 9 to activate interrupts in userland
    push eax ; put EFLAGS

    push dword [ebx + 32] ; put regs->cs onto stack for ring 3
    push dword [ebx + 28] ; put userland entrypoint, regs->eip onto stack for ring 3

    mov ax, [ebx + 44]  ; set userland segments
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; TODO restore registers

    iretd   ; jmp to x86 userland