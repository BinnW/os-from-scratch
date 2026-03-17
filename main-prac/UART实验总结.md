# UART 驱动实验总结

## 实验目标
实现 IMX6ULL UART1 串口驱动，完成以下功能：
- UART 初始化（时钟、GPIO、寄存器配置）
- 串口发送/接收
- 串口回显功能

---

## 一、硬件连接

| 功能 | 引脚 | 说明 |
|------|------|------|
| UART1_TX | GPIO1_IO16 | 发送 |
| UART1_RX | GPIO1_IO17 | 接收 |

---

## 二、代码文件结构

```
main-prac/
├── bsp/
│   ├── clk/
│   │   ├── bsp_clk_uart.h      # UART 时钟头文件
│   │   └── bsp_clk_uart.c      # UART 时钟实现
│   └── uart/
│       ├── bsp_uart.h          # UART 头文件
│       └── bsp_uart.c          # UART 实现
├── project/
│   └── main.c                  # 主程序（回显功能）
└── Makefile                    # 编译配置
```

---

## 三、关键代码

### 1. UART 时钟初始化 (bsp_clk_uart.c)

```c
#include "bsp_clk_uart.h"

void uart1_clk_init(void)
{
    /* 1. 开启UART1时钟 - CCGR1[13:10] */
    CCM->CCGR1 |= (0xF << 10);

    /* 2. 选择UART时钟源为 pll3_80m
     * CSCMR1[15] = 0: 选择 pll3_80m
     */
    CCM->CSCMR1 &= ~(1 << 15);

    /* 3. 设置UART时钟分频为1分频
     * CSCDR1[5:3] = 000: 1分频
     */
    CCM->CSCDR1 &= ~(7 << 3);
}
```

**说明：**
- UART1 时钟源选择 pll3_80M (80MHz)
- 分频设置为 1

---

### 2. UART 初始化 (bsp_uart.c)

```c
void uart_init(void)
{
    /* 1. 初始化 UART1 时钟 */
    uart1_clk_init();

    /* 2. 配置 UART1 引脚复用 */
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0);
    
    /* 配置引脚属性 */
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 0x10B0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 0x10B0);

    /* 3. 配置控制寄存器 */
    UART1->UCR1 = 0;
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;   /* 使能 UART */

    UART1->UCR2 = 0;
    UART1->UCR2 |= (1 << 5);    /* WS: 8位数据位 */
    UART1->UCR2 |= (1 << 6);    /* STPB: 1位停止位 */
    UART1->UCR2 &= ~(1 << 8);   /* PREN: 禁用奇偶校验 */
    UART1->UCR2 |= (1 << 2);    /* TXEN: 使能发送 */
    UART1->UCR2 |= (1 << 1);    /* RXEN: 使能接收 */
    UART1->UCR2 |= (1 << 0);    /* SRST: 复位 */

    UART1->UCR3 = 0;
    UART1->UCR3 |= (1 << 2);    /* RXEN: 使能接收 */

    /* UFCR: 设置FIFO水线 */
    UART1->UFCR = (5 << 7) | (0 << 0);

    /* 4. 设置波特率 115200 */
    uart_set_baudrate(UART1, UART_CLK_FREQ, UART_BAUDRATE);

    /* 5. 等待发送完成 */
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
}
```

---

### 3. 串口发送字符

```c
void uart_putchar(char c)
{
    /* 等待发送缓冲区空 */
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
    
    /* 发送数据 */
    UART1->UTXD = c;
}
```

---

### 4. 串口接收字符

```c
char uart_getchar(void)
{
    /* 等待接收到数据 */
    while (!(UART1->URXD & UART_URXD_CHARRDY_MASK));
    
    /* 返回接收到的数据 */
    return (char)(UART1->URXD & 0xFF);
}
```

---

### 5. 使能/去使能/复位函数

```c
/* UART 使能 */
void uart_enable(void)
{
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;
}

/* UART 去使能 */
void uart_disable(void)
{
    UART1->UCR1 &= ~UART_UCR1_UARTEN_MASK;
}

/* UART 软件复位 */
void uart_softreset(void)
{
    UART1->UCR2 &= ~UART_UCR2_SRST_MASK;  /* 复位信号置0 */
    while (!(UART1->UCR2 & UART_UCR2_SRST_MASK));  /* 等待复位完成 */
}
```

---

### 6. 主程序 - 回显功能 (main.c)

```c
int main(void)
{
    imx6u_clkinit();      // 初始化 CPU 时钟
    clk_init();           // 初始化外设时钟
    led_init();           // 初始化 LED
    uart_init();          // 初始化 UART1
    
    uart_puts("\r\nUART Echo Test!\r\n");
    uart_puts("Please input characters:\r\n");

    while (1) {
        char ch = uart_getchar();   // 读取数据
        uart_putchar(ch);           // 回显发送
    }

    return 0;
}
```

---

## 四、关键寄存器

### UART 控制寄存器

| 寄存器 | 位 | 说明 |
|--------|-----|------|
| UCR1 | 0 | UARTEN: 使能UART |
| UCR2 | 0 | SRST: 软件复位 |
| UCR2 | 1 | RXEN: 使能接收 |
| UCR2 | 2 | TXEN: 使能发送 |
| UCR2 | 5 | WS: 8位数据位 |
| UCR2 | 6 | STPB: 1位停止位 |
| UCR2 | 8 | PREN: 奇偶校验使能 |
| USR2 | 3 | TXDC: 发送完成标志 |
| URXD | 15 | CHARRDY: 字符就绪 |

---

## 五、编译

```bash
cd main-prac
make clean
make
```

生成 `main-prac.bin` 后使用 `imdownload` 烧录到开发板。

---

## 六、实验现象

1. 串口输出提示信息：`"UART Echo Test!"`
2. 开发板等待输入
3. 输入字符后，开发板会回显相同字符
