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


int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *str, *s;
    int flags;
    int field_width;
    int precision;
    int len, i;
    int qualifier;

    for (str = buf; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }
        flags = 0;
        repeat:
            fmt++;
            switch (*fmt)
            {
                case '-':
                    flags |= LEFT;
                    goto repeat;
                case '+':
                    flags |= PLUS;
                    goto repeat;
                case ' ':
                    flags |= SPACE;
                    goto repeat;
                case '#':
                    flags |= SPECIAL;
                    goto repeat;
                case '0':
                    flags |= ZEROPAD;
                    goto repeat;
                default:
                    break;
            }
        /* get field width */
        field_width = -1;
        // 提取%后续的数字，表示数据区域的宽度
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            // 如果不是数字而是*，则宽度由可变参数提供
            fmt++;
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
        // 获取数据区域宽度后，提取出显示数据的精度
        // 宽度后跟有.，其后的数字表明了显示数据的精度
        /* get precision */
        precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (is_digit(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if (*fmt == '*') {
                fmt++;
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }
        // 随后显示数据的规格
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z') {
            qualifier = *fmt;
            fmt++;
        }
        // 如果匹配出字符c，程序将可变参数转换为一个字符
        switch (*fmt) {
            case 'c':
                if (!(flags & LEFT)) {
                    while (--field_width > 0) {
                        *str++ = ' ';
                    }
                }
                *str++ = (unsigned char)va_arg(args, int);
                while (--field_width > 0)
                    *str++ = ' ';
                break;
            case 's':
                s = va_arg(args, char*);
                if (!s)
                    s = '\0';
                len = strlen(s);
                if (precision < 0) {
                    precision = len;
                } else if (len > precision){
                    len = precision;
                }
                if (!(flags & LEFT))
                    while (len < field_width--)
                        *str++ = ' ';
                for (i = 0; i < len; i++)
                    *str++ = *s++;
                while (len < field_width--)
                    *str++ = ' ';
                break;
            case 'o':
                if (qualifier == '1')
                    str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
                else
                    str = number(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
                break;
            case 'p':
                if (field_width == -1) {
                    field_width = 2 * sizeof(void *);
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
                break;
            case 'x':
                flags |= SMALL;
            case 'X':
                if (qualifier == '1') {
                    str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
                } else {
                    str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
                }
                break;
            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                if (qualifier == '1')
                    str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
                else
                    str = number(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
                break;
            case 'n':
                if (qualifier == '1') {
                    long *ip = va_arg(args, long *);
                    *ip = (str - buf);
                } else {
                    int *ip = va_arg(args, int *);
                    *ip = (str - buf);
                }
                break;
            case '%':
                *str++ = '%';
                break;
            default:
                *str++ = '%';
                if (*fmt)
                    *str++ = *fmt;
                else
                    fmt--;
                break;
        }
    }
    return 0;
}

int skip_atoi(const char **s)
{
    // 只能将数值字母转换为整数值，竟然这样简单
    // need to mark
    int i = 0;
    while (is_digit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
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
