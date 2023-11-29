ORG 0x7c00
BITS 16

;=============================================================================
; Constants for GDT Segment Offsets
;
; These constants represent the offsets of the code and data segments in the 
; Global Descriptor Table (GDT).
;=============================================================================
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

;=============================================================================
; bpb 
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
; start
;
; Jump to the initialization routine to begin the bootloader setup process.
;
; @param None
;
; @return None
;=============================================================================
start:
    jmp 0x0:init_bootloader

;=============================================================================
; init_bootloader
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
; load_protected
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
.load_protected:
    cli                         ; Disable interrupts to avoid unwanted interruptions
    lgdt [gdt_descriptor]       ; Load the Global Descriptor Table (GDT)

    mov eax, cr0                ; Load the current value of control register CR0 into the eax register
    or eax, 0x1                 ; Set the first bit of CR0 to activate protected mode
    mov cr0, eax                ; Write the modified value back to CR0

    jmp CODE_SEG:load_kernel   ; Jump to the entry point of the loaded kernel in protected mode

;=============================================================================
; The GDT contains entries telling the CPU about memory segments
;=============================================================================
gdt_start:
gdt_null: 
            dq 0x0              ; Acts as a placeholder and validation point
; offset 0x8
gdt_code:
            dw 0xffff           ; Size of the segment - 1 (either in bytes or in 4 KiB units - see flags)
            dw 0x0              ; The address where the segment starts bits 0 - 15
            db 0x0              ; The address where the segment starts bits 16 - 23
            db 0x9a             ; Access information (ring, executable, etc.)
            db 0b11001111       ; Defines the segment size unit and 16/32 bit.
            db 0x0              ; The address where the segment starts bits 24 - 31
; offset 0x10
gdt_data:
            dw 0xffff           ; Size of the segment - 1 (either in bytes or in 4 KiB units - see flags)
            dw 0x0              ; The address where the segment starts bits 0 - 15
            db 0x0              ; The address where the segment starts bits 16 - 23
            db 0x92             ; Access information (ring, executable, etc.)
            db 0b11001111       ; Defines the segment size unit and 16/32 bit.
            db 0x0              ; The address where the segment starts bits 24 - 31
gdt_end:

;=============================================================================
; GDT Descriptor
;
; This section defines the GDT descriptor, specifying the length of the GDT
; and the starting address.
;=============================================================================
gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; 16 bit length
    dd gdt_start                 ; 32 bit base address

[BITS 32]
;=============================================================================
; load_kernel
;
; Loads the kernel from an ATA disk.
;
; @param None
;
; @return None
;=============================================================================
load_kernel:
    mov eax, 1                  ; Set the LBA (Logical Block Address) of the first sector after the bootloader
    mov ecx, 100                ; Set the number of sectors to read (for now, reading 100 sectors)
    mov edi, 0x0100000          ; Set the address to load the kernel to

    call ata_lba_read           ; Crucial for loading the kernel from the ATA disk.
                                ; It handles the process of reading sectors and loading them into memory
                                ; at the specified address (edi). Without this, the kernel data won't be loaded
                                ; and subsequent jumps to the kernel entry point would lead to incorrect behavior
    jmp CODE_SEG:0x0100000      ; Its switched to the code gdt code segment and jump to the entry point of the loaded kernel

;=============================================================================
; ata_lba_read
;
; Responsible for reading data from the ATA (Advanced Technology
; Attachment) disk using LBA (Logical Block Addressing) mode. It sends the
; necessary commands and parameters to the ATA controller to initiate the read
; operation.
;
; @param ECX: Number of sectors to read
; @param EBX: LBA (Logical Block Address) value
;
; @return EDI: The address of the buffer to store data obtained from the disk
;=============================================================================
ata_lba_read:
    ; Send the highest 8 bits of the lba
    mov ebx, eax                            ; Backup the original LBA
    shr eax, 24                             ; Move the highest 8 bits to the lowest 8 bits
    or eax, 0xe0                            ; Set bit 6 in AL for LBA mode
    mov dx, ATA_CONTROL_PORT                ; I/O port address for the control register of the ATA controller
    out dx, al                              ; Send the 8 bits to the ATA controller

    ; Send the total sectors to read
    mov eax, ecx                            ; The total sectors are hold in ECX (N_SECTORS_TO_READ)
    mov dx, ATA_SECTOR_COUNT_PORT           ; Address to send the number of sectors to read
    out dx, al                              ; Send the lower 8 Bits to the ATA controller

    ; Send the lowest 8 bits of the lba
    mov edx, ATA_LBA_LOW_PORT               ; Port to send bit 0 - 7 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    out dx, al                              ; Send the bit 0 - 7 to the ATA controller

    ; Send the middle 8 bits of the lba
    mov edx, ATA_LBA_MID_PORT               ; Port to send bit 8 - 15 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    shr eax, 8                              ; Get bit 8 - 15 in AL
    out dx, al                              ; Send the bit 8 - 15 to the ATA controller

    ; Send the highest 8 bits of the lba
    mov edx, ATA_LBA_HIGH_PORT              ; Port to send bit 16 - 23 of LBA
    mov eax, ebx                            ; Restore LBA from EBX
    shr eax, 16                             ; Get bit 16 - 23 in AL
    out dx, al                              ; Send the bit 16 - 23 to the ATA controller
 
    mov edx, ATA_COMMAND_PORT               ; Command port
    mov al, 0x20                            ; Read with retry
    out dx, al                              ; Send the command to the ATA controller
;=============================================================================
; ata_read_next_sector
;
; Represents the beginning of a loop that reads multiple sectors
; from the ATA disk. The loop will continue until ECX (the number of remaining
; sectors to read) becomes zero. The purpose of pushing ECX onto the stack is
; to preserve its current value so that it can be restored later after the loop.
;
; @param None
;
; @return None
;=============================================================================
.ata_read_next_sector:
    push ecx                                ; Store the current value of ECX on the stack so that we can decrement it with the loop below
;=============================================================================
; ata_check_is_readable
;
; Checks if the ATA controller is ready for data transfer. It reads
; the status register from the ATA command port and checks the 4th bit. If the
; 4th bit is set, it means the device is ready, and data can be read. If the
; bit is not set, the ATA controller is not ready yet, so it jumps back to wait.
; Once the device is ready, it reads data from the ATA data port using rep insw.
; The process repeats for a specified number of times (256 times in this case),
; and ECX is decremented accordingly.
;
; @param None
;
; @return None
;
;=============================================================================
.ata_check_is_readable:
    ; Check if the ATA controller is ready
    mov dx, ATA_COMMAND_PORT                ; Load the command port address into DX
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

ATA_CONTROL_PORT equ 0x1F6
ATA_SECTOR_COUNT_PORT equ 0x1F2
ATA_LBA_LOW_PORT equ 0x1F3
ATA_LBA_MID_PORT equ 0x1F4
ATA_LBA_HIGH_PORT equ 0x1F5
ATA_COMMAND_PORT equ 0x1F7
ATA_DATA_PORT equ 0x1F0

;=============================================================================
; Fill Remaining Boot Sector Bytes and Add Magic Signature
;
; Fill the remaining bytes of the boot sector with zeros up to byte 510.
; Then add the magic BIOS signature 0xaa55 at the end to indicate a valid 
; bootable sector.
;=============================================================================
times 510 - ($ - $$) db 0x0
dw 0xaa55