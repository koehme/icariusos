all:
	nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin
clean:
	rm -rf ./bin/boot.bin