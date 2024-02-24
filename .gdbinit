target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break heap.c:heap_free

c