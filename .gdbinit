target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break fat16.c:fat16_open
break fat16.c:fat16_get_entry
break vfs.c:vfs_fopen
break vfs.c:vfs_fread

c