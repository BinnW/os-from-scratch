#include "bsp_delay.h"


void delay_short(uint32_t count)
{
    volatile uint32_t i;

    for (i = 0; i < count; i++)
    {
        __asm("nop");
    }
}


void delay(uint32_t count)
{
    volatile uint32_t i;

    for (i = 0; i < count; i++)
    {
        delay_short(0x7ff); // 每次短延时1000个nop指令
    }
}