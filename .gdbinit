target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break fat16.c:fat16_write

c