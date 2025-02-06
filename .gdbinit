target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break asm_isr13_wrapper
break isr_13_handler

c