ORG 0                           ; Allows manual setting of segments to 0x7C00 later,
                                ; addressing potential BIOS discrepancies in segment values during boot loading
BITS 16

; Some BIOS systems expect the bios parameter block to be present
; The inclusion of this block helps to prevent damage to our code during the boot process
bpb:
    jmp short start  ; Jump to the start code after the BPB
    nop              ; No-operation instruction as a placeholder
    times 33 db 0    ; Additional space or padding, filled with null bytes

; Initial Jump to direct boot loader execution to a predictable location
; for the CS segment and ensure a consistent environment for further boot code execution
start:
    jmp 0x7c0:init

isr_0:
    mov ah, 0x0e
    mov al, '0'
    int 0x10
    iret

; Initializes the environment for the bootloader.
; configuring segment registers and setting up the stack
init:
    cli                         ; Disable interrupts (cli) to ensure a stable environment during critical initialization,
                                ; preventing interruptions that might interfere with the setup process
    mov ax, 0x7c0               ; Set up data segment (ds) and extra segment (es)
    mov ds, ax
    mov es, ax
    mov ax, 0x00                ; Set the stack segment (SS) to 0x00
                                ; to initialize the stack at the bottom of the addressable memory space in the x86 real mode

    mov ss, ax
    mov sp, 0x7c00              ; Set the stack pointer (sp) to 0x7C00, which is the end of the bootloader code,
                                ; to allow the stack to grow downward from this location in the x86 real mode
    sti                         ; Enable interrupts

    mov ax, 0x00                ; Set the DS temporarily to the appropiate 0x00 starting address for the isr
    mov ds, ax                  ; Who our first interrupt service handler will be placed

    mov word[ds:0x00], isr_0    ; Set the first ISR to custom ISR isr_0 at address 0x0000:0x0000 (Instruction Pointer)
    mov word[ds:0x02], 0x7c0    ; Set the CS (Code Segment) to 0x7c0 because (0x7c0 << 4) + isr_0 or (0x7c0 * 16) + isr_0 = physical address of isr_0

    int 0                       ; call interrupt isr_0

    mov ax, 0x7c0               ; Restore the original DS value for correct data segment mapping
    mov ds, ax

; This is a print routine designed for testing purposes
; It prints a null-terminated string pointed to by SI
print_message:
    mov si, message
    call print
    jmp $

print:
    mov bx, 0x0
.loop:
    lodsb                       ; Load the byte at the address in SI into AL and increment SI
    cmp al, 0x0
    je .done                    ; If null terminator is found, exit the loop
    call .print_ch
    jmp .loop                   ; Jump back to the start of the loop
.done
    ret
.print_ch
    mov ah, 0x0e
    int 0x10
    ret

message: db "Hello World!", 0x0

times 510-($ - $$) db 0x0
dw 0xaa55