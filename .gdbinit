target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break vfs.c:vfs_fopen
break fat16.c:fat16_get

c