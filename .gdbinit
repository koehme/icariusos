target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break *0x00000000

c