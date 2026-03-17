#include "bsp_epittimer.h"
#include "bsp_int.h"
#include "bsp_led.h"


void epit1_init(unsigned int frac, unsigned int value)
{
    // 首先清零CR寄存器
    if (frac > 0xFFF)
        frac = 0xFFF;
    EPIT1->CR = 0;

    // CR寄存器需要再重新设置
    // bit 25:24  01 时钟源选择Peripheral clock=66MHz
    // bit 15:4   frac 分频值传入
    // bit 3      1 set-and-forget模式，当计数器到0需要从LR重新加载数值
    // bit 2      1 比较中断使能
    // bit 1      1 初始值来源于LR寄存器值
    // bit 0      0 先关闭EPIT1
    EPIT1->CR = (1 << 24 | frac << 4 | 1 << 3 | 1 << 2 | 1 << 1);
    EPIT1->LR = value;
    EPIT1->CMPR = 0;

    // 使能对应中断
    GIC_EnableIRQ(EPIT1_IRQn);

    system_register_irqhandler(EPIT1_IRQn,
        (system_irq_handler_t) epit1_irqhandler,
        NULL);

    EPIT1->CR |= 1;  // 在这里开始使能EPIT
}

void epit1_irqhandler()
{
    static unsigned char state = 0;
    state = !state;
    if(EPIT1->SR & 1) {                // 当比较事件发生时
        led_switch(LED0, state);       // 周期定时器到了，反转LED
    }

    EPIT1->SR |= 1 << 0;               // 清除中断标志位
}