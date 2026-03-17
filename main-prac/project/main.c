// UART 实验 - printf/scanf 重定向到串口测试
#include "bsp_led.h"
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_uart.h"
#include "bsp_stdio.h"

int main(void)
{
    // 初始化各外设
    imx6u_clkinit();      // 初始化 CPU 时钟
    clk_init();           // 初始化外设时钟
    led_init();           // 初始化 LED
    
    // 初始化 UART1
    uart_init();
    
    // 测试 printf - 通过 _write 重定向到 UART
    printf("\r\n=== UART Printf/Scanf Test ===\r\n");
    printf("System initialized!\r\n");
    printf("Baudrate: %d\r\n", 115200);
    printf("This message is printed via printf()\r\n");
    
    // 测试带格式的输出
    int num = 42;
    int neg_num = -123;
    printf("Integer: %d, Negative: %d\r\n", num, neg_num);
    printf("Hex: %x, HEX: %X\r\n", 0xDEAD, 0xBEEF);
    
    // 测试 scanf - 通过 _read 重定向到 UART
    printf("\r\n--- Input Test ---\r\n");
    printf("Please enter an integer: ");
    
    int input_num;
    if (scanf("%d", &input_num) == 1) {
        printf("\r\nYou entered: %d\r\n", input_num);
        printf("Double of that is: %d\r\n", input_num * 2);
    } else {
        printf("\r\nInvalid input!\r\n");
    }
    
    // 测试字符输入
    printf("\r\nPlease enter a character: ");
    char ch;
    if (scanf("%c", &ch) == 1) {
        printf("\r\nYou entered: '%c' (ASCII: %d)\r\n", ch, (int)ch);
    }
    
    // 测试字符串输入
    printf("\r\nPlease enter a string (no spaces): ");
    char str[32];
    if (scanf("%s", str) == 1) {
        printf("\r\nYou entered: \"%s\"\r\n", str);
    }
    
    printf("\r\n=== Test Complete ===\r\n");
    
    // 闪烁 LED 指示测试完成
    while (1) {
        led_on();
        delay(500);
        led_off();
        delay(500);
    }

    return 0;
}
