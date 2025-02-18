global _start

section .text
_start:

asm_user_shell:
    ; Print shell prompt
    mov eax, 0x4         ; Syscall: SYS_WRITE (4)
    mov ebx, 0x1         ; STDOUT (1)
    mov ecx, msg         ; Pointer to the message
    mov edx, msg_len     ; Message length
    int 0x80             ; Invoke syscall

.loop:
    ; Read a single character from STDIN
    mov eax, 0x3         ; Syscall: SYS_READ (3)
    mov ebx, 0x0         ; STDIN (0)
    mov ecx, buf         ; Buffer to store input
    mov edx, 0x1         ; Read 1 character
    int 0x80             ; Invoke syscall

    ; Echo the character to STDOUT
    mov eax, 0x4         ; Syscall: SYS_WRITE (4)
    mov ebx, 0x1         ; STDOUT (1)
    mov ecx, buf         ; Pointer to the read character
    mov edx, 0x1         ; Length = 1 character
    int 0x80             ; Invoke syscall

    ; Check if 'q' (0x71) is pressed
    mov al, [buf]        ; Load character from buffer
    cmp al, 0x71         ; Compare with 'q'
    je .exit             ; If 'q' is pressed, exit

    jmp .loop            ; Otherwise, continue

.exit:
    ; Exit the user process
    mov eax, 0x1         ; Syscall: SYS_EXIT (1)
    xor ebx, ebx         ; Exit code 0
    int 0x80             ; Invoke syscall

section .data
    msg db "[icariusOS]:~# ", 0  ; Shell prompt
    msg_len equ $ - msg          ; Compute message length

section .bss
    buf resb 2  ; Reserve 2 byte for input character