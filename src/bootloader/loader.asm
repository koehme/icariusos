[BITS 32]
global _start
CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x00200000
    mov esp, ebp
; Enable the fast a20 line, so that all memory can be accessed
.enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

    jmp $