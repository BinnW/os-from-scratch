#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "imx6ul.h"

enum key_value {
    KEY_NONE = 0U,
    KEY_VALUE
};

void key_init();
int key_get_value();

#endif