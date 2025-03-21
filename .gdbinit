target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break _sys_open
break _sys_close

c