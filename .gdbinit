target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:kmain
break vfs.c:vfs_fopen

c