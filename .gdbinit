target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:253
break kernel.c:260

c