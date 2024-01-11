target remote :1234
layout split
focus cmd
set print pretty on
break _start
break kmain
break kernel.c:156
c