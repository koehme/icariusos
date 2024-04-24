target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break mouse.c:mouse_handler

c