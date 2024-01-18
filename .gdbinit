target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel
break kernel.c:170
break kernel.c:197
c