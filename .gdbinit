target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break _test_divide_by_zero

c