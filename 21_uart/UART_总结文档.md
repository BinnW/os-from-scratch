# UART 驱动与 printf/scanf 重定向总结文档

## 项目概述

本项目是 i.MX6ULL 处理器的裸机编程实践，使用 UART1 作为调试串口，实现了 printf/scanf 函数的重定向。

---

## 1. 关键寄存器配置

### 1.1 UART 寄存器映射

UART1 基地址在 `imx6ul.h` 中定义，使用 `UART1` 宏访问。

### 1.2 寄存器详解

| 寄存器 | 用途 | 本项目配置 |
|--------|------|-----------|
| **UCR1** | UART 控制寄存器1 | `UARTEN=1` 启用 UART |
| **UCR2** | UART 控制寄存器2 | `WS=1`(8位数据), `STPB=1`(1停止位), `TXEN=1`, `RXEN=1`, `SRST=1` |
| **UCR3** | UART 控制寄存器3 | `UARTEN=1` 启用 FIFO |
| **UFCR** | UART FIFO 控制 | `RXWATER=1`, `TXWATER=0` |
| **UBIR** | UART 波特率索引寄存器 | `71` |
| **UBMR** | UART 波特率分子寄存器 | `3124` |
| **USR2** | UART 状态寄存器2 | `TXDC` 标志表示发送完成 |
| **UTXD** | UART 发送数据寄存器 | 写入要发送的字符 |
| **URXD** | UART 接收数据寄存器 | 读取接收的字符 |

### 1.3 波特率计算公式

```
波特率 = UART_CLK / (16 * (UBMR + 1) / (UBIR + 1))

本项目配置:
- UART_CLK = 80MHz (pll3_80M)
- 目标波特率 = 115200
- UBIR = 71, UBMR = 3124
```

---

## 2. UART 初始化流程

### 2.1 初始化步骤

```
uart_init()
    │
    ├─ 1. uart1_clk_init()      // 初始化 UART1 时钟
    │
    ├─ 2. IOMUXC 配置           // 配置 UART1_TX/RX 引脚复用
    │   - IOMUXC_SetPinMux()    // 设置引脚复用为 UART 功能
    │   - IOMUXC_SetPinConfig() // 配置引脚电气特性 (0x10B0)
    │
    ├─ 3. 软件复位              // UCR1 &= ~UARTEN
    │
    ├─ 4. 控制寄存器配置
    │   - UCR1: 启用 UART
    │   - UCR2: 8N1 (8位数据, 无校验, 1停止位), 启用 TX/RX
    │   - UCR3: 启用 FIFO
    │   - UFCR: 设置 FIFO 水位线
    │
    ├─ 5. 设置波特率            // uart_set_baudrate(115200)
    │
    └─ 6. 等待发送完成          // while (!(USR2 & TXDC))
```

### 2.2 引脚配置

| 引脚 | IOMUXC 配置 | 功能 |
|------|------------|------|
| UART1_TX | `IOMUXC_UART1_TX_DATA_UART1_TX` | UART1 发送 |
| UART1_RX | `IOMUXC_UART1_RX_DATA_UART1_RX` | UART1 接收 |

电气特性: `0x10B0` (默认配置)

---

## 3. 数据发送/接收函数

### 3.1 发送函数

```c
void uart_putchar(char c)
{
    // 等待发送缓冲区为空 (TXDC 标志置位)
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
    
    // 写入数据到发送寄存器
    UART1->UTXD = c;
}
```

### 3.2 接收函数

```c
char uart_getchar(void)
{
    // 等待数据就绪 (CHARRDY 标志置位)
    while (!(UART1->URXD & UART_URXD_CHARRDY_MASK));
    
    // 返回接收到的数据 (低8位)
    return (char)(UART1->URXD & 0xFF);
}
```

### 3.3 字符串发送

```c
void uart_puts(const char *str)
{
    while (*str) {
        uart_putchar(*str++);
        // 换行符自动补充回车符
        if (*str == '\n') {
            uart_putchar('\r');
        }
    }
}
```

---

## 4. printf/scanf 重定向原理

### 4.1 重定向机制

