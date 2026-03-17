# IMX6ULL GPIO 驱动解析

## 一、GPIO 基础知识

### 1. 什么是 GPIO？

**GPIO (General Purpose Input/Output)** - 通用输入输出接口，是嵌入式系统中常用的外设。

- **输入模式**：读取外部信号（如按键、传感器）
- **输出模式**：控制外部设备（如 LED、继电器）

### 2. IMX6ULL GPIO 特性

IMX6ULL 共有 5 组 GPIO（GPIO1 ~ GPIO5），每组 32 个引脚：

| GPIO 组 | 引脚数量 | 基地址 |
|---------|----------|--------|
| GPIO1 | 32 | 0x0209C000 |
| GPIO2 | 32 | 0x020A0000 |
| GPIO3 | 32 | 0x020A4000 |
| GPIO4 | 32 | 0x020A8000 |
| GPIO5 | 32 | 0x020AC000 |

### 3. GPIO 关键寄存器

每个 GPIO 组包含以下核心寄存器：

| 寄存器 | 功能 |
|--------|------|
| **DR** | 数据寄存器：读取输入电平 / 设置输出电平 |
| **GDIR** | 方向寄存器：配置输入(0)或输出(1) |
| **PSR** | _pad status register_：读取引脚状态 |
| **ICR1/ICR2** | 中断控制寄存器：配置中断触发方式 |
| **IMR** | 中断屏蔽寄存器：使能/禁用中断 |
| **ISR** | 中断状态寄存器：清除中断标志 |
| **EDGE_SEL** | 边沿选择寄存器：选择双边沿触发 |

---

## 二、项目中的 GPIO 代码解析

### 1. GPIO 驱动头文件 (bsp_gpio.h)

```c
typedef enum _gpio_pin_direction {
    kGPIO_DigitalInput = 0U,   // 输入模式
    kGPIO_DigitalOutput = 1U   // 输出模式
} gpio_pin_direction_t;

// GPIO 中断触发类型
typedef enum _gpio_interrupt_mode {
    kGPIO_NoIntMode = 0U,          // 禁用中断
    kGPIO_IntLowLevel = 1U,        // 低电平触发
    kGPIO_IntHighLevel = 2U,       // 高电平触发
    kGPIO_IntRisingEdge = 3U,      // 上升沿触发
    kGPIO_IntFallingEdge = 4U,     // 下降沿触发
    kGPIO_IntRisingFallingEdge = 5U // 双边沿触发
} gpio_pin_config_t;

// 引脚配置结构体
typedef struct _gpio_pin_config {
    gpio_pin_direction_t direction;      // 输入/输出方向
    uint8_t outputLogic;                  // 输出电平（高/低）
    gpio_interrupt_mode_t interruptMode;  // 中断模式
} gpio_pin_config_t;
```

---

### 2. GPIO 初始化 (gpio_init)

```c
void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    // 根据 direction 配置输入/输出模式
    if (config->direction == kGPIO_DigitalOutput) {
        base->GDIR |= (1 << pin);    // 设置为输出模式
        gpio_write(base, pin, config->outputLogic);
    } else if (config->direction == kGPIO_DigitalInput) {
        base->GDIR &= ~(1 << pin);   // 设置为输入模式
    }
    
    // 配置中断
    gpio_intconfig(base, pin, config->interruptMode);
}
```

**解析：**

| 步骤 | 操作 | 寄存器 |
|------|------|--------|
| 1 | 设置 GPIO 方向 | GDIR |
| 2 | 写初始输出电平（输出模式）| DR |
| 3 | 配置中断模式 | ICR1/ICR2, EDGE_SEL |

---

### 3. GPIO 读/写

**读取引脚电平：**
```c
int gpio_read(GPIO_Type *base, int pin)
{
    return (base->DR >> pin) & 0x1;  // 读取 DR 寄存器的对应位
}
```

**写入引脚电平：**
```c
void gpio_write(GPIO_Type *base, int pin, int value)
{
    if (value == 0U) {
        base->DR &= ~(1 << pin);  // 输出低电平
    } else {
        base->DR |= (1 << pin);   // 输出高电平
    }
}
```

---

### 4. GPIO 中断配置

```c
void gpio_intconfig(GPIO_Type *base, unsigned int pin, gpio_interrupt_mode_t pin_int_mode)
{
    volatile uint32_t *icr;
    uint32_t icrShift;

    // 清除边沿选择（使用 ICR）
    base->EDGE_SEL &= ~(1U << pin);

    // ICR1 管理 IO0-15，ICR2 管理 IO16-31
    if (pin < 16) {
        icr = &(base->ICR1);
    } else {
        icr = &(base->ICR2);
        icrShift -= 16;
    }

    // 配置中断触发方式
    switch (pin_int_mode) {
    case kGPIO_IntLowLevel:      // 00
        *icr &= ~(3U << (2 * icrShift));
        break;
    case kGPIO_IntHighLevel:      // 01
        *icr = (*icr & (~(3U << (2 * icrShift)))) | (1U << (2 * icrShift));
        break;
    case kGPIO_IntRisingEdge:    // 10
        *icr = (*icr & (~(3U << (2 * icrShift)))) | (2U << (2 * icrShift));
        break;
    case kGPIO_IntFallingEdge:  // 11
        *icr |= (3U << (2 * icrShift));
        break;
    case kGPIO_IntRisingFallingEdge:
        base->EDGE_SEL |= (1U << pin);  // 使用 EDGE_SEL 寄存器
        break;
    default:
        break;
    }
}
```

