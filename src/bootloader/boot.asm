ORG 0x7c00                      ; Allows manual setting of segments to 0x7C00 later,
                                ; addressing potential BIOS discrepancies in segment values during boot loading
BITS 16

CODE_SEG equ gdt_code - gdt_head
DATA_SEG equ gdt_data - gdt_head

; Some BIOS systems expect the bios parameter block to be present
; The inclusion of this block helps to prevent damage to our code during the boot process
bpb:
    jmp short start  ; Jump to the start code after the BPB
    nop              ; No-operation instruction as a placeholder
    times 33 db 0    ; Additional space or padding, filled with null bytes

; Initial Jump to direct boot loader execution to a predictable location
; for the CS segment and ensure a consistent environment for further boot code execution
start:
    jmp 0:init_bootloader

; Configuring segment registers and setting up the stack
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
 
.load_protected_mode:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load_32

; It contains entries telling the CPU about memory segments
gdt_head:
    gdt_null: 
            dq 0x0              ; 8 bytes
    gdt_code:
            dw 0xffff           ; Segment limit 0 - 15 (16 bits)
            dw 0x0              ; Base address 0 - 15
            db 0x0              ; Base address 16 - 23
            db 0x9a             ; Access byte
            db 0b11001111       ; Set the flags
            db 0x0              ; Base address 24 - 31
    gdt_data:
            dw 0xffff           ; Segment limit 0 - 15 (16 bits)
            dw 0x0              ; Base address 0 - 15
            db 0x0              ; Base address 16 - 23
            db 0x92             ; Access byte
            db 0b11001111       ; Set the flags
            db 0x0              ; Base address 24 - 31
    gdt_tail:

gdt_descriptor:
    dw gdt_tail - gdt_head - 1
    dd gdt_head

[BITS 32]
load_32:
    mov ax, DATA_SEG
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

times 510 - ($ - $$) db 0x0     ; Fill up the remaining bytes minus the magic signature 0xaa55 with 0
dw 0xaa55                       ; magic bios signature