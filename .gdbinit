target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

add-symbol-file ./src/user/icarsh/elf/icarsh.elf 0x00000000

break main
break kmain

c