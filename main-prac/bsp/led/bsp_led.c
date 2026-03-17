#include "bsp_led.h"


void led_init(void)
{
    // 2.配置GPIO1_IO03引脚复用为GPIO功能
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0U);

    // 3.配置GPIO1_IO03引脚的上拉电阻
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);

    // 4.设置GPIO1_IO03为输出模式
    GPIO1->GDIR |= (1 << 3); // 设置GPIO1_IO03为输出模式

    // 默认打开
    GPIO1->DR &= ~(1 << 3); // 设置GPIO1_IO03输出低电平，点亮LED

    return;
}


void led_on(void)
{
    GPIO1->DR &= ~(1 << 3); // 设置GPIO1_IO03输出低电平，点亮LED
    return;
}


void led_off(void)
{
    GPIO1->DR |= (1 << 3); // 设置GPIO1_IO03输出高电平，熄灭LED
    return;
}


void led_switch(int led, int status)
{
    switch (led)
    {
    case LED0:
        if (status == ON)
            led_on();
        else
            led_off();
        break;
    default:
        break;
    }
}