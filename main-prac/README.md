# i.MX6UL 裸机驱动实践项目

## 1. 项目简介

本项目是一个基于 **i.MX6UL** 芯片的嵌入式裸机驱动开发实践项目。通过从零开始编写各类外设驱动，帮助学习者深入理解 ARM  Cortex-A7 架构及 i.MX6UL 芯片的外设工作原理。

项目不依赖任何操作系统（OS-free），直接在硬件层面编写驱动代码，是嵌入式Linux驱动开发的基础练习。

## 2. 硬件平台

| 项目 | 说明 |
|------|------|
| **CPU** | NXP i.MX6UL (MCIMX6Y2) |
| **架构** | ARM Cortex-A7, ARMv7-A |
| **主频** | 528MHz (可配置更高) |
| **内存** | DDR3 |
| **存储** | NAND Flash / eMMC |
| **调试串口** | UART1 (115200-8-N-1) |

### 芯片特性
- 集成 LCD 控制器、摄像头接口、CAN、Ethernet、USB 等外设
- 支持多种启动方式（SD卡、NAND、eMMC、串口等）
- 适用于工业控制、HMI、IoT网关等领域

## 3. 项目结构

```
main-prac/
├── imx6ul/                 # i.MX6UL 芯片头文件
│   ├── cc.h               # 数据类型定义
│   ├── MCIMX6Y2.h         # 芯片寄存器定义 (CMSIS)
│   ├── core_ca7.h         # Cortex-A7 内核定义
│   ├── fsl_common.h       # 通用外设定义
│   ├── fsl_iomuxc.h       # IO 复用配置
│   └── imx6ul.h           # 统一包含所有头文件
│
├── bsp/                   # Board Support Package - 板级驱动
│   ├── led/               # LED 驱动
│   ├── key/               # 按键驱动
│   ├── beep/              # 蜂鸣器驱动
│   ├── uart/              # 串口驱动 (UART1)
│   ├── gpio/              # GPIO 通用驱动
│   ├── clk/               # 时钟管理驱动
│   ├── delay/             # 延时函数
│   ├── int/               # 中断管理
│   ├── exit/              # 外部中断驱动
│   ├── epittimer/         # EPIT 定时器驱动
│   ├── keyfilter/         # 按键消抖滤波
│   └── stdio/             # printf/scanf 重定向
│
├── project/               # 用户应用程序
│   └── main.c             # 主程序入口
│
├── obj/                   # 编译生成的 .o 文件目录
│
├── Makefile               # 编译脚本
├── imx6ul.lds             # 链接脚本 (入口地址 0x87800000)
├── imxdownload            # 镜像烧录工具
└── *.bin / *.elf          # 编译产物
```

## 4. 已实现的功能模块

### 4.1 LED 控制
- `led_init()` - 初始化 LED 对应的 GPIO
- `led_on()` / `led_off()` - 点亮/熄灭 LED
- `led_switch(led, status)` - 通用 LED 控制

### 4.2 按键输入
- `key_init()` - 初始化按键 GPIO
- `key_getvalue()` - 获取按键值
- 支持按键消抖处理 (`keyfilter` 模块)

### 4.3 串口通信 (UART1)
- `uart_init()` - 初始化 UART1, 波特率 115200
- `uart_putchar()` - 发送单个字符
- `uart_getchar()` - 接收单个字符
- `uart_puts()` - 发送字符串
- 使用 DMA 或轮询模式

### 4.4 printf/scanf 重定向
- 通过实现 `_write()` 和 `_read()` 系统调用
- 将标准输入输出重定向到 UART1
- 支持整型、字符、字符串、格式化输出

### 4.5 时钟管理
- `imx6u_clkinit()` - CPU 时钟初始化
- `clk_init()` - 外设时钟使能
- UART 时钟配置 (PLL3@80MHz)

### 4.6 延时函数
- `delay()` - 毫秒级延时
- 基于 EPIT 定时器实现精确定时

### 4.7 其他外设
- **蜂鸣器 (BEEP)** - 蜂鸣器控制
- **外部中断 (EXIT)** -  GPIO 中断配置
- **EPIT 定时器** - 周期定时中断

## 5. 编译方法

### 5.1 环境准备
确保已安装 ARM 交叉编译工具链：
```bash
sudo apt-get install gcc-arm-linux-gnueabi
```

### 5.2 编译步骤
```bash
cd /path/to/main-prac

# 清理旧文件
make clean

# 编译项目
make

# 编译成功后生成以下文件:
# main-prac.bin  - 可执行镜像 (烧录用)
# main-prac.elf  - ELF 格式 (调试用)
# main-prac.dis  - 反汇编文件
```

### 5.3 烧录运行
```bash
# 方法1: 使用 imxdownload 工具烧录到 SD 卡
sudo ./imxdownload main-prac.bin /dev/sdX

# 方法2: 通过uboot网络加载 (tftp)
# 在 uboot 中:
tftp 80800000 main-prac.bin
go 87800000
```

### 5.4 调试串口
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 校验位: None
- 流控制: None

连接 TTL 串口模块到开发板 UART1 引脚，使用串口终端软件（如 minicom、PuTTY）查看输出。

## 6. 注意事项

### 6.1 地址映射
- 程序入口地址: `0x87800000` (DDR 起始地址)
- 链接脚本定义在 `imx6ul.lds` 中
- 需根据实际内存配置调整地址

### 6.2 交叉编译工具链
- 本项目使用 `arm-linux-gnueabi-` 工具链
- 如使用其他工具链（如 `arm-none-eabi-`），请修改 Makefile 中的 `CROSS_COMPILE` 变量

### 6.3 GPIO 引脚复用
- 使用 `IOMUXC` 配置引脚功能
- 不同开发板引脚定义可能不同，请根据硬件原理图修改

### 6.4 串口通信
- 当前默认使用 UART1
- 修改 `bsp_uart.h` 中的引脚配置可切换到其他 UART

### 6.5 调试建议
1. **先跑通 UART** - 确保串口输出正常后再调试其他模块
2. **善用 LED** - 通过 LED 闪烁状态判断程序运行位置
3. **反汇编辅助** - 查看 `*.dis` 文件辅助调试
4. **分段测试** - 每个驱动模块独立测试后再集成

### 6.6 后续学习方向
- 添加 RTOS 支持 (FreeRTOS / uC/OS-II)
- 实现更多外设驱动 (LCD, ETH, USB, SD卡)
- 移植 U-Boot 或构建最小 Linux 系统

## 7. 参考资料

- [i.MX6ULL Reference Manual](https://www.nxp.com.cn/docs/en/reference-manual/IMX6ULLRM.pdf)
- [Cortex-A7 Technical Reference Manual](https://developer.arm.com/documentation/ddi0464/latest/)
- 正点原子 i.MX6U 驱动开发教程

---

**Happy Hacking! 🧅**
