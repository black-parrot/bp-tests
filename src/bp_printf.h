#ifndef __BP_PRINTF_H__
#define __BP_PRINTF_H__

#include <stdio.h>
#include "bp_utils.h"

#define BP_PRINTF_MAXLEN 1024

char bp_printf_buf[BP_PRINTF_MAXLEN];

#define bp_printf(fmt, ...) \
    snprintf(bp_printf_buf, BP_PRINTF_MAXLEN, fmt, __VA_ARGS__); \
    bp_print_string(bp_printf_buf);

#endif // __BP_PRINTF_H__
