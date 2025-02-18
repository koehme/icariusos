target remote :1234
layout split
focus cmd
set print pretty on
set disassembly-flavor intel

break vfs_fopen
break path_parser_parse
break _path_parser_parse_filename

c