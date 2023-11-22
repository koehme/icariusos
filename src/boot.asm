ORG 0x7c00
BITS 16

start:
    mov si, message
    call print
    jmp $

print:
    mov bx, 0x0
.loop:
    lodsb
    cmp al, 0x0
    je .done
    call .print_ch
    jmp .loop
.done
    ret
.print_ch
    mov ah, 0x0e
    int 0x10
    ret

message: db "Hello World!", 0x0

times 510-($ - $$) db 0x0
dw 0xaa55