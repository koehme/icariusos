target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kmain:195
break kmain:196
break kmain:198

break vfs.c:vfs_fopen

c