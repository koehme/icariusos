global asm_outb
global asm_outw
global asm_outl

global asm_inb
global asm_inw
global asm_inl

;=============================================================================
; Reads a byte from a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
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
; Reads a word (16 bits) from a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
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
; Reads a dword (32 bits) from a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
;=============================================================================
asm_inl:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    xor eax, eax             ; Clear EAX to store the result, in x86 assembly eax is implicit the return value
    mov edx, [ebp+8]         ; Copy the 16 bit address to receiving data from (first argument +8) into EDX

    in eax, dx                ; Read a word (32 bits) from the specified I/O port into AX

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret

;=============================================================================
; Sends a byte (8 bits) value to a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
; @param [ebp+12]: The value to be sent (8 bits)
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
; Sends a word (16 bits) value to a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
; @param [ebp+12]: The value to be sent (16 bits)
;=============================================================================
asm_outw:
    push ebp                 ; Save the base pointer
    mov ebp, esp             ; Get the actual stack frame for outb

    mov eax, [ebp+12]        ; Copy the 16 bit value (second argument +12) into EAX
    mov edx, [ebp+8]         ; Copy the 16 bit address to send (first argument +8) into EDX

    out dx, ax               ; Send the value in AX (16 bit) to the specified I/O port in DX

    pop ebp                  ; Restore the original base pointer so that it is not lost
    ret

;=============================================================================
; Sends a dword (32 bits) value to a specified I/O port
; @param [ebp+8]: The address of the I/O port (16 bits)
; @param [ebp+12]: The value to be sent (32 bits)
;=============================================================================
asm_outl:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12]        ; Copy the 32 bit value (second argument +12) into EAX
    mov edx, [ebp+8]         ; Copy the 16 bit address to send (first argument +8) into EDX

    out dx, eax             ; Send the value in EAX (32 bit) to the specified I/O port in EDX

    pop ebp
    ret