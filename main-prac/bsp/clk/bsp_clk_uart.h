#ifndef __BSP_CLK_UART_H
#define __BSP_CLK_UART_H

#include "imx6ul.h"

// UART1 clock initialization
// Select pll3_80m clock source, 1 divider
void uart1_clk_init(void);

#endif
