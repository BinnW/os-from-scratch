#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "imx6ul.h"

// UART configuration parameters
#define UART_BAUDRATE    115200  // Baud rate
#define UART_CLK_FREQ    80000000 // UART clock frequency: pll3_80M = 80MHz

// Function declarations
void uart_init(void);
void uart_enable(void);
void uart_disable(void);
void uart_softreset(void);
void uart_putchar(char c);
char uart_getchar(void);
void uart_puts(const char *str);

#endif
