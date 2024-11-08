target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kmain
break page_paging_init
break heap_init
break asm_do_sti

c