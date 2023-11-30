[BITS 32]

global _start
global simulate

extern kmain

CODE_SEG equ 0x08
DATA_SEG equ 0x10

;=============================================================================
; _start
;
; Entry point for the 32 bit protected mode. Sets up segment registers and initializes the
; stack pointer.
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
;=============================================================================
; .enable_a20
;
; Enabling the A20 gate, which allows access to
; the full memory addressing capability of the x86 architecture. The A20 gate
; is a control line in the keyboard controller that, when enabled, allows the
; CPU to address memory beyond the 1st megabyte.
;
; The process involves reading the status register from the keyboard controller,
; setting the second bit (A20 bit), and then writing the modified status back.
;
; @param None
;
; @return None
;=============================================================================    
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

times 512 - ($ - $$) db 0x0