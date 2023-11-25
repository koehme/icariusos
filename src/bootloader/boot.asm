ORG 0x7c00
BITS 16

;=============================================================================
; Constants for GDT Segment Offsets
;
; These constants represent the offsets of the code and data segments in the 
; Global Descriptor Table (GDT).
;=============================================================================
CODE_SEG equ gdt_code - gdt_head
DATA_SEG equ gdt_data - gdt_head

;=============================================================================
; BIOS Parameter Block (BPB)
;
; Some BIOS systems expect the bios parameter block to be present. The inclusion 
; of this block helps prevent damage to our code during the boot process. It 
; contains a jump instruction to the start code, a no-operation instruction as 
; a placeholder, and additional space filled with null bytes.
;
; @param None
;
; @return None
;=============================================================================
bpb:
    jmp short start  ; Jump to the start code after the BPB
    nop              ; No-operation instruction as a placeholder
    times 33 db 0    ; Additional space or padding, filled with null bytes

;=============================================================================
; Start
;
; Jump to the initialization routine to begin the bootloader setup process.
;
; @param None
;
; @return None
;=============================================================================
start:
    jmp 0:init_bootloader

;=============================================================================
; Initialize Bootloader
;
; This function initializes the bootloader environment by disabling interrupts 
; (cli) to ensure a stable setup during critical initialization. It sets up 
; the data segment (ds) and extra segment (es), configures the stack pointer (sp)
; to the end of the bootloader code (0x7C00), allowing the stack to grow 
; downward in x86 real mode. Finally, it enables interrupts (sti).
;
; @param None
;
; @return None
;=============================================================================
init_bootloader:
    cli                         ; Disable interrupts (cli) to ensure a stable environment during critical initialization,
                                ; preventing interruptions that might interfere with the setup process
    mov ax, 0x00                ; Set up data segment (ds) and extra segment (es)
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00              ; Set the stack pointer (sp) to 0x7C00, which is the end of the bootloader code,
                                ; to allow the stack to grow downward from this location in the x86 real mode
    sti                         ; Enable interrupts

;=============================================================================
; Load Protected Mode
;
; This function is responsible for transitioning the processor into Protected
; Mode by disabling interrupts, loading the Global Descriptor Table (GDT), 
; and setting the necessary control register (CR0) bits. It ensures a smooth
; transition to a more secure and feature-rich operating mode.
;
; @param None
;
; @return None
;=============================================================================
    cli                         ; Disable interrupts to avoid unwanted interruptions
    lgdt [gdt_descriptor]       ; Load the Global Descriptor Table (GDT)
    mov eax, cr0                ; Load the current value of control register CR0 into the eax register
    or eax, 0x1                 ; Set the first bit of CR0 to activate protected mode
    mov cr0, eax                ; Write the modified value back to CR0

[BITS 32]
load_32:
    mov eax, 1                  ; First sector after bootloader (LBA)
    mov ecx, N_SECTORS_TO_READ  ; Read 100 sectors for now
    mov edi, KERNEL_START       ; Address to load the kernel to

    call ata_lba_read
    jmp CODE_SEG:KERNEL_START

;=============================================================================
; ATA read sectors (LBA mode) 
;
; @param ECX Number of sectors to read
; @param EBX LBA (Logical Block Address) value
;
; @return EDI The address of buffer to put data obtained from disk
;=============================================================================
ata_lba_read:
    call .ata_lba_send
    ret

;=============================================================================
; ATA LBA Send
;
; Sends the necessary commands to the ATA controller to specify the number of
; sectors to read and the corresponding Logical Block Address (LBA).
;
; @param ECX Number of sectors to read
; @param EBX LBA (Logical Block Address) value
;
; @return None
;=============================================================================
.ata_lba_send:
    ; Send the highest 8 bits of the lba
    mov ebx, eax                            ; Backup the LBA it represents the first sector for now
    shr eax, 24                             ; Move the highest 8 bits to the lowest 8 bits
    or eax, 0xe0                            ; Set bit 6 in AL for LBA mode
    mov dx, ATA_DRIVE_HEAD_PORT             ; I/O port address for the control register of the ATA controller
    out dx, al                              ; Send the 8 bits to the LBA controller#

    ; Send the total sectors to read
    mov eax, ecx                            ; The total sectors are hold in ECX (N_SECTORS_TO_READ)
    mov dx, ATA_SECTOR_COUNT_PORT           ; Address to send the number of sectors to read
    out dx, al                              ; Send the lower 8 Bits to the LBA controller

    ; Send the lowest 8 bits of the lba
    mov edx, ATA_LBA_LOW_PORT               ; Port to send bit 0 - 7 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    out dx, al

    ; Send the middle 8 bits of the lba
    mov edx, ATA_LBA_MID_PORT               ; Port to send bit 8 - 15 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    shr eax, 8                              ; Get bit 8 - 15 in AL
    out dx, al

    ; Send the highest 8 bits of the lba
    mov edx, ATA_LBA_HIGH_PORT              ; Port to send bit 16 - 23 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    shr eax, 16                             ; Get bit 16 - 23 in AL
    out dx, al
 
    mov edx, ATA_STATUS_COMMAND_PORT        ; Command port
    mov al, 0x20                            ; Read with retry
    out dx, al

