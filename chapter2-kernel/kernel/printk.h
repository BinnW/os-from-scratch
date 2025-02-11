#include<stdarg.h>
#include "font.h"
#include "linkage.h"
// 定义打印字符的通用接口

// 需要先定义表示屏幕位置信息的接口
struct position
{
    int XResolution;
    int YResolution;

    int XPosition;
    int YPosition;

    int XCharSize;
    int YCharSize;

    unsigned int * FB_addr;
    unsigned long FB_length;
}Pos;

char buf[4096] = {0};

const int LEFT = 0;
const int SPACE = 0;
const int PLUS = 0;
const int SPECIAL = 0;
const int ZEROPAD = 0;
