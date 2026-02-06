#include "bsp_key.h"
#include "bsp_gpio.h"
#include "bsp_clk.h"
#include "bsp_led.h"
#include "bsp_delay.h"

void key_init(void)
{
    gpio_pin_config_t key_config;
    // 配置GPIO 1(教程里是UART CTS)
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0U);
    /*
        bit 16:0 HYS关闭
        bit [15:14]:11 默认22k上拉
        bit [13]:1     pull功能
        bit [12]:1     pull/keeper使能
        bit [11]:0     关闭开路输出
        bit [7:6]:10   速度100MHz
        bit [5:3]:000  关闭输出
        bit [0]:0      低转换率
    */
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);
    key_config.direction = kGPIO_DigitalInput;
    // 配置输入
    gpio_init(GPIO1, 18, &key_config);
}

int key_get_value(void)
{
    int ret = KEY_NONE;
    static u8 release = 1;   /* 1表示高电平，这个时候按键松开 */

    // 检测按下，低电表示按下了
    if ((release == 1) && gpio_read(GPIO1, 18) == 0) {
        // 进行延时检测，对于嵌入式来讲，延时也是很关键的一步
        // 如何计算这个延时也需要再记录一下
        delay(10);
        release = 0;
        if (gpio_read(GPIO1, 18) == 0)
            ret = KEY_VALUE;
    } else if (gpio_read(GPIO1, 18) == 1) {
        // 按键松开，标志位重置
        ret = KEY_NONE;
        release = 1;
    }

    return ret;
}