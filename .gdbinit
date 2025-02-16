target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break asm_user_shell
break asm_syscall

c