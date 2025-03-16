#include "lib.h"
#include "printk.h"
#include "gate.h"

void Start_Kernel()
{
    int* addr = (int*)0xffff800000a00000;
    int i;

    Pos.XResolution = 1440;
    Pos.YResolution = 900;

    Pos.XPosition = 0;
    Pos.YPosition = 0;

    Pos.XCharSize = 8;
    Pos.YCharSize = 16;

    Pos.FB_addr = (int *)0xffff800000a00000;
    Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4);

    // 打印红色条
    for (i = 0; i < 1440 * 20; i++)
    {
        *((char*)addr + 0) = 0x00;
        *((char*)addr + 1) = 0x00;
        *((char*)addr + 2) = 0xff;
        *((char*)addr + 3) = 0x00;
        addr += 1;
    }

    // 打印绿色条
    for (i = 0; i < 1440 * 20; i++)
    {
        *((char*)addr + 0) = 0x00;
        *((char*)addr + 1) = 0xff;
        *((char*)addr + 2) = 0x00;
        *((char*)addr + 3) = 0x00;
        addr += 1;
    }

    // 打印蓝色条
    for (i = 0; i < 1440 * 20; i++)
    {
        *((char*)addr + 0) = 0xff;
        *((char*)addr + 1) = 0x00;
        *((char*)addr + 2) = 0x00;
        *((char*)addr + 3) = 0x00;
        addr += 1;
    }

    // 打印白色条
    for (i = 0; i < 1440 * 20; i++)
    {
        *((char*)addr + 0) = 0xff;
        *((char*)addr + 1) = 0xff;
        *((char*)addr + 2) = 0xff;
        *((char*)addr + 3) = 0x00;
        addr += 1;
    }

    color_printk(YELLOW, BLACK, "Hello\t\tWorld!\n\n");

    int a = 1/0;

    while(1);
}