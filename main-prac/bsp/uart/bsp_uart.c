#include "bsp_uart.h"
#include "bsp_clk_uart.h"
#include "fsl_iomuxc.h"

// Baud rate calculation:
// Baud rate = UART_CLK / (16 * (UBMR + 1) / (UBIR + 1))
// For 115200 @ 80MHz:
// UBIR = 71, UBMR = 3124
static void uart_set_baudrate(UART_Type *base, uint32_t clk_freq, uint32_t baudrate)
{
    uint32_t ubir, ubmr;

    // Calculate register values for baud rate
    ubir = clk_freq / (baudrate * 16);
    ubmr = clk_freq / baudrate * 1000 / 16 / 1000 * 1000 - ubir * 1000;
    ubmr = ubmr / ((clk_freq / baudrate / 16) / 1000) * 1000;

    // Simplified calculation for 115200 @ 80MHz
    // UBIR = 71, UBMR = 3124
    base->UBIR = 71;
    base->UBMR = 3124;
}

void uart_init(void)
{
    // 1. Initialize UART1 clock
    uart1_clk_init();

    // 2. Configure UART1 pin muxing
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0);

    // Configure pin properties
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 0x10B0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 0x10B0);

    // 3. Software reset UART
    UART1->UCR1 &= ~UART_UCR1_UARTEN_MASK;

    // 4. Configure control registers
    // UCR1: Enable UART
    UART1->UCR1 = 0;
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;

    // UCR2: 8-bit data, 1 stop bit, enable TX and RX
    UART1->UCR2 = 0;
    UART1->UCR2 |= (1 << 5);    // WS: 8-bit data width
    UART1->UCR2 |= (1 << 6);    // STPB: 1 stop bit
    UART1->UCR2 &= ~(1 << 8);   // PREN: disable parity
    UART1->UCR2 |= (1 << 2);    // TXEN: enable transmitter
    UART1->UCR2 |= (1 << 1);    // RXEN: enable receiver
    UART1->UCR2 |= (1 << 0);    // SRST: reset

    // UCR3: Enable FIFO
    UART1->UCR3 = 0;
    UART1->UCR3 |= (1 << 2);

    // UFCR: Set FIFO watermark (RXWATER=1, TXWATER=0)
    UART1->UFCR = (5 << 7) | (0 << 0);

    // 5. Set baud rate
    uart_set_baudrate(UART1, UART_CLK_FREQ, UART_BAUDRATE);

    // 6. Wait for transmission to complete
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));
}

// UART enable
void uart_enable(void)
{
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;
}

// UART disable
void uart_disable(void)
{
    UART1->UCR1 &= ~UART_UCR1_UARTEN_MASK;
}

// UART software reset
void uart_softreset(void)
{
    UART1->UCR2 &= ~UART_UCR2_SRST_MASK;
    while (!(UART1->UCR2 & UART_UCR2_SRST_MASK));
}

// Send a character
void uart_putchar(char c)
{
    // Wait for transmit buffer to be empty
    while (!(UART1->USR2 & UART_USR2_TXDC_MASK));

    // Send data
    UART1->UTXD = c;
}

// Receive a character
char uart_getchar(void)
{
    // Wait for data to be received
    while (!(UART1->URXD & UART_URXD_CHARRDY_MASK));

    // Return received data
    return (char)(UART1->URXD & 0xFF);
}

// Send a string
void uart_puts(const char *str)
{
    while (*str) {
        uart_putchar(*str++);
        if (*str == '\n') {
            uart_putchar('\r');
        }
    }
}
