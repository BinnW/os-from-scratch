objs := start.o main.o

# 默认目标，目标是生成ledc.bin
ledc.bin:$(objs)
	arm-linux-gnueabi-ld -Timx6ul.lds -o ledc.elf $^
	arm-linux-gnueabi-objcopy -O binary -S -g ledc.elf ledc.bin
	arm-linux-gnueabi-objdump -d ledc.elf > ledc.dis

%.o:%.s
	arm-linux-gnueabi-gcc -Wall -nostdlib -c -o $@ $<

%.o:%.S
	arm-linux-gnueabi-gcc -Wall -nostdlib -c -o $@ $<

%.o:%.c
	arm-linux-gnueabi-gcc -Wall -nostdlib -c -o $@ $<

clean:
	rm -f *.o ledc.elf ledc.bin ledc.dis