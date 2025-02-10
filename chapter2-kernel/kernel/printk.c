#include<stdarg.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"


void putchar(unsigned int * fb, int XSize, int x, int y, unsigned int FRColor, unsigned int BKColor, unsigned char font)
{
    // 
    int i = 0, j = 0;
    unsigned int * addr = NULL;
    unsigned char * fontp = NULL;
    int testval = 0;
    fontp = font_ascii[font];

    for (i = 0; i < 16; i++) {
        addr = fb + XSize * (y + i) + x;
        testval = 0x100;
        for (j = 0; j < 8; j++) {
            testval = testval >> 1;
            if (*fontp * testval) {
                *addr = FRColor;
            } else {
                *addr = BKColor;
            }
            addr++;
        }
        fontp++;
    }
}


int color_printk(unsigned int FRColor, unsigned int BKColor, const char * fmt, ...)
{
    int i = 0;
    int count = 0;
    int line = 0;
    va_list args;
    va_start(args, fmt);
    // 解析color_printk提供的格式化字符及其参数，一会儿vsprintf自己实现
    i = vsprintf(buf, fmt, args);
    va_end(args);

    for (count = 0; count < i || line; count++) {
        if (line > 0) {
            count--;
            goto Label_tab;
        }
        
        if ((unsigned char)*(buf+count) == '\n') {
            // 如果发现某个待检测字符是转义字符\n，则将光标行数+1，列位置设为0
            Pos.YPosition++;
            Pos.XPosition = 0;
        } else if ((unsigned char)*(buf + count) == '\b') {
            Pos.XPosition--;
            if (Pos.XPosition < 0) {
                // 如果X小于零了，退到上一行，从最末尾开始计算
                Pos.XPosition = (Pos.XResolution / Pos.XCharSize - 1) * Pos.XCharSize;
                Pos.YPosition--;
                if (Pos.YPosition < 0) {
                    Pos.YPosition = (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize;
                }
            }
            // putchar 还没实现呢，putchar才是实现打印的函数
            putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRColor, BKColor, ' ');
        } else if ((unsigned char) * (buf + count) == '\t') {
            // 计算当前光标距下一个制表符需要填充的空格符数，将计算结果保存至局部变量line中
            // 8表示一个制表位占用8个显示字符
            line = ((Pos.XPosition + 8) & ~(8-1)) - Pos.XPosition;
        Label_tab:
            line--;
            putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRColor, BKColor, ' ');
            Pos.XPosition++;
        } else {
            putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRColor, BKColor, (unsigned char)*(buf+count));
            Pos.XPosition++;
        }

        // 调整下一个字符的位置，需要变更XPosition以及YPosition
        if (Pos.XPosition >= Pos.XResolution / Pos.XCharSize) {
            Pos.YPosition++;
            Pos.XPosition = 0;
        }
        // 如果到最后一行了，就自动覆盖最上面的
        if (Pos.YPosition >= Pos.YResolution / Pos.YCharSize) {
            Pos.YPosition = 0;
        }
    }
}
