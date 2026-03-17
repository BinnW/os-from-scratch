#include "bsp_clk_uart.h"

// UART1 clock initialization
// Select pll3_80m clock source, 1 divider
void uart1_clk_init(void)
{
    // Enable UART1 clock - CCGR1[13:10]
    CCM->CCGR1 |= (0xF << 10);

    // Select UART clock source as pll3_80m
    // CSCMR1[15] = 0: select pll3_80m
    CCM->CSCMR1 &= ~(1 << 15);

    // Set UART clock divider to 1
    // CSCDR1[5:3] = 000: 1 divider
    CCM->CSCDR1 &= ~(7 << 3);
}
