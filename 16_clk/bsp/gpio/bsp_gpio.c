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