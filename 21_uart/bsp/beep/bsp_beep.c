#include "bsp_beep.h"

void beep_init(void)
{
    // 配置SNVS TAMPER1复用为GPIO5_IO01
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0U);
    // 配置电气特性
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0x10B0);
    // 设置GPIO5_IO01为输出模式
    GPIO5->GDIR |= (1 << 1); // 设置GPIO5_IO01
    // 与LED保持一致，默认打开
    GPIO5->DR &= ~(1 << 1); // 设置GPIO5_IO01输出低电平，使能蜂鸣器
}


void beep_on(void)
{
    GPIO5->DR &= ~(1 << 1); // 设置GPIO5_IO01输出低电平，使能蜂鸣器
}

void beep_off(void)
{
    GPIO5->DR |= (1 << 1); // 设置GPIO5_IO01输出高电平，关闭蜂鸣器
}

void beep_switch(int status)
{
    if (status == ON)
        beep_on();
    else
        beep_off();
}
