org 0x7C00
bits 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; FAT 16 HEADER
jmp short start                                      ; Jump to the start code after the BPB
nop                                                  ; No-operation instruction as a placeholder
OEMIdentifier                  db "MSDOS5.0"         ; OEM name
BytesPerSector                 dw 512                ; Bytes per sector
SectorsPerCluster              db 128                ; Sectors per cluster
ReservedSectors                dw 1                  ; Reserved sectors (boot sector)
FATCopies                      db 2                  ; Number of FATs
RootDirEntries                 dw 64                 ; Maximum root directory entries 
NumSectors                     dw 0                  ; Total sectors (if 0, check the later entry)
MediaType                      db 0xF8               ; Media descriptor (0xF8 for hard drive)
SectorsPerFat                  dw 256                ; Sectors per FAT
SectorsPerTrack                dw 32                 ; Sectors per track
NumberOfHeads                  dw 64                 ; Number of heads
HiddenSectors                  dd 0                  ; Hidden sectors
SectorsBig:                    dd 0x773594           ; Total sectors (if the earlier entry is 0)
; Extended BPB (Dos 4.0)
DriveNumber                    db 0x80               ; Drive number (0x80 for the first hard drive)
WinNTBit                       db 0x00               ; WinNT bit (typically set to 0 for non-WinNT systems)
Signature                      db 0x29               ; Signature bit for extended BPB
VolumeID                       dd 0xD105             ; Unique volume ID (can have any value)
VolumeIDString                 db 'ICARIUS  OS'      ; Volume label string
SystemIDString                 db 'FAT16   '         ; File system ID string (8 characters, can be padded with spaces)

start:
    jmp 0x0:init_bootloader

init_bootloader:
    cli                                             
    mov ax, 0x00              
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00     
    sti      
    ; Implement calc the lba of the root directory, later i will remove the code for ata_lba_read because its no longer need and i want to use 0x13 bios to load the kernel into memory
    mov ax, [SectorsPerFat]     ; Load SectorsPerFat into ax
    mov bl, [FATCopies]
    xor bh, bh                  ; Clean up the higher part of bx
    mul bx                      ; SectorsPerFat * FATCopies =	512 sectors
    add ax, [ReservedSectors]   ; LBA of the root directory = 512 + 1 = 513

    push ax                     ; Preserve the lba 513 of the root directory

    mov ax, [RootDirEntries]    ; 64
    shl ax, 5                   ; ax *= 32 (In FAT file systems, the root directory entry size is fixed at 32 bytes per entry) 
    xor dx, dx                  ; reset carry
    div word [BytesPerSector]   ; (32 * RootDirEntries) / BytesPerSector 
                                ; The idea is to divide the size of the root directory in bytes by the size of a sector to get the number of sectors that occupy the root directory
    test dx, dx                 ; carry in dx
    jz .root_dir_after
    inc ax                      ; dividend / divisor = ax

; Read the data from the root dir from the disk
.root_dir_after:
    mov cl, al                  ; size of the root dir
    pop ax                      ; restore the lba of the root dir

.load_protected:    
    cli                        
    lgdt [gdt_descriptor]       

    mov eax, cr0                
    or eax, 0x1                
    mov cr0, eax                

    jmp CODE_SEG:load_kernel

gdt_start:
gdt_null: 
            dq 0x0              

gdt_code:
            dw 0xffff          
            dw 0x0              
            db 0x0            
            db 0x9a            
            db 0b11001111      
            db 0x0            
gdt_data:
            dw 0xffff           
            dw 0x0             
            db 0x0         
            db 0x92           
            db 0b11001111       
            db 0x0             
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  
    dd gdt_start                

[BITS 32]
load_kernel:
    mov eax, 1                 
    mov ecx, 100             
    mov edi, 0x0100000          

    call ata_lba_read           
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax                           
    shr eax, 24                            
    or eax, 0xe0                           
    mov dx, ATA_CONTROL_PORT                
    out dx, al                            

    mov eax, ecx                          
    mov dx, ATA_SECTOR_COUNT_PORT         
    out dx, al                     

    mov edx, ATA_LBA_LOW_PORT        
    mov eax, ebx       
    out dx, al                      

    mov edx, ATA_LBA_MID_PORT          
    mov eax, ebx                     
    shr eax, 8                            
    out dx, al                     

    mov edx, ATA_LBA_HIGH_PORT           
    mov eax, ebx                          
    shr eax, 16                            
    out dx, al                             

    mov edx, ATA_COMMAND_PORT              
    mov al, 0x20                          
    out dx, al             

.ata_read_next_sector:
    push ecx                                

.ata_check_is_readable:
    mov dx, ATA_COMMAND_PORT             
    in al, dx                              
    test al, 0b00001000                   
    jz .ata_check_is_readable             

    mov ecx, 256                            
    mov dx, ATA_DATA_PORT                   
    rep insw                            
    pop ecx                                
    loop .ata_read_next_sector        
    ret

ATA_CONTROL_PORT equ 0x1F6
ATA_SECTOR_COUNT_PORT equ 0x1F2
ATA_LBA_LOW_PORT equ 0x1F3
ATA_LBA_MID_PORT equ 0x1F4
ATA_LBA_HIGH_PORT equ 0x1F5
ATA_COMMAND_PORT equ 0x1F7
ATA_DATA_PORT equ 0x1F0

times 510 - ($ - $$) db 0x0
dw 0xaa55