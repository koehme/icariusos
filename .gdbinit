target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:461
break asm_task_switch_to_userland

c