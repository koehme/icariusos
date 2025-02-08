global asm_enter_usermode

; ------------------------------------------------
; void asm_enter_usermode(task_registers_t* regs)
; rstores the task's registers and performs an IRETD to switch from kernel mode to user mode
asm_enter_usermode:
    mov ebp, esp
    mov ebx, [ebp + 4]    ; Load pointer to regs into EBX (regs = &task->registers)

    ; Push user mode stack setup (for IRETD)
    push dword [ebx + 44] ; Push user mode SS (Stack Segment)
    push dword [ebx + 40] ; Push user mode ESP (Stack Pointer)

    mov eax, [ebx + 36]   ; Load EFLAGS from regs->eflags
    or eax, 0x200         ; Set Interrupt Flag (IF) to enable interrupts in user mode
    push eax              ; Push modified EFLAGS

    push dword [ebx + 32] ; Push user mode CS (Code Segment)
    push dword [ebx + 28] ; Push user mode entry point (EIP)

    ; Set up segment registers for user mode
    mov ax, [ebx + 44]    ; Load user mode SS (Stack Segment)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore general-purpose registers
    push dword [ebp + 4]  ; Push pointer to regs onto the stack
    call asm_task_restore_register
    add esp, 4            ; Clean up the stack after function call

    iretd                 ; Perform an interrupt return to switch to user mode

; ------------------------------------------------
; void asm_task_restore_register(task_registers_t* regs)
; restores all general-purpose registers from a saved task context.
asm_task_restore_register:
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8]    ; Load pointer to task_registers_t

    mov edi, [ebx]        ; Restore EDI
    mov esi, [ebx + 4]    ; Restore ESI
    mov ebp, [ebx + 8]    ; Restore EBP
    mov edx, [ebx + 16]   ; Restore EDX
    mov ecx, [ebx + 20]   ; Restore ECX
    mov eax, [ebx + 24]   ; Restore EAX
    mov ebx, [ebx + 12]   ; Restore EBX

    pop ebp
    ret