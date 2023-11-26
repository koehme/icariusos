[BITS 32]

global _start
extern kmain

CODE_SEG equ 0x08
DATA_SEG equ 0x10

;=============================================================================
; Start Initialization
;
; This section sets up segment registers, initializes the stack pointer and 
; enables the fast A20 line for accessing all memory.
;
; @param None
;
; @return None
;=============================================================================
_start:
    mov ax, DATA_SEG            ; Set up data segment registers
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x00200000         ; Set base pointer (ebp) and stack pointer (esp)
    mov esp, ebp
    ; Enable the fast a20 line, so that all memory can be accessed
.enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
.run_kernel:
    call kmain
    jmp $                       ; Infinite loop to halt execution at this point

times 512 - ($ - $$) db 0x0