#include "bsp_exit.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_beep.h"

void exit_init(void)
{
    // 还是先把GPIO初始化了，把输入输出以及中断方式都配置好
    gpio_pin_config_t key_config;

    // 设置IO复用，GPIO1 IO18
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    // 初始化GPIO中断模式
    key_config.direction = kGPIO_DigitalInput;
    key_config.interruptMode = kGPIO_IntFallingEdge;
    key_config.outputLogic = 1;
    gpio_init(GPIO1, 18, &key_config);

    // 使能GIC中断、注册中断服务函数、使能GPIO中断
    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn,
        (system_irq_handler_t)gpio1_io18_irqhandler,
        NULL);
    gpio_enableint(GPIO1, 18);
}

void gpio1_io18_irqhandler(void)
{
    static unsigned char state = 0;

    // 延时消抖
    // 一般中断服务函数中禁止使用延时函数，因为中断服务需要“快进快出”
    delay(10);
    // 表示按键暗下去了
    if (gpio_read(GPIO1, 18) == 0) {
        state = !state;
        beep_switch(state);
    }

    gpio_clearintflags(GPIO1, 18);
}
