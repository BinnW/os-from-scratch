#ifndef __BSP_EXIT_H
#define __BSP_EXIT_H
#include "imx6ul.h"

// KEY按键中断驱动
// 中断初始化
void exit_init(void);
// 中断处理函数
void gpio1_io18_irqhandler(void);


#endif