global asm_user_shell

section .text

; void asm_user_shell(void);
asm_user_shell:
    ; ssize_t sys_write(unsigned int fd, const char * buf, size_t count)
    mov eax, 0x4 ; Set SYSCALL to SYS_WRITE (4)
    mov ebx, 0x1 ; Set STDOUT (1)
    mov ecx, msg ; String 'msg'
    mov edx, msg_len ; Length of 'msg'

    int 0x80 ; Invoke Syscall

.loop:
    jmp .loop        ; Infinite loop

section .data
    msg db "[icariusOS]: Root@Kernel:~# ", 0 ; String 'msg'
    msg_len equ $ - msg ; Length of our string 'msg'