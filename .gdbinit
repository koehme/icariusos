target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break test_ata_write
break fat16_open
break _create_fat16_file
break fat16_write
break _update_fat_entry

c