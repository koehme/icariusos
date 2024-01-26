target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:kmain
break fat16.c:print_fat16_lfn_entry

c