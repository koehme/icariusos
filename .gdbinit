target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break start_higher_half_kernel
break kmain

c