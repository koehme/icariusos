[BITS 32]

global _start
global simulate

extern kmain

CODE_SEG equ 0x08
DATA_SEG equ 0x10

;=============================================================================
; _start
;
; Entry point for the 32 bit protected mode. Sets up segment registers, initializes the
; stack pointer and enables the fast A20 line for accessing all memory.
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
    
.enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

;=============================================================================
; run_kernel
;
; Jump to the kernel at C function kmain
;
; @param None
;
; @return None
;=============================================================================
.run_kernel:
    call kmain
    jmp $                       ; Infinite loop to halt execution at this point

;=============================================================================
; simulate
;
; Test routine generates an interrupt and will be removed soon.
;
; @param None
;
; @return None
;=============================================================================
simulate:
    int 0

times 512 - ($ - $$) db 0x0