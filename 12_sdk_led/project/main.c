// 使用SDK的方式重构LED灯裸机驱动实验
#include "bsp_led.h"
#include "bsp_clk.h"
#include "bsp_delay.h"

int main(void)
{
    // 2.初始化时钟
    clk_init();

    // 2.初始化LED灯
    led_init();

    // 3.循环点亮LED灯
    while (1)
    {
        led_switch(LED0, ON);   // 点亮LED灯
        delay(1000); // 延时1秒
        led_switch(LED0, OFF);  // 熄灭LED灯
        delay(1000); // 延时1秒
    }

    return 0;
}