.ata_read_next_sector:
    push ecx                                ; Store the current value of ECX on the stack so that we can decrement it with the loop below

.ata_check_is_readable:
    ; Check if the ATA controller is ready
    mov dx, ATA_STATUS_COMMAND_PORT         ; Load the command port address into DX
    in al, dx                               ; Read a byte from the command port into AL
    test al, 0b00001000                     ; Check if after bitwise and with AL and the 4 bit is true so the device is ready. 
    jz .ata_check_is_readable               ; If this bit is not set the device is not ready and we have to wait. ATA controller is not ready yet, so jump back
    ; Read data from the ATA data port
    mov ecx, 256                            ; 256 times rep insw like 512 byte or 1 sector (256 * 16 bits = 512 bytes)
    mov dx, ATA_DATA_PORT                   ; Put the data port in DX
    rep insw                                ; Reads a word (16bit) from an io port like DX and put it into a buffer ES:DI
    pop ecx                                 ; Restore the original value of ECX from the stack
    loop .ata_read_next_sector              ; Decrement ECX and jump to .ata_read_next_sector if ECX is not zero
    ret

N_SECTORS_TO_READ       equ 100             ; N sectors to read (Makefile)
ATA_DATA_PORT           equ 0x1F0           ; Data Port
ATA_ERROR_PORT          equ 0x1F1           ; Error Register
ATA_SECTOR_COUNT_PORT   equ 0x1F2           ; Sector Count Register
ATA_LBA_LOW_PORT        equ 0x1F3           ; LBA Low Register
ATA_LBA_MID_PORT        equ 0x1F4           ; LBA Mid Register
ATA_LBA_HIGH_PORT       equ 0x1F5           ; LBA High Register
ATA_DRIVE_HEAD_PORT     equ 0x1F6           ; Drive/Head Register
ATA_STATUS_COMMAND_PORT equ 0x1F7           ; Command/Status Register

KERNEL_START            equ 0x010000        ; Start of kernel memory address

;=============================================================================
; The GDT contains entries telling the CPU about memory segments
;
; @return None
;=============================================================================
gdt_head:
    gdt_null: 
            dq 0x0              ; Acts as a placeholder and validation point
    gdt_code:
            dw 0xffff           ; Size of the segment - 1 (either in bytes or in 4 KiB units - see flags)
            dw 0x0              ; The address where the segment starts bits 0 - 15
            db 0x0              ; The address where the segment starts bits 16 - 23
            db 0xb10011010      ; Access information (ring, executable, etc.)
            db 0b11001111       ; Defines the segment size unit and 16/32 bit.
            db 0x0              ; The address where the segment starts bits 24 - 31
    gdt_data:
            dw 0xffff           ; Size of the segment - 1 (either in bytes or in 4 KiB units - see flags)
            dw 0x0              ; The address where the segment starts bits 0 - 15
            db 0x0              ; The address where the segment starts bits 16 - 23
            db 0b10010010       ; Access information (ring, executable, etc.)
            db 0b11001111       ; Defines the segment size unit and 16/32 bit.
            db 0x0              ; The address where the segment starts bits 24 - 31
    gdt_tail:

;=============================================================================
; GDT Descriptor
;
; This section defines the GDT descriptor, specifying the length of the GDT
; and the starting address.
;=============================================================================
gdt_descriptor:
    dw gdt_tail - gdt_head - 1  ; 16 bit
    dd gdt_head                 ; 32 bit

;=============================================================================
; Fill Remaining Boot Sector Bytes and Add Magic Signature
;
; Fill the remaining bytes of the boot sector with zeros up to byte 510.
; Then add the magic BIOS signature 0xaa55 at the end to indicate a valid 
; bootable sector.
;=============================================================================
times 510 - ($ - $$) db 0x0
dw 0xaa55