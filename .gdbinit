target remote :1234

layout split
focus cmd

set architecture i386
set print pretty on
set disassembly-flavor intel

b asm_isr13_wrapper
b asm_isr14_wrapper
b isr_13_handler
b isr_14_handler

add-symbol-file ./src/user/icarsh/elf/icarsh.elf 0x00000000
break *0x0
c