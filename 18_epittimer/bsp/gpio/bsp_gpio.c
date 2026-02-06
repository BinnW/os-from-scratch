#include "bsp_gpio.h"


void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    // 根据direction配置输出输出，然后再进行read或write
    if (config->direction == kGPIO_DigitalOutput) {
        base->GDIR |= (1 << pin);
        gpio_write(base, pin, config->outputLogic);
    } else if (config->direction == kGPIO_DigitalInput) {
        base->GDIR &= ~(1 << pin);
    }
    gpio_intconfig(base, pin, config->interruptMode);
    return;
}

int gpio_read(GPIO_Type *base, int pin)
{
    // 读取的是高低电平，作为输入时就需要读取
    return (base->DR >> pin) & 0x1;
}

void gpio_write(GPIO_Type *base, int pin, int value)
{
    // 配置输出高低电平
    if (value == 0U) {
        base->DR &= ~(1 << pin);
    } else if (value == 1U) {
        base->DR |= 1 << pin;
    }
}

void gpio_intconfig(GPIO_Type *base, unsigned int pin, gpio_interrupt_mode_t pin_int_mode)
{
    volatile uint32_t *icr;
    uint32_t icrShift;

    icrShift = pin;

    base->EDGE_SEL &= ~(1U << pin);

    if (pin < 16) {
        icr = &(base->ICR1);
    } else {
        icr = &(base->ICR2);
        icrShift -= 16;
    }

    // 操作寄存器
    switch (pin_int_mode) {
    case kGPIO_IntLowLevel:
        *icr &= ~(3U << (2 * icrShift));
        break;
    case kGPIO_IntHighLevel:
        *icr = (*icr & (~(3U << (2 * icrShift)))) | (1U << (2 * icrShift));
        break;
    case kGPIO_IntRisingEdge:
        *icr = (*icr & (~(3U << (2 * icrShift)))) | (2U << (2 * icrShift));
        break;
    case kGPIO_IntFallingEdge:
        *icr |= (3U << (2 * icrShift));
        break;
    case kGPIO_IntRisingFallingEdge:
        base->EDGE_SEL |= (1U << pin);
        break;
    default:
        break;
    }
}

// 中断使能
void gpio_enableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR |= (1 << pin);
}


// 禁止GPIO中断功能
void gpio_disableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR &= ~(1 << pin);
}

// 清除中断标志位
void gpio_clearintflags(GPIO_Type *base, unsigned int pin)
{
    base->ISR |= (1 << pin);
}