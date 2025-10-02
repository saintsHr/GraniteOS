.SILENT:
all:
	mkdir -p build
	mkdir -p bin
	mkdir -p isodir/boot/grub

	i686-elf-as boot/boot.s -o build/boot.o
	i686-elf-gcc -c src/kernel.c -o build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-gcc -T linker.ld -o bin/os.bin -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o -lgcc

	cp bin/os.bin isodir/boot/os.bin
	cp grub.cfg isodir/boot/grub/grub.cfg

	grub-mkrescue -o bin/os.iso isodir

.SILENT:
run:
	qemu-system-i386 -cdrom bin/os.iso