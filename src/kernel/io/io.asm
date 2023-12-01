section asm

global asm_outb
global asm_outw

global asm_inb
global asm_inw

;=============================================================================
; asm_inb
;
; Reads a byte (8 bits) from a specified I/O port.
;
; This function reads a byte from the I/O port specified by the address in DX.
;
; @param [ebp+8]: The address of the I/O port (16 bits).
;
; @return The byte read from the specified I/O port (stored in EAX).
;=============================================================================
asm_inb:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    xor eax, eax             ; Clear EAX to store the result, in x86 assembly eax is implicit the return value
    mov edx, [ebp+8]         ; Copy the 16 bit address to receiving data from (first argument +8) into EDX

    in al, dx                ; Read a byte (8 bits) from the specified I/O port into AL

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret

;=============================================================================
; asm_inw
;
; Reads a word (16 bits) from a specified I/O port.
;
; This function reads a word from the I/O port specified by the address in DX.
;
; @param [ebp+8]: The address of the I/O port (16 bits).
;
; @return The word read from the specified I/O port (stored in EAX).
;=============================================================================
asm_inw:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    xor eax, eax             ; Clear EAX to store the result, in x86 assembly eax is implicit the return value
    mov edx, [ebp+8]         ; Copy the 16 bit address to receiving data from (first argument +8) into EDX

    in ax, dx                ; Read a word (16 bits) from the specified I/O port into AX

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret

;=============================================================================
; asm_outb
;
; Sends a byte value to a specified I/O port.
;
; Responsible for sending the value in AL to the I/O port specified by the
; address in DX. This is commonly used for low-level I/O operations,
; such as interacting with hardware devices.
;
; @param [ebp+8]: The address of the I/O port (16 bits).
; @param [ebp+12]: The value to be sent (8 bits).
;
; @return None
;=============================================================================
asm_outb:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    mov eax, [ebp+12]        ; Copy the 8 bit value (second argument +12) into EAX
    mov edx, [ebp+8]         ; Copy the 16 bit address to send (first argument +8) into EDX

    out dx, al               ; Send the value in AL to the specified I/O port in DX

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret

;=============================================================================
; asm_outw
;
; Sends a word (16 bits) value to a specified I/O port.
;
; Responsible for sending the 16-bit value in AX to the I/O port specified by the
; address in DX. This is commonly used for low-level I/O operations, such as
; interacting with hardware devices that expect 16-bit data.
;
; @param [ebp+8]: The address of the I/O port (16 bits).
; @param [ebp+12]: The value to be sent (16 bits).
;
; @return None
;=============================================================================
asm_outw:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    mov eax, [ebp+12]        ; Copy the 16 bit value (second argument +12) into EAX
    mov edx, [ebp+8]         ; Copy the 16 bit address to send (first argument +8) into EDX

    out dx, ax               ; Send the value in AX (16 bit) to the specified I/O port in DX

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret