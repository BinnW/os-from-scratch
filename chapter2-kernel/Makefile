all: 
	boot.bin loader.bin

loader.bin:
	nasm loader.asm -o loader.bin

boot.bin:
	nasm boot.asm -o boot.bin

head.o:		head.S
	gcc -E head.S > head.s
	as --64 -o head.o head.s

main.o:		main.c
	gcc -mcmodel=large -fno-builtin -m64 -c main.c -o main.o

system:      head.o main.o
	ld -b  elf64-x86-64 -T Kernel.lds -o system head.o main.o

all: 		system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

clean:
	rm -rf *.bin *.asm~ Makefile~ loader.bin boot.bin
