target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break vfs.c:vfs_fopen
break vfs.c:vfs_fstat

c