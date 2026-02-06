// 使用SDK的方式重构LED灯裸机驱动实验
#include "bsp_led.h"
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_beep.h"

// 配置，灯亮蜂鸣器响，灯灭蜂鸣器不响
int main(void)
{
    // 2.初始化时钟
    clk_init();

    // 2.初始化LED灯和蜂鸣器
    led_init();
    beep_init();

    // 3.循环点亮LED灯
    while (1)
    {
        led_switch(LED0, ON);   // 点亮LED灯
        beep_switch(ON);        // 打开蜂鸣器
        delay(1000); // 延时1秒
        led_switch(LED0, OFF);  // 熄灭LED灯
        beep_switch(OFF);       // 关闭蜂鸣器
        delay(1000); // 延时1秒
    }
    
    return 0;
}
