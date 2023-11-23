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
    jmp 0x7c0:init_bootloader

; Configuring segment registers and setting up the stack
init_bootloader:
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

    call .read_from_disk

; Read data from the disk
.read_from_disk:
    call setup_disk_read

    int 0x13                    ; Invoke the BIOS interrupt to read from the disk
    jc .error_read_from_disk    ; Jump to error handling if the carry flag is set (indicates an error)

    mov si, buffer
    call print
    jmp $                       ; Continue execution if the disk read was successful

.error_read_from_disk:
    mov si, message             ; Load the address of the error message string into SI
    call print                  ; Call the print routine to display the error message
    jmp $                       ; Halt execution after displaying the error message

; Set up disk read routine
setup_disk_read:
    mov ah, 0x2                 ; BIOS disk read routine
    mov al, 0x1                 ; Number of sectors to read (1 sector)
    mov ch, 0x0                 ; Cylinder number
    mov cl, 0x2                 ; Starting sector number (sector 2)
    mov dh, 0x0                 ; Head number
    mov bx, buffer              ; Memory buffer where data will be loaded (ES:BX)
    ret                         ; Return from the function

; It prints a null-terminated string pointed to by SI
print:
    mov bx, 0x0
.get_next_ch:
    lodsb                       ; Load the byte at the address in SI into AL and increment SI

    cmp al, 0x0                 ; Compare the loaded byte to the null terminator (end of the string)
    je .print_done              ; If null terminator is found, exit the loop

    call .print_ch              
    jmp .get_next_ch            ; Jump back to the start of the loop to process the next character
.print_done
    ret
.print_ch
    mov ah, 0x0e
    int 0x10
    ret

message: 
    db 'Unable to read from the disk. Check the disk and try again.', 0      

times 510 - ($ - $$) db 0x0     ; Fill up the remaining bytes minus the magic signature 0xaa55 with 0
dw 0xaa55                       ; magic bios signature

; Reserving a buffer of 512 bytes for data loaded from hard disk sector 2
buffer: 
    resb 0x200 - ($ - $$) ; Placeholder label until address 0x200 (512 bytes for boot sector) - (current '$' position 0x200 - entry position '$$' is 0x200)
