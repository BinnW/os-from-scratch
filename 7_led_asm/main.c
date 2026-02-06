#include "main.h"

void clk_enable(void)
{
    CCM_CCGR0 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR0
    CCM_CCGR1 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR1
    CCM_CCGR2 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR2
    CCM_CCGR3 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR3
    CCM_CCGR4 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR4
    CCM_CCGR5 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR5
    CCM_CCGR6 = 0xFFFFFFFF; // Enable all clocks in CCM_CCGR6
    return;
}

void led_init(void)
{
    // Configure GPIO1_IO03 as GPIO
    SW_MUX_GPIO1_IO03 = 0x5; // Set MUX mode to GPIO
    SW_PAD_GPIO1_IO03 = 0x10B0; // Set pad configuration

    // Set GPIO1_IO03 as output
    GPIO1_GDIR = 0x8; // Set bit 3 of GDIR to 1

    // Initialize GPIO1_DR to turn off the LED
    GPIO1_DR = 0x0; // Clear bit 3 of DR to turn on the LED
    // after init, turn led on
}

void led_off(void)
{
    GPIO1_DR |= (1 << 3); // Set bit 3 of DR to turn on the LED
}

void led_on(void)
{
    GPIO1_DR &= ~(1 << 3); // Clear bit 3 of DR to turn off the LED
}


void delay_short(unsigned int count)
{
    while (count--) {
        asm volatile("nop"); // No operation, just a delay
    }
}

void delay(volatile unsigned int n)
{
    while (n--) {
        delay_short(n);
    }
}

int main(void)
{
    clk_enable(); // Enable clocks
    led_init(); // Initialize LED

    while (1) {
        led_on(); // Turn on LED
        delay(500); // Delay for a while
        led_off(); // Turn off LED
        delay(500); // Delay for a while
    }

    return 0; // This line will never be reached
}
