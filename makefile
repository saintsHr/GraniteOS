src := $(shell find src -name '*.c')

.SILENT:
all:
	mkdir -p build
	mkdir -p bin
	mkdir -p isodir/boot/grub

	i686-elf-as boot/boot.s -o build/boot.o
	i686-elf-gcc -c $(src) -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	mv *.o build/
	i686-elf-gcc -T linker.ld -o bin/granite.bin -ffreestanding -O2 -nostdlib build/*.o -lgcc

	cp bin/granite.bin isodir/boot/granite.bin
	cp grub.cfg isodir/boot/grub/grub.cfg

	grub-mkrescue -o bin/granite.iso isodir

.SILENT:
run:
	qemu-system-i386 -cdrom bin/granite.iso -vga std