target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break kernel.c:174
break kernel.c:175

break kernel.c:196
break kernel.c:197

break kernel.c:203
break kernel.c:204

break kernel.c:208
break kernel.c:209

c