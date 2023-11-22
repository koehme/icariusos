ORG 0                           ; Allows manual setting of segments to 0x7C00 later, addressing potential BIOS discrepancies in segment values during boot loading.
BITS 16

; Initializes the environment for the bootloader.
; configuring segment registers and setting up the stack
init:
    cli                         ; Disable interrupts (cli) to ensure a stable environment during critical initialization,
                                ; preventing interruptions that might interfere with the setup process
    mov ax, 0x7c0               ; Set up data segment (ds) and extra segment (es)
    mov ds, ax
    mov es, ax
    mov ax, 0x00                ; Set the stack segment (ss) to 0x00
                                ; to initialize the stack at the bottom of the addressable memory space in the x86 real mode

    mov ss, ax
    mov sp, 0x7c00              ; Set the stack pointer (sp) to 0x7C00, which is the end of the bootloader code,
                                ; to allow the stack to grow downward from this location in the x86 real mode
    sti                         ; Enable interrupts

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