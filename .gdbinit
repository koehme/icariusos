target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break task_create_user
break page_map_between

c