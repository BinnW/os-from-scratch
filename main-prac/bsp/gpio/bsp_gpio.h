#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "imx6ul.h"

typedef enum _gpio_pin_direction {
    kGPIO_DigitalInput = 0U,
    kGPIO_DigitalOutput = 1U
} gpio_pin_direction_t;

// GPIO中断触发类型
typedef enum _gpio_interrupt_mode {
    kGPIO_NoIntMode = 0U,
    kGPIO_IntLowLevel = 1U,
    kGPIO_IntHighLevel = 2U,
    kGPIO_IntRisingEdge = 3U,
    kGPIO_IntFallingEdge = 4U,
    kGPIO_IntRisingFallingEdge = 5U,
} gpio_interrupt_mode_t;

typedef struct _gpio_pin_config {
    gpio_pin_direction_t direction;
    uint8_t outputLogic;        // 高低电平
    gpio_interrupt_mode_t interruptMode; // GPIO pin脚使用的中断方式
} gpio_pin_config_t;

void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config);
int gpio_read(GPIO_Type *base, int pin);
void gpio_write(GPIO_Type *base, int pin, int value);

void gpio_intconfig(GPIO_Type *base, unsigned int pin, gpio_interrupt_mode_t pinInterruptMode);
void gpio_enableint(GPIO_Type *base, unsigned int pin);
void gpio_disableint(GPIO_Type *base, unsigned int pin);
void gpio_clearintflags(GPIO_Type *base, unsigned int pin);

#endif