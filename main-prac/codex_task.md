## 任务：重写 IMX6ULL UART 驱动代码

### 目标目录
/home/binn/linux-prac/os-from-scratch/main-prac/

### 需要创建/重写的文件

1. **bsp/clk/bsp_clk_uart.h** 和 **bsp_clk_uart.c**
   - UART1 时钟初始化
   - 选择 pll3_80m 时钟源
   - 1分频

2. **bsp/uart/bsp_uart.h** 和 **bsp_uart.c**
   - UART 初始化
   - 发送函数 uart_putchar
   - 接收函数 uart_getchar
   - 字符串发送 uart_puts
   - 使能 uart_enable
   - 去使能 uart_disable
   - 软复位 uart_softreset

### 技术要求
- 使用 IMX6ULL 的 UART1
- 引脚：UART1_TX (GPIO1_IO16) 和 UART1_RX (GPIO1_IO17)
- 波特率：115200
- 数据位：8位
- 停止位：1位
- 校验：无

### 参考
- 项目中已有的代码风格：bsp/gpio/, bsp/led/
- 头文件目录：imx6ul/, bsp/clk/, bsp/led/, bsp/gpio/
- 需要更新 Makefile 添加 uart 源文件路径

### 输出要求
- 代码风格与项目保持一致
- 确保 `make` 编译通过
