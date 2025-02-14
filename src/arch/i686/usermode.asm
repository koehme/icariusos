global asm_usermode_entrypoint

section .text

; void asm_usermode_entrypoint(void);
asm_usermode_entrypoint:
    mov eax, 0x1 ; Set Syscall number to SYS_EXIT (1)
    int 0x80     ; Invoke Syscall
    hlt          ; Halt CPU if execution unexpectedly returns