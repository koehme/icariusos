KERNEL_VIRTUAL_START EQU 0xC0000000

BITS 32

global _start
extern kmain
global kernel_directory

section .multiboot
align 8

header_start:
    dd 0xe85250d6              
    dd 0                        
    dd header_end - header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) 
    dd 5                           
    dd 20                            
    dd 800                             
    dd 600                             
    dd 32                           
header_end:

section .bss
stack_bottom:
    resb 16384 * 8
stack_top:

section .data
ALIGN 4096
kernel_directory:
    DD 0x00000083 ; First Page Table Entry (0x00000000 - 0x003FFFFF)
    TIMES 768-1 DD 0
    ; Kernel Mapping: Mapping von 0xC0000000 - 0xC2FFFFFF (48 MiB)
    DD 0x00000083 ; Entry 768 (0xC0000000 - 0xC03FFFFF) mapped to 0x00000000 - 0x003FFFFF
    DD 0x00400083 ; Entry 769 (0xC0400000 - 0xC07FFFFF) mapped to 0x00400000 - 0x007FFFFF
    DD 0x00800083 ; Entry 770 (0xC0800000 - 0xC0BFFFFF) mapped to 0x00800000 - 0x00BFFFFF
    DD 0x00C00083 ; Entry 771 (0xC0C00000 - 0xC0FFFFFF) mapped to 0x00C00000 - 0x00FFFFFF
    DD 0x01000083 ; Entry 772 (0xC1000000 - 0xC13FFFFF) mapped to 0x01000000 - 0x013FFFFF
    DD 0x01400083 ; Entry 773 (0xC1400000 - 0xC17FFFFF) mapped to 0x01400000 - 0x017FFFFF
    DD 0x01800083 ; Entry 774 (0xC1800000 - 0xC1BFFFFF) mapped to 0x01800000 - 0x01BFFFFF
    DD 0x01C00083 ; Entry 775 (0xC1C00000 - 0xC1FFFFFF) mapped to 0x01C00000 - 0x01FFFFFF
    DD 0x02000083 ; Entry 776 (0xC2000000 - 0xC23FFFFF) mapped to 0x02000000 - 0x023FFFFF
    DD 0x02400083 ; Entry 777 (0xC2400000 - 0xC27FFFFF) mapped to 0x02400000 - 0x027FFFFF
    DD 0x02800083 ; Entry 778 (0xC2800000 - 0xC2BFFFFF) mapped to 0x02800000 - 0x02BFFFFF
    DD 0x02C00083 ; Entry 779 (0xC2C00000 - 0xC2FFFFFF) mapped to 0x02C00000 - 0x02FFFFFF

    TIMES 128-12 DD 0

    ; Framebuffer Mapping bei 0xFD000000
    DD 0xFD000083 ; Entry 896 (0xE0000000 - 0xE03FFFFF) mapped to 0xFD000000 - 0xFD3FFFFF

    TIMES 1024-897 DD 0 ; Fill up the rest of the Page Directory

section .text

%include "./src/arch/i686/gdt.asm"
%include "./src/arch/i686/pic.asm"

_start:
    ; Load the address of kernel_directory into ecx
    mov ecx, kernel_directory
    sub ecx, KERNEL_VIRTUAL_START
    mov cr3, ecx           ; Setze CR3 auf die Adresse von kernel_directory

    ; Enable PSE with 4 MiB pages
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; Enable paging
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    ; Jump to higher half kernel start
    lea ecx, [start_higher_half_kernel]
    jmp ecx

start_higher_half_kernel:
    ; Setup stack
    mov esp, stack_top
    xor ebp, ebp
    push ebx
    push eax
    cli
    
    ; Call kernel_main
    call kmain
    
    ; Halt if kernel_main returns
halt:
    hlt
    jmp halt


