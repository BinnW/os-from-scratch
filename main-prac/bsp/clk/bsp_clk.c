#include "bsp_clk.h"


void clk_init(void)
{
    CCM->CCGR0 = 0xffffffff;
    CCM->CCGR1 = 0xffffffff;
    CCM->CCGR2 = 0xffffffff;
    CCM->CCGR3 = 0xffffffff;
    CCM->CCGR4 = 0xffffffff;
    CCM->CCGR5 = 0xffffffff;
    CCM->CCGR6 = 0xffffffff;
}


void imx6u_clkinit(void)
{
    unsigned int reg = 0;
    // 先设置PLL1
    // 判断当前的时钟是哪个模式, 为0表示使用的是main_clk而不是step_clk
    if (((CCM->CCSR >> 2) & 0x1) == 0) {
        CCM->CCSR &= ~(1 << 8); // step_clk选择24MHz
        CCM->CCSR |= (1 << 2);  // 选择step_clk
    }
    // 接下来是设置PLL1的频率，13位是enable，然后设置88为div_select
    CCM_ANALOG->PLL_ARM = (1 << 13) | ((88 < 0) & 0x7F);
    // 切换回来，到main_clk
    CCM->CCSR &= ~(1 << 2);
    // 设置二分频
    CCM->CACRR |= 1;

    // 随后设置PLL2的各个PFD，到NXP推荐的频率上去
    // CCM_ANALOG->PFD_528 |= (1 << 7) | (1 << 15) | (1 << 23) | (1 << 31);
    // CCM_ANALOG->PFD_528 |= (27 & 0x3F) << 0;
    // CCM_ANALOG->PFD_528 |= (16 & 0x3F) << 8;
    // CCM_ANALOG->PFD_528 |= (24 & 0x3F) << 16;
    // CCM_ANALOG->PFD_528 |= (32 & 0x3F) << 24;
    // 对于时钟的设置不要再继续直接操作寄存器了，一下全部赋值
    reg = CCM_ANALOG->PFD_528;
    reg &= ~(0x3F3F3F3F);
    reg |= (27 & 0x3F) << 0;
    reg |= (16 & 0x3F) << 8;
    reg |= (24 & 0x3F) << 16;
    reg |= (32 & 0x3F) << 24;
    CCM_ANALOG->PFD_528 = reg;


    // 然后设置PLL3
    reg = CCM_ANALOG->PFD_480;
    reg &= ~(0x3F3F3F3F);
    reg |= 12;
    reg |= 16 << 8;
    reg |= 17 << 16;
    reg |= 19 << 24;
    CCM_ANALOG->PFD_480 = reg;

    // 最后设置AHB，最小6MHz，最大132MHz
    CCM->CBCMR &= ~(3 << 18);
    CCM->CBCMR |= (1 << 18);
    CCM->CBCDR &= ~(1 << 25);
    while (CCM->CDHIPR & (1 << 5));


    // 最后设置IPG_CLK_ROOT最小3MHz，最大66MHz
    CCM->CBCDR &= ~(3 << 8);
    CCM->CBCDR |= (1 << 8);

    // 设置PRECLK_CLK_ROOT时钟
    CCM->CSCMR1 &= ~(1 << 6);
    CCM->CSCMR1 &= ~(7 << 0);

}