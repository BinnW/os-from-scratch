#ifndef __BSP_KEYFILTER__
#define __BSP_KEYFILTER__

/* function defination */

void filterkey_init(void);
// 需要定时器初始化
void filtertimer_init(unsigned int value);
void filtertimer_stop(void);
void filtertimer_restart(unsigned int value);
void filtertimer_irqhandler(void);
void gpio1_16_31_irqhandler(void);

#endif