target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kmain
break kernel.c:116

c