ORG 0x7c00                      ; Allows manual setting of segments to 0x7C00 later,
                                ; addressing potential BIOS discrepancies in segment values during boot loading
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
    mov ax, 0x00               ; Set up data segment (ds) and extra segment (es)
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

    ; TODO: Implement reading from the ATA hard disk and loading our kernel into memory at 0x0100000 (1024*1024)
    ; We must start reading from sector 1 because the first 512 bytes are the boot sector
    ; For now, let's read 100 sectors. After loading, we want to jump with jmp CODE_SEG:0x0100000 to the code segment (GDT pointer) to the kernel location at 0x0100000
    jmp $                       ; Temporary, will be replaced soon after implementing the ATA disk read driver

;=============================================================================
; ATA read sectors (LBA mode) 
;
; @param EAX Logical Block Address of sector
; @param CL  Number of sectors to read
; @param RDI The address of buffer to put data obtained from disk
;
; @return None
;=============================================================================
ata_lba_read: ; TODO


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