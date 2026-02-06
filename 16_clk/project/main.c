// 使用SDK的方式重构LED灯裸机驱动实验
#include "bsp_led.h"
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_beep.h"
#include "bsp_key.h"

// 配置，灯亮蜂鸣器响，灯灭蜂鸣器不响
int main(void)
{
    // 按键打开/关闭蜂鸣器，LED灯闪烁
    int count;
    u8 led_status = OFF;
    u8 beep_status = OFF;
    int key_value;

    imx6u_clkinit();
    clk_init();
    led_init();
    // beep_init();
    key_init();

    while (1) {
        key_value = key_get_value();
        switch (key_value) {
            case KEY_VALUE:
                // 按键按下，改变beep的状态
                beep_status = !beep_status;
                beep_switch(beep_status);
                break;
        }

        count++;
        if (count == 50) {
            count = 0;
            led_status = !led_status;
            led_switch(LED0, led_status);
        }
        delay(10);
    }

    
    return 0;
}
