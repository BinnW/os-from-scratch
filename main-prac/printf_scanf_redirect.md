# printf/scanf 重定向到 UART 串口详解

> 基于 i.MX6UL 裸机环境下的 printf/scanf 重定向实现

## 目录

1. [什么是标准输入输出（stdin/stdout/stderr）](#1-什么是标准输入输出stdinstdoutstderr)
2. [printf/scanf 的底层原理（_write/_read 系统调用）](#2-printfscanf-的底层原理_write_read-系统调用)
3. [裸机环境下为什么需要重定向](#3-裸机环境下为什么需要重定向)
4. [i.MX6UL UART 寄存器配置](#4-imx6ul-uart-寄存器配置)
5. [_write 和 _read 函数的实现细节](#5-_write-和-_read-函数的实现细节)
6. [最小化 printf 的实现要点](#6-最小化-printf-的实现要点)
7. [最小化 scanf 的实现要点](#7-最小化-scanf-的实现要点)
8. [常见问题和调试方法](#8-常见问题和调试方法)

---

## 1. 什么是标准输入输出（stdin/stdout/stderr）

### 1.1 概念定义

标准输入输出是 C 标准库提供的一套统一的 I/O 接口，是 UNIX/Linux 系统的核心概念：

| 文件描述符 | 名称 | 默认关联 | 用途 |
|-----------|------|----------|------|
| 0 | stdin | 键盘/终端 | 标准输入，读取数据 |
| 1 | stdout | 显示器/终端 | 标准输出，正常输出 |
| 2 | stderr | 显示器/终端 | 标准错误，错误信息 |

### 1.2 在裸机环境中的含义

在 **有操作系统** 的环境中：
- `printf()` 默认输出到 stdout（终端）
- `scanf()` 默认从 stdin 读取（键盘）
- 操作系统通过文件描述符 0/1/2 管理这些流

在 **裸机（Bare-metal）** 环境中：
- 没有文件系统
- 没有操作系统提供的 stdin/stdout
- 默认的 printf/scanf 依赖 C 库的底层实现，如果链接了完整 C 库（如 glibc），会尝试调用系统调用，但在裸机环境下这些系统调用不存在，导致程序崩溃或无输出

### 1.3 printf/scanf 与文件描述符的关系

```
应用程序调用
    ↓
printf() / scanf()
    ↓
C 标准库实现
    ↓
_write() / _read()  ← 需要我们自己实现！
    ↓
硬件（UART）
```

`_write()` 和 `_read()` 是 C 标准库与底层硬件之间的桥梁函数。

---

## 2. printf/scanf 的底层原理（_write/_read 系统调用）

### 2.1 函数原型

```c
/* 文件描述符写入函数 */
int _write(int fd, char *ptr, size_t len);

/* 文件描述符读取函数 */
int _read(int fd, char *ptr, size_t len);
```

参数说明：
- `fd`: 文件描述符（0=stdin, 1=stdout, 2=stderr）
- `ptr`: 数据缓冲区指针
- `len`: 数据长度
- 返回值：实际写入/读取的字节数

### 2.2 printf 的调用链

```
printf("Hello %d\n", value);
    ↓
解析格式字符串 "%d"
    ↓
va_start() 启动可变参数
    ↓
格式化参数转换为字符串
    ↓
_write(fd=1, buffer, len)  ← 输出到 stdout
    ↓
_write() 重定向到 uart_putchar()
    ↓
UART 硬件发送字符
```

### 2.3 scanf 的调用链

```
scanf("%d", &value);
    ↓
解析格式字符串 "%d"
    ↓
va_start() 启动可变参数
    ↓
_read(fd=0, buffer, len)  ← 从 stdin 读取
    ↓
_read() 重定向到 uart_getchar()
    ↓
UART 硬件接收字符
    ↓
解析输入字符串，转换为目标类型
```

### 2.4 为什么要实现 _write/_read？

ARM GCC 编译裸机程序时：
- 使用 `float` 或 `double` 会链接 `libgloss`（或类似库）
- 库内部会调用 `_write()` 和 `_read()` 作为底层输出/输入函数
- 如果不实现这些函数，链接器会报 undefined symbol 错误，或运行时程序崩溃

---

## 3. 裸机环境下为什么需要重定向

### 3.1 裸机环境的特点

| 特性 | 有操作系统 | 裸机环境 |
|------|------------|----------|
| 文件系统 | 有 | 无 |
| 系统调用 | 有 | 无 |
| 标准库 | glibc | newlib/libgloss 或自定义 |
| 硬件访问 | 通过驱动 | 直接操作寄存器 |
| 输出设备 | 终端/显示器 | UART/调试串口 |

### 3.2 没有重定向的后果

1. **printf 无输出**：printf 内部调用 _write，但 _write 没有实现，程序死机或静默失败
2 **scanf 无法输入**：scanf 内部调用 _read，没有实现会导致程序挂起
3. **链接错误**：如果使用完整 C 库，可能报 undefined symbol 错误

### 3.3 重定向的价值

通过重定向，我们可以：

```c
/* 将标准输出重定向到 UART */
int _write(int fd, char *ptr, size_t len) {
    if (fd == 1 || fd == 2) {  // stdout 或 stderr
        for (size_t i = 0; i < len; i++) {
            uart_putchar(ptr[i]);  // 逐字符通过 UART 发送
        }
    }
    return len;
}
```

这样，所有 `printf()` 调用都会通过 UART 发送出去。

---

## 4. i.MX6UL UART 寄存器配置

### 4.1 i.MX6UL UART 简介

i.MX6UL 芯片集成了多个 UART 外设，本项目使用 **UART1**：

- 基础地址：`0x02020000`
- 支持 5/6/7/8 位数据位
- 1 或 2 位停止位
- 可选奇偶校验
- 最高支持 5 Mbps 波特率

### 4.2 关键寄存器列表

| 寄存器 | 偏移 | 名称 | 功能 |
|--------|------|------|------|
| UCR1 | 0x00 | 控制寄存器1 | UART 使能、睡眠等 |
| UCR2 | 0x04 | 控制寄存器2 | 数据位宽、停止位、奇偶校验、发送/接收使能 |
| UCR3 | 0x08 | 控制寄存器3 | FIFO 控制等 |
| UFCR | 0x0C | FIFO 控制寄存器 | FIFO 阈值设置 |
| USR1 | 0x10 | 状态寄存器1 | 各种状态标志 |
| USR2 | 0x14 | 状态寄存器2 | 发送完成、接收就绪等 |
| UTS | 0x1C | 测试寄存器 | 调试用 |
| UBIR | 0x28 | 波特率索引寄存器 | 波特率生成参数 |
| UBMR | 0x2C | 波特率匹配寄存器 | 波特率生成参数 |
| UTXD | 0x40 | 发送数据寄存器 | 写入要发送的数据 |
| URXD | 0x60 | 接收数据寄存器 | 读取接收的数据 |

### 4.3 初始化流程

```c
void uart_init(void)
{
    /* 1. 初始化 UART 时钟 */
    uart1_clk_init();
    
    /* 2. 配置 UART1 引脚复用 */
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0);
    
    /* 3. 软件复位 UART */
    UART1->UCR1 &= ~UART_UCR1_UARTEN_MASK;
    
    /* 4. 配置控制寄存器 */
    UART1->UCR1 = 0;
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;  // 使能 UART
    
    UART1->UCR2 = 0;
    UART1->UCR2 |= (1 << 5);   // WS: 8位数据宽度
    UART1->UCR2 |= (1 << 6);   // STPB: 1位停止位
    UART1->UCR2 &= ~(1 << 8);  // PREN: 禁用奇偶校验
    UART1->UCR2 |= (1 << 2);   // TXEN: 使能发送
    UART1->UCR2 |= (1 << 1);   // RXEN: 使能接收
    UART1->UCR2 |= (1 << 0);   // SRST: 软件复位
    
    UART1->UCR3 |= (1 << 2);    // 使能 FIFO
    
    /* 5. 设置波特率 */
    uart_set_baudrate(UART1, UART_CLK_FREQ, 115200);
    
    /* 6. 等待发送完成 */
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
}
```

### 4.4 关键寄存器位说明

#### UCR2 寄存器
| 位 | 名称 | 说明 |
|----|------|------|
| 0 | SRST | 软件复位（写1复位） |
| 1 | RXEN | 接收使能 |
| 2 | TXEN | 发送使位 |
| 5 | WS | 数据宽度：0=7位，1=8位 |
| 6 | STPB | 停止位：0=1位，1=2位 |
| 8 | PREN | 奇偶校验使能 |

#### USR2 寄存器（状态标志）
| 位 | 名称 | 说明 |
|----|------|------|
| 0 | RDR | 接收数据就绪 |
| 1 | ORE | 溢出错误 |
| 3 | TXDC | 发送完成（位移寄存器空） |

### 4.5 波特率计算

波特率计算公式：
```
Baud Rate = UART_CLK / (16 × (UBMR + 1) / (UBIR + 1))
```

重排后：
```
UBMR = UART_CLK × 1000 / (16 × BaudRate × (UBIR + 1)) - 1
```

对于 80MHz 时钟和 115200 波特率：
- UBIR = 71
- UBMR = 3124

```c
// 简化计算
UART1->UBIR = 71;
UART1->UBMR = 3124;
```

---

## 5. _write 和 _read 函数的实现细节

### 5.1 _write 实现

```c
/**
 * _write - Write to file descriptor (redirected to UART)
 * @fd: file descriptor (1 = stdout, 2 = stderr)
 * @ptr: buffer containing data to write
 * @len: number of bytes to write
 * 
 * Returns: number of bytes written, or -1 on error
 */
int _write(int fd, char *ptr, size_t len)
{
    size_t i;
    
    /* 只处理 stdout (fd=1) 和 stderr (fd=2) */
    if (fd != 1 && fd != 2) {
        return -1;
    }
    
    /* 逐字符写入 UART */
    for (i = 0; i < len; i++) {
        uart_putchar(ptr[i]);
        
        /* 处理换行符：添加回车 */
        if (ptr[i] == '\n') {
            uart_putchar('\r');
        }
    }
    
    return len;
}
```

**关键点**：
- 只处理 stdout(1) 和 stderr(2)，忽略其他文件描述符
- `\n` 转换为 `\r\n`（UART 串口需要）
- 返回实际写入的字节数

### 5.2 _read 实现

```c
/**
 * _read - Read from file descriptor (redirected to UART)
 * @fd: file descriptor (0 = stdin)
 * @ptr: buffer to store read data
 * @len: maximum number of bytes to read
 * 
 * Returns: number of bytes read, or -1 on error
 */
int _read(int fd, char *ptr, size_t len)
{
    size_t i;
    
    /* 只处理 stdin (fd=0) */
    if (fd != 0) {
        return -1;
    }
    
    /* 从 UART 读取字符，直到缓冲区满或收到换行符 */
    for (i = 0; i < len; i++) {
        char ch = uart_getchar();
        
        /* 回显字符 */
        uart_putchar(ch);
        
        /* 处理回车符：转换为换行符 */
        if (ch == '\r') {
            uart_putchar('\n');
            ptr[i] = '\n';
            return i + 1;
        }
        
        ptr[i] = ch;
        
        /* 遇到换行符返回 */
        if (ch == '\n') {
            return i + 1;
        }
    }
    
    return len;
}
```

**关键点**：
- 只处理 stdin(0)
- 实现字符回显（方便调试）
- `\r` 转换为 `\n`（统一行结束符）
- 遇到换行符或回车符结束输入

### 5.3 底层 UART 函数

```c
/* 发送一个字符 */
void uart_putchar(char c)
{
    /* 等待发送缓冲区为空（TXDC = 1）*/
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
    
    /* 写入数据到发送寄存器 */
    UART1->UTXD = c;
}

/* 接收一个字符 */
char uart_getchar(void)
{
    /* 等待数据就绪（CHARRDY = 1）*/
    while (!(UART1->URXD & UART_URXD_CHARRDY_MASK));
    
    /* 返回接收的数据（低8位）*/
    return (char)(UART1->URXD & 0xFF);
}
```

---

## 6. 最小化 printf 的实现要点

### 6.1 支持的格式符

本实现支持的格式符：

| 格式符 | 说明 | 示例 |
|--------|------|------|
| `%d` / `%i` | 有符号十进制整数 | printf("%d", -123) → "-123" |
| `%u` | 无符号十进制整数 | printf("%u", 255) → "255" |
| `%x` | 无符号十六进制（小写） | printf("%x", 0xDEAD) → "dead" |
| `%X` | 无符号十六进制（大写） | printf("%X", 0xDEAD) → "DEAD" |
| `%c` | 字符 | printf("%c", 'A') → "A" |
| `%s` | 字符串 | printf("%s", "hello") → "hello" |
| `%%` | 输出百分号 | printf("%%") → "%" |

### 6.2 核心实现逻辑

```c
int printf(const char *format, ...)
{
    va_list args;
    const char *p;
    int count = 0;
    
    va_start(args, format);
    
    for (p = format; *p != '\0'; p++) {
        if (*p != '%') {
            print_char(*p);  // 普通字符直接输出
            count++;
            continue;
        }
        
        p++;  // 跳过 '%'
        
        switch (*p) {
            case '%':
                print_char('%');
                break;
            case 'c': {
                char c = (char)va_arg(args, int);
                print_char(c);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                print_string(s);
                break;
            }
            case 'd':
            case 'i': {
                int num = va_arg(args, int);
                print_number(num, num < 0);
                break;
            }
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                print_number((int)num, false);
                break;
            }
            case 'x':
            case 'X': {
                unsigned int num = va_arg(args, unsigned int);
                print_hex(num, *p == 'X');
                break;
            }
        }
    }
    
    va_end(args);
    return count;
}
```

### 6.3 整数转字符串实现

```c
/* 输出有符号整数 */
static void print_number(int num, bool negative)
{
    char buffer[16];
    char *p = buffer + sizeof(buffer) - 1;
    unsigned int u;
    
    /* 处理负数：利用补码特性 */
    if (negative) {
        u = (unsigned int)(-(num + 1)) + 1;
    } else {
        u = (unsigned int)num;
    }
    
    if (u == 0) {
        print_char('0');
        return;
    }
    
    *p = '\0';
    while (u > 0) {
        *--p = '0' + (u % 10);
        u /= 10;
    }
    
    if (negative) {
        print_char('-');
    }
    
    while (*p) {
        print_char(*p++);
    }
}

/* 输出十六进制数 */
static void print_hex(unsigned int num, bool uppercase)
{
    char buffer[16];
    char *p = buffer + sizeof(buffer) - 1;
    static const char hex_lc[] = "0123456789abcdef";
    static const char hex_uc[] = "0123456789ABCDEF";
    const char *hex = uppercase ? hex_uc : hex_lc;
    
    if (num == 0) {
        print_string("0x0");
        return;
    }
    
    *p = '\0';
    while (num > 0) {
        *--p = hex[num % 16];
        num /= 16;
    }
    
    print_string("0x");
    while (*p) {
        print_char(*p++);
    }
}
```

### 6.4 不支持的功能（简化版）

本实现是简化版，不支持以下功能：

- 字段宽度（如 `%5d`）
- 精度（如 `%.2f`）
- 长度修饰符（如 `%ld`, `%lld`）
- 浮点数（`%f`, `%e`, `%g`）
- 指针（`%p`）
- 负号/零填充（如 `%05d`）

---

## 7. 最小化 scanf 的实现要点

### 7.1 支持的格式符

| 格式符 | 说明 | 示例 |
|--------|------|------|
| `%d` / `%i` | 有符号十进制整数 | scanf("%d", &num) |
| `%u` | 无符号十进制整数 | scanf("%u", &num) |
| `%x` | 无符号十六进制整数 | scanf("%x", &num) |
| `%c` | 单个字符 | scanf("%c", &ch) |
| `%s` | 字符串（空格分隔） | scanf("%s", str) |

### 7.2 核心实现逻辑

```c
int scanf(const char *format, ...)
{
    va_list args;
    int count = 0;
    char input_buffer[64];
    int input_len = 0;
    char ch;
    
    /* 1. 读取一行输入 */
    while (1) {
        ch = uart_getchar();
        uart_putchar(ch);  /* 回显 */
        
        if (ch == '\r' || ch == '\n') {
            uart_putchar('\n');
            break;
        }
        
        if (input_len < (int)(sizeof(input_buffer) - 1)) {
            input_buffer[input_len++] = ch;
        }
    }
    input_buffer[input_len] = '\0';
    
    /* 2. 解析输入 */
    va_start(args, format);
    const char *input_p = input_buffer;
    const char *p;
    
    for (p = format; *p != '\0' && *input_p != '\0'; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd':
                case 'i': {
                    int *num_ptr = va_arg(args, int *);
                    int parsed = 0;
                    int consumed = parse_number(input_p, &parsed);
                    if (consumed > 0) {
                        *num_ptr = parsed;
                        count++;
                        input_p += consumed;
                    }
                    break;
                }
                /* 其他格式符处理类似... */
            }
        }
    }
    
    va_end(args);
    return count;
}
```

### 7.3 数字解析实现

```c
static int parse_number(const char *s, int *result)
{
    int value = 0;
    int count = 0;
    bool negative = false;
    
    /* 处理负号 */
    if (*s == '-') {
        negative = true;
        s++;
        count++;
    }
    
    /* 确保第一个字符是数字 */
    if (*s < '0' || *s > '9') {
        return 0;
    }
    
    /* 解析数字 */
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
        count++;
    }
    
    *result = negative ? -value : value;
    return count;
}
```

### 7.4 十六进制解析

```c
/* 简单的十六进制解析 */
case 'x': {
    unsigned int *num_ptr = va_arg(args, unsigned int *);
    unsigned int value = 0;
    
    while ((*input_p >= '0' && *input_p <= '9') ||
           (*input_p >= 'a' && *input_p <= 'f') ||
           (*input_p >= 'A' && *input_p <= 'F')) {
        char c = *input_p;
        unsigned int digit;
        
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'f')
            digit = c - 'a' + 10;
        else
            digit = c - 'A' + 10;
        
        value = value * 16 + digit;
        input_p++;
    }
    
    *num_ptr = value;
    if (value != 0) count++;
    break;
}
```

---

## 8. 常见问题和调试方法

### 8.1 常见问题

#### 问题1：printf 无输出

可能原因：
1. UART 未正确初始化（时钟、引脚、寄存器）
2. _write 函数未正确实现或未链接
3. 波特率不匹配
4. 串口助手设置错误（波特率/数据位/停止位/校验位）

**排查方法**：
```c
/* 先用最底层函数测试 */
uart_init();
uart_putchar('A');  // 直接发送一个字符测试
```

#### 问题2：scanf 阻塞（程序卡死）

可能原因：
1. uart_getchar() 中的等待死循环
2. 串口未发送数据
3. UART 接收引脚配置错误

**排查方法**：
```c
/* 检查接收就绪标志 */
while (!(UART1->URXD & UART_URXD_CHARRDY_MASK));
```

#### 问题3：输出乱码

可能原因：
1. 波特率不匹配（最常见）
2. 数据位/停止位/校验位设置不一致

**排查方法**：
- 确认上位机波特率：115200
- 确认 8N1 设置（8位数据，无校验，1位停止位）

#### 问题4：链接错误 undefined reference to `_write`

可能原因：
1. 使用了完整 C 库但未实现系统调用
2. 链接脚本未正确配置

**解决方案**：
- 实现 `_write` 和 `_read` 函数
- 或使用简化版 C 库（newlib）

### 8.2 调试方法

#### 1. 基础测试：单字符输出

```c
void debug_test(void)
{
    uart_init();
    uart_putchar('O');
    uart_putchar('K');
    uart_putchar('\r');
    uart_putchar('\n');
}
```

#### 2. 字符串测试

```c
void debug_test2(void)
{
    uart_init();
    uart_puts("Debug test OK\r\n");
}
```

#### 3. printf 测试

```c
void debug_test3(void)
{
    printf("Integer: %d\r\n", 123);
    printf("Hex: 0x%x\r\n", 0xDEAD);
    printf("String: %s\r\n", "Hello");
}
```

#### 4. 检查寄存器

```c
void uart_debug_status(void)
{
    printf("USR1: 0x%X\r\n", UART1->USR1);
    printf("USR2: 0x%X\r\n", UART1->USR2);
    printf("UCR1: 0x%X\r\n", UART1->UCR1);
    printf("UCR2: 0x%X\r\n", UART1->UCR2);
}
```

### 8.3 调试技巧

1. **先验证硬件**：确保 UART 硬件工作正常
2. **分层测试**：从底层到上层逐层测试
3. **回显功能**：实现字符回显，便于观察输入
4. **缓冲区溢出保护**：在 scanf 中限制输入长度
5. **换行符处理**：注意 `\n` 和 `\r` 的转换

---

## 附录：完整代码结构

```
project/
├── main.c                    # 主程序入口
└── bsp/
    ├── stdio/
    │   ├── bsp_stdio.c       # printf/scanf 实现
    │   └── bsp_stdio.h
    └── uart/
        ├── bsp_uart.c        # UART 驱动
        └── bsp_uart.h
```

---

*文档基于 i.MX6UL 裸机开发环境编写*
