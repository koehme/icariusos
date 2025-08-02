target remote :1234

layout split
focus cmd

set architecture i386
set print pretty on
set disassembly-flavor intel

b syscall_dispatch
b _sys_exit
add-symbol-file ./src/user/icarsh/elf/icarsh.elf 0x00000000

c