global asm_task_switch_to_userland

; void asm_task_switch_to_userland(task_registers_t* regs)
asm_task_switch_to_userland:
    mov ebp, esp
    mov ebx, [ebp + 4]    ; var ebx = &regs;

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

    ; restore stored registers
    push ebx ; ebx points to &regs
    call asm_task_restore_register
    add esp, 4

    iretd  ; leave kernelland and walk to userland :D

; void asm_task_restore_register(task_registers_t* regs);
asm_task_restore_register:
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8] ; Load pointer to task_registers_t

    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]

    add esp, 4
    ret