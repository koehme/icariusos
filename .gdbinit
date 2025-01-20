target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break asm_interrupt_14h
break idt.c:isr_14h_handler

c