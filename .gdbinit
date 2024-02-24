target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break page.c:page_init_directory

c