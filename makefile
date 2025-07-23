CFLAGS = -std=gnu99 -ffreestanding -O2 -Wextra -m32
CC = i686-elf-gcc
AS = i686-elf-as
LDFLAGS = -ffreestanding -O2 -nostdlib -m32
TOLINK = \
	Objects/boot.o \
	Objects/gdt_flush.o \
	Objects/gdt.o \
	Objects/kernel.o \
	Objects/util.o \
	Objects/idt.o \
	Objects/idts.o \
	Objects/timer.o \
	Objects/VGA.o \
	Objects/keyboard.o \
	Objects/memory.o \
	Objects/kmalloc.o \
	Objects/liballoc.o \
	Objects/liballoc_hooks.o \
	Objects/ata_pio.o \
	Objects/syscall.o \
	Objects/syscall_dispatch.o \
	Objects/SFS.o 
build:
	$(AS) src/boot.asm -o Objects/boot.o
	nasm -felf32 src/idt.s -o Objects/idts.o
	nasm -felf32 src/syscall.asm -o Objects/syscall.o
	$(AS) src/gdt_flush.s -o Objects/gdt_flush.o

	$(CC) -c src/timer.c -o Objects/timer.o $(CFLAGS)
	$(CC) -c src/keyboard.c -o Objects/keyboard.o $(CFLAGS)
	$(CC) -c src/VGA.c -o Objects/VGA.o $(CFLAGS)
	$(CC) -c src/memory.c -o Objects/memory.o $(CFLAGS)
	$(CC) -c src/kernel.c -o Objects/kernel.o $(CFLAGS)
	$(CC) -c src/idt.c -o Objects/idt.o $(CFLAGS)
	$(CC) -c src/util.c -o Objects/util.o $(CFLAGS)
	$(CC) -c src/gdt.c -o Objects/gdt.o $(CFLAGS)
	$(CC) -c src/kmalloc.c -o Objects/kmalloc.o $(CFLAGS)
	$(CC) -c src/liballoc.c -o Objects/liballoc.o $(CFLAGS)
	$(CC) -c src/liballoc_hooks.c -o Objects/liballoc_hooks.o $(CFLAGS)
	$(CC) -c src/ata_pio.c -o Objects/ata_pio.o $(CFLAGS)
	$(CC) -c src/SFS.c -o Objects/SFS.o $(CFLAGS)
	$(CC) -c src/syscall_dispatch.c -o Objects/syscall_dispatch.o $(CFLAGS)
	$(CC) -T src/linker.ld -o iso/boot/kernel.elf $(LDFLAGS) $(TOLINK) -lgcc

build_grub: build
	grub-mkrescue -o CorrectOS.iso iso/

run:
	qemu-system-i386 -cdrom CorrectOS.iso -hda hdd.img -m 512M -vga std

clean:
	rm -f iso/boot/kernel.elf CorrectOS.iso
	rm -f Objects/*.o