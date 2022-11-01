#ifndef  __DEV_CONSOLE_H__
#define  __DEV_CONSOLE_H__


#include "drv_config.h"

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */


#define DEV_CONSOLEBUF_SIZE 128
#define DEV_CONSOLE_DEVICE_NAME "uart1"


#define _ISDIGIT(c)  ((unsigned)((c) - '0') < 10)

rt_device_t dev_console_set_device(void);

int rt_kprintf(const char *fmt, ...);

#endif

