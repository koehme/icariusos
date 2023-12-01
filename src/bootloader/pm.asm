[BITS 32]

global _start
global simulate

extern kmain

CODE_SEG equ 0x08
DATA_SEG equ 0x10

%define ICW_1 00010001b				; Enables initialization mode and we are sending ICW 4
%define MODE_8086 0b00000001        ; 80x86 mode

%define PIC_1_CTRL 0x20				; Primary PIC control register
%define PIC_1_DATA 0x21				; Primary PIC data register

%define PIC_2_CTRL 0xA0				; Secondary PIC control register    
%define PIC_2_DATA 0xA1				; Secondary PIC data register
    
%define IRQ_0	0x20				; IRQs 0-7 mapped to use interrupts 0x20-0x27
%define IRQ_8	0x28				; IRQs 8-15 mapped to use interrupts 0x28-0x36

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
; run_kernel
;
; Jump to the kernel at C function kmain
;
; @param None
;
; @return None
;=============================================================================
.run_kernel:
    call enable_a20
    call remap_pic1
    call kmain
    jmp $                       ; Infinite loop to halt execution at this point

;=============================================================================
; enable_a20
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
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

;=============================================================================
; remap_pic1
;
; Initialize and configure the Programmable Interrupt Controller (PIC) 
; for handling hardware interrupts in 80x86 mode.
;
; @param None
;
; @return None
;=============================================================================
remap_pic1:
    ; Send ICW 1 --------------------------------------------------------
    mov al, ICW_1               ; Initialize Control Word 1
    out PIC_1_CTRL, al          ; Send ICW 1 to the control port of PIC1

    ; Remap PIC1 --------------------------------------------------------
    mov al, IRQ_0               ; IRQ_0 = 0x20
                                ; The interrupt vector offset is set to 32 (0x20)
                                ; to avoid conflicts with CPU-reserved interrupts.
    out PIC_1_DATA, al          ; Send it to the PIC1

    ; Send ICW 4 - Set x86 mode ----------------------------------------
    mov al, MODE_8086           ; Bit 0 enables 80x86 mode
    out PIC_1_DATA, al          ; Send it to the PIC1
    ret

times 512 - ($ - $$) db 0x0