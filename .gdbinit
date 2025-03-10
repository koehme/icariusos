target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break test_ata_write
break fat16_open
break fat16_create_file
break fat16_write
break fat16_create_fat_entry

c