ARM GCC 裸机环境下，标准库的 I/O 函数通过系统调用实现。实现 `_write` 和 `_read` 函数即可将标准 I/O 重定向到 UART。

### 4.2 _write 函数 (printf 输出)

```c
int _write(int fd, char *ptr, size_t len)
{
    // fd = 1 (stdout) 或 fd = 2 (stderr)
    for (i = 0; i < len; i++) {
        uart_putchar(ptr[i]);
        
        // 换行自动补充回车
        if (ptr[i] == '\n') {
            uart_putchar('\r');
        }
    }
    return len;
}
```

### 4.3 _read 函数 (scanf 输入)

```c
int _read(int fd, char *ptr, size_t len)
{
    // fd = 0 (stdin)
    for (i = 0; i < len; i++) {
        ch = uart_getchar();
        uart_putchar(ch);  // 回显
        
        // 回车转换为换行
        if (ch == '\r') {
            uart_putchar('\n');
            ptr[i] = '\n';
            return i + 1;
        }
        
        ptr[i] = ch;
        
        // 遇到换行符返回
        if (ch == '\n') {
            return i + 1;
        }
    }
    return len;
}
```

### 4.4 printf 实现 (简化版)

项目实现了精简版 `printf`，支持以下格式符：

| 格式符 | 说明 | 示例 |
|--------|------|------|
| `%d` / `%i` | 有符号十进制整数 | `-123` |
| `%u` | 无符号十进制整数 | `456` |
| `%x` | 小写十六进制 | `0xdead` |
| `%X` | 大写十六进制 | `0xDEAD` |
| `%c` | 字符 | `'A'` |
| `%s` | 字符串 | `"hello"` |
| `%%` | 输出百分号 | `%` |

### 4.5 scanf 实现 (简化版)

项目实现了精简版 `scanf`，支持以下格式符：

| 格式符 | 说明 | 示例输入 |
|--------|------|---------|
| `%d` / `%i` | 有符号十进制整数 | `-123` |
| `%u` | 无符号十进制整数 | `456` |
| `%x` | 十六进制整数 | `0xdead` |
| `%c` | 单个字符 | `A` |
| `%s` | 字符串 (不含空格) | `hello` |

---

## 5. Makefile 配置

### 5.1 编译工具链

```makefile
CROSS_COMPILE ?= arm-linux-gnueabi
CC := $(CROSS_COMPILE)-gcc
```

### 5.2 头文件目录

```makefile
INCDIRS := ... \
           bsp/uart \
           bsp/stdio
```

### 5.3 源文件目录

```makefile
SRCDIRS := project \
           ... \
           bsp/uart \
           bsp/stdio
```

### 5.4 编译选项

```makefile
# 自由standing 环境 (无操作系统)
# ARMv7-A 架构 + 浮点单元
$(CC) -Wall -ffreestanding -march=armv7-a+fp -c -O2 $(INCLUDE)
```

---

## 6. 使用示例 (main.c)

```c
int main(void)
{
    // 初始化
    imx6u_clkinit();
    clk_init();
    uart_init();
    
    // printf 测试
    printf("\r\n=== UART Test ===\r\n");
    printf("Integer: %d\r\n", 42);
    printf("Hex: %x\r\n", 0xDEAD);
    
    // scanf 测试
    int num;
    printf("Please enter a number: ");
    scanf("%d", &num);
    printf("You entered: %d\r\n", num);
    
    while (1);
    return 0;
}
```

---

## 7. 关键要点总结

1. **UART 初始化顺序**: 时钟 → 引脚 → 复位 → 控制寄存器 → 波特率 → 等待发送完成
2. **波特率配置**: UBIR=71, UBMR=3124 对应 80MHz 时钟下的 115200 波特率
3. **数据发送**: 轮询 TXDC 标志等待发送完成
4. **数据接收**: 轮询 CHARRDY 标志等待数据就绪
5. **printf 重定向**: 实现 `_write()` 函数，将 stdout/stderr 定向到 UART
6. **scanf 重定向**: 实现 `_read()` 函数，将 stdin 定向到 UART，并实现回显
7. **换行处理**: 发送时 `\n` 自动补充 `\r`；接收时 `\r` 转换为 `\n`

---

*文档生成时间: 2026-03-18*
