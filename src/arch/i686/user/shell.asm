global asm_user_shell

section .text

; void asm_user_shell(void);
asm_user_shell:
    mov eax, 0x1 ; Set Syscall number to SYS_EXIT (1)
    int 0x80     ; Invoke Syscall
    hlt          ; Halt CPU if execution unexpectedly return