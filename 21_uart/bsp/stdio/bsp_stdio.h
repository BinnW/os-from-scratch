#ifndef __BSP_STDIO_H
#define __BSP_STDIO_H

#include "imx6ul.h"

/* Redirect system calls to UART */
int _write(int fd, char *ptr, size_t len);
int _read(int fd, char *ptr, size_t len);

/* Minimal printf implementation */
int printf(const char *format, ...);

/* Minimal scanf implementation */
int scanf(const char *format, ...);

#endif
