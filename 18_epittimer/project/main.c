// 使用SDK的方式重构LED灯裸机驱动实验
#include "bsp_led.h"
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_int.h"
#include "bsp_exit.h"
#include "bsp_epittimer.h"

// 配置，灯亮蜂鸣器响，灯灭蜂鸣器不响
int main(void)
{
    // 表示beep状态
    unsigned char state = OFF;

    int_init();
    imx6u_clkinit();
    clk_init();
    led_init();
    beep_init();
    key_init();
    exit_init();
    epit1_init(0, 66000000/2);  //表示1分频，计数值为66000000/2，也就是500ms

    while (1) {
        delay(500);
    }

    return 0;
}
