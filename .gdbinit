target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:193
break string.c:itoa

c