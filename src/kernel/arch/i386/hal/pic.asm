%define ICW_1 00010001b				; Enables initialization mode and we are sending ICW 4
%define MODE_8086 0b00000001        ; 80x86 mode

%define PIC_1_CTRL 0x20				; Primary PIC control register
%define PIC_1_DATA 0x21				; Primary PIC data register

%define PIC_2_CTRL 0xA0				; Secondary PIC control register    
%define PIC_2_DATA 0xA1				; Secondary PIC data register
    
%define IRQ_0	0x20				; IRQs 0-7 mapped to use interrupts 0x20-0x27
%define IRQ_8	0x28				; IRQs 8-15 mapped to use interrupts 0x28-0x36

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

;=============================================================================
; remap_pic2
;
; Initialize and configure the Programmable Interrupt Controller (PIC) 
; for handling hardware interrupts in 80x86 mode.
;
; @param None
;
; @return None
;=============================================================================
remap_pic2:
    ; Send ICW 1 --------------------------------------------------------
    mov al, ICW_1               ; Initialize Control Word 1
    out PIC_2_CTRL, al          ; Send ICW 1 to the control port of PIC2

    ; Remap PIC2 --------------------------------------------------------
    mov al, IRQ_8               ; IRQ_8 corresponds to the mapping offset for PIC2
    out PIC_2_DATA, al          ; Send it to the PIC2

    ; Send ICW 2 - Set IRQ mapping offset for PIC2 ----------------------
    mov al, IRQ_8               ; Same offset as above
    out PIC_2_DATA, al          ; Send it to the PIC2

    ; Send ICW 3 - Set up the master/slave relationship ----------------
    mov al, 2                   ; Bit 1 (value 2) indicates that PIC2 is connected to IRQ2 of PIC1
    out PIC_2_DATA, al          ; Send it to the PIC2

    ; Send ICW 4 - Set x86 mode ----------------------------------------
    mov al, MODE_8086           ; Bit 0 enables 80x86 mode
    out PIC_2_DATA, al          ; Send it to the PIC2

    ret