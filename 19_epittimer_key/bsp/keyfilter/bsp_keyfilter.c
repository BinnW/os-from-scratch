#include "bsp_key.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_beep.h"
#include "bsp_keyfilter.h"


void filterkey_init(void)
{
    gpio_pin_config_t key_config;
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    // 将该GPIO初始化为中断
    key_config.direction = kGPIO_DigitalInput;
    key_config.interruptMode = kGPIO_IntFallingEdge;
    key_config.outputLogic = 1;
    gpio_init(GPIO1, 18, &key_config);

    // 需要使能GPIO中断，注册中断处理函数
    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn,
        (system_irq_handler_t)gpio1_16_31_irqhandler,
        NULL);

    gpio_enableint(GPIO1, 18);      // 使能GPIO1_IO18的功能
    filtertimer_init(66000000/100); // 初始化为10ms定时器
}

void filtertimer_init(unsigned int value)
{
    EPIT1->CR = 0;
    EPIT1->CR = (1 << 24 | 1 << 3 | 1 << 2 | 1 << 1);
    EPIT1->LR = value;
    EPIT1->CMPR = 0;

    // 使能中断
    GIC_EnableIRQ(EPIT1_IRQn);

    system_register_irqhandler(EPIT1_IRQn,
        (system_irq_handler_t) filtertimer_irqhandler,
        NULL);
}

void filtertimer_stop(void)
{
    EPIT1->CR &= ~(1 << 0);
}

void filtertimer_restart(unsigned int value)
{
    filtertimer_stop();
    EPIT1->LR = value;
    EPIT1->CR |= 1;
}

void filtertimer_irqhandler()
{
    // 定时器到了之后翻转
    static unsigned char state = OFF;

    if (EPIT1->SR & 1) {
        filtertimer_stop();
        if (gpio_read(GPIO1, 18) == 0) {
            state = !state;
            beep_switch(state);
        }
    }

    EPIT1->SR |= 1 << 0;
}

void gpio1_16_31_irqhandler()
{
    filtertimer_restart(66000000/100);
    gpio_clearintflags(GPIO1, 18); // 清除中断标志位？表示中断处理结束？
}

