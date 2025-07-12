target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break irq0_handler
break irq1_handler
break task_unblock
break rr_add
break _rr_dequeue
break rr_yield

break _sys_read

c