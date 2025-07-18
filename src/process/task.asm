[BITS 32]
global asm_enter_task
global asm_restore_kernel_segment
global asm_restore_user_segment

; ------------------------------------------------
; void asm_enter_task(task_registers_t* regs)
; restores the task's registers and performs an IRETD to switching tasks
asm_enter_task:
    mov ebp, esp
    mov ebx, [ebp + 4]    ; Load pointer to regs into EBX (regs = &task->registers)

    ; Push stack setup (for IRETD)
    push dword [ebx + 44] ; Push SS (Stack Segment)
    push dword [ebx + 40] ; Push ESP (Stack Pointer)

    mov eax, [ebx + 36]   ; Load EFLAGS from regs->eflags
    or eax, 0x200         ; Set Interrupt Flag (IF) to enable interrupts
    push eax              ; Push modified EFLAGS

    push dword [ebx + 32] ; Push CS (Code Segment)
    push dword [ebx + 28] ; Push entry point (EIP)

    ; Setup segment registers
    mov ax, [ebx + 44]    ; Load SS (Stack Segment)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore general-purpose registers
    push dword [ebp + 4]  ; Push pointer to regs onto the stack
    call asm_task_restore_register
    add esp, 4            ; Clean up the stack after function call

    iretd                 ; Perform an interrupt return to switch to next task

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

asm_restore_kernel_segment:
    mov ax, 0x10    ; Kernel Data Segment (GDT_KERNEL_DATA_SEGMENT)
    mov ds, ax      ; Data Segment
    mov es, ax      ; Extra Segment
    mov fs, ax      ; FS Segment
    mov gs, ax      ; GS Segment
    ret

asm_restore_user_segment:
    mov ax, 0x23    ; USER_DS (Ring 3 Data-Segment)
    mov ds, ax      ; Data Segment
    mov es, ax      ; Extra Segment
    mov fs, ax      ; FS Segment
    mov gs, ax      ; GS Segment
    ret