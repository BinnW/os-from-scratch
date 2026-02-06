# 编译工具链确认
CROSS_COMPILE ?= arm-linux-gnueabi
# 目标文件名
TARGET ?= bsp

# 工具链定义
CC     := $(CROSS_COMPILE)-gcc
LD     := $(CROSS_COMPILE)-ld
OBJCOPY := $(CROSS_COMPILE)-objcopy
OBJDUMP := $(CROSS_COMPILE)-objdump

# 头文件所在目录
INCDIRS := imx6ul  \
		   bsp/clk \
		   bsp/led \
		   bsp/delay \
		   bsp/beep \
		   bsp/gpio \
		   bsp/key

# 源代码所在目录
SRCDIRS := project \
		   bsp/led \
		   bsp/clk \
		   bsp/delay \
		   bsp/beep \
		   bsp/gpio \
		   bsp/key

# 为每个头文件所在目录添加-I，为了gcc编译时使用
INCLUDE := $(patsubst %, -I %, $(INCDIRS))

# 搜索源文件目录下的所有.s和.c文件
SFILES  := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.s))
CFILES  := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

# 去掉.s和.c的文件夹路径，得到文件名
SFILENDIR := $(notdir $(SFILES))
CFILENDIR := $(notdir $(CFILES))

# 这一步得到目标文件的.o文件名，结合obj路径，
# 不定义这个obj的话就直接生成在当前目录下
SOBJS := $(patsubst %, obj/%, $(SFILENDIR:.s=.o))
COBJS := $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS := $(SOBJS) $(COBJS)

# 设置源文件搜索路径，告诉make在哪些目录查找源文件
VPATH := $(SRCDIRS)

# 这个PHONY是什么？
# A：clean是一个伪目标，表示这个目标不是一个实际的文件，而是一个命令。
.PHONY: clean

# 主体构建规则，目标是bsp.bin，来源是所有的.o系列文件
$(TARGET).bin : $(OBJS)
# 用依赖的OBJS生成elf文件
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^
# 用elf文件生成bin文件，通过objcopy
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
# 生成elf文件的反汇编代码
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

# 这里有些疑问
$(SOBJS) : obj/%.o : %.s
# 
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<


$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<

clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(COBJS) $(SOBJS)
