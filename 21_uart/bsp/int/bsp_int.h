#ifndef __BSP_INT_H
#define __BSP_INT_H

#include "imx6ul.h"

typedef void (*system_irq_handler_t) (unsigned int giccIar, void *param);

typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler;
    void *userParam;
} sys_irq_handle_t;

// 开始功能函数定义
void int_init(void);
void system_irqtable_init(void);
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam);
void system_irqhandler(unsigned int giccIar);
void default_irqhandler(unsigned int giccIar, void *userParam);

#endif