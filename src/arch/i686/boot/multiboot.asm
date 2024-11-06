section .multiboot_header
bits 32

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