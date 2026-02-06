led.bin: led.s
	arm-linux-gnueabi-gcc -g -c led.s -o led.o
	arm-linux-gnueabi-ld -Ttext 0x87800000 led.o -o led.elf
	arm-linux-gnueabi-objcopy -O binary -S -g led.elf led.bin
	arm-linux-gnueabi-objdump -d led.elf > led.dis

clean:
	rm -f *.o led.elf led.bin led.dis