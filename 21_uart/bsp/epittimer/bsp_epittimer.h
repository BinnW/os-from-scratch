#ifndef __BSP_EPITTIMER__
#define __BSP_EPITTIMER__

#include "imx6ul.h"

// 初始化的这俩参数是干啥的？
void epit1_init(unsigned int frac, unsigned int value);
void epit1_irqhandler(void);

#endif