**ICR 寄存器位定义：**

| bit[1:0] | 触发方式 |
|----------|----------|
| 00 | 低电平 |
| 01 | 高电平 |
| 10 | 上升沿 |
| 11 | 下降沿 |

---

### 5. GPIO 中断使能/禁用/清除

```c
// 使能中断
void gpio_enableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR |= (1 << pin);  // IMR 寄存器对应位置 1
}

// 禁用中断
void gpio_disableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR &= ~(1 << pin); // IMR 寄存器对应位清 0
}

// 清除中断标志
void gpio_clearintflags(GPIO_Type *base, unsigned int pin)
{
    base->ISR |= (1 << pin);  // 写 1 清除中断标志
}
```

---

## 三、实战：LED 驱动解析

### 1. 硬件连接

根据正点原子 IMX6ULL 开发板：

| LED | GPIO | 引脚 |
|-----|------|------|
| LED0 | GPIO1_IO03 | 复用为 GPIO |

**电路分析：**
- LED 低电平点亮：当 GPIO 输出低电平时，LED 导通发光
- LED 高电平熄灭：当 GPIO 输出高电平时，LED 截止

---

### 2. LED 初始化代码解析

```c
void led_init(void)
{
    /* 1. 配置引脚复用为 GPIO 功能 */
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0U);

    /* 2. 配置引脚电气属性 */
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);

    /* 3. 设置 GPIO1_IO03 为输出模式 */
    GPIO1->GDIR |= (1 << 3);  // bit3 = 1，配置为输出

    /* 4. 默认点亮 LED（输出低电平）*/
    GPIO1->DR &= ~(1 << 3);   // bit3 = 0，输出低电平
}
```

**步骤解析：**

| 步骤 | 代码 | 说明 |
|------|------|------|
| 1 | IOMUXC_SetPinMux | 将引脚复用为 GPIO 功能 |
| 2 | IOMUXC_SetPinConfig | 配置引脚电气属性（上下拉、驱动能力等）|
| 3 | GPIO1->GDIR \|= (1<<3) | 设置为输出模式 |
| 4 | GPIO1->DR &= ~(1<<3) | 输出低电平，点亮 LED |

---

### 3. LED 控制

```c
void led_on(void)
{
    GPIO1->DR &= ~(1 << 3);  // 输出低电平，点亮 LED
}

void led_off(void)
{
    GPIO1->DR |= (1 << 3);   // 输出高电平，熄灭 LED
}
```

---

## 四、IOMUXC 引脚复用

### 1. 什么是 IOMUXC？

IOMUXC (IOMUX Controller) 负责管理引脚的复用功能。一个物理引脚可以配置为多种功能：

- GPIO
- UART
- I2C
- SPI
- PWM
- 等等...

### 2. 配置步骤

**第一步：设置复用功能 (SetPinMux)**

```c
IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0U);
```

参数说明：
- `IOMUXC_GPIO1_IO03_GPIO1_IO03` - 引脚功能宏定义
- `0U` -SION (Software Input On) 配置，通常为 0

**第二步：设置电气属性 (SetPinConfig)**

```c
IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);
```

`0x10B0` 各位含义：

| 位 | 含义 |
|----|------|
| bit[5:3] = 111 | DSE = 000 (禁用) / 110 (50Ω) / 101 (100Ω) / 011 (200Ω) / 001 (330Ω) |
| bit[6] = 1 | SRE = 0 (慢) / 1 (快) |
| bit[7] = 1 | SPEED = 100MHz |
| bit[11:8] = 1010 | PKE/PUE/PUS 等配置 |
| bit[12] = 1 | PKE = 1 (使能上下拉) |

---

## 五、GPIO 寄存器映射汇总

### GPIO 寄存器地址（以 GPIO1 为例）

| 寄存器 | 地址偏移 | 功能 |
|--------|----------|------|
| DR | 0x00 | 数据寄存器 |
| GDIR | 0x04 | 方向寄存器 |
| PSR | 0x08 | _pad status_ |
| ICR1 | 0x0C | 中断控制寄存器 1 (IO0-15) |
| ICR2 | 0x10 | 中断控制寄存器 2 (IO16-31) |
| IMR | 0x14 | 中断屏蔽寄存器 |
| ISR | 0x18 | 中断状态寄存器 |
| EDGE_SEL | 0x1C | 边沿选择寄存器 |

---

## 六、总结

### GPIO 开发流程

```
1. 配置 IOMUXC 引脚复用
   └─> 设置为 GPIO 功能

2. 配置引脚电气属性
   └─> 上下拉、驱动能力等

3. 初始化 GPIO
   └─> 设置方向（输入/输出）
   └─> 配置中断（可选）

4. 读写 GPIO
   └─> 输出：操作 DR 寄存器
   └─> 输入：读取 DR/PSR 寄存器

5. 中断处理（可选）
   └─> 使能 IMR
   └─> 编写中断处理函数
   └─> 清除 ISR 标志
```

### 本项目 GPIO 文件

| 文件 | 功能 |
|------|------|
| bsp/gpio/bsp_gpio.h | GPIO 驱动头文件 |
| bsp/gpio/bsp_gpio.c | GPIO 驱动实现 |
| bsp/led/bsp_led.c | LED 驱动（使用 GPIO）|
