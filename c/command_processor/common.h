/*
 * Copyright (C) 2017 SilkID
 * common.h: Common Utility functions
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>	/* for uint8_t */

#include "cmd_proc_types.h"

typedef void * HANDLE;

struct conf {
    /* Filled by command line */
    int dpi;
    int mode;
    int encrypt;
    int antifake;

    /* Filled at runtime by querying the sensor */
    int width;
    int height;
    int fp_buf_size;
    HANDLE handle;
};
typedef struct conf conf_t;

/*
 * Dump the buffer in a byte format,
 * print_len is the line length
 */
void dump_buffer(uint8_t *buf, int size, int print_len);

/* Get default params from sensor */
void get_default_params(conf_t *conf);

/* returns a random 32-bit unsigned integer from /dev/urandom */
uint32_t get_random_num(void);

#endif  /* __COMMON_H__ */
