target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kmain
break scheduler_tick
break irq0_handler

c