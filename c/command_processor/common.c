/*
 * Copyright (C) 2017 SilkID
 *
 * common.c: common functions needed by other modules
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>			/* for uint_8 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>			/* above 3 for open() */
#include <unistd.h>			/* for read() */
#include <string.h>			/* for memset() */

#include "common.h"
#include "libfpsensor.h"
#include "libfpsensordef.h"

/*
 * Common functions that need not be bothered with main
 */
void dump_buffer(uint8_t *buf, int size, int print_len)
{
    int i;

    if (!print_len || print_len > 32)
        print_len = 10;

    for (i = 0; i < size; i++) {
        printf("%x  ", buf[i]);
        if (i % print_len == 0)
            printf("\n");
    }
}

/*
 * Opened once and fd, is Valid for the lifetime of program.
 */
unsigned int get_random_num(void)
{
    static int fd = -1;
    unsigned int val = 0;
    if (fd < 0)
        fd = open("/dev/urandom", O_RDONLY);

    read(fd, &val, sizeof(val));
    return val;
}

void get_default_params(conf_t *conf)
{
	silk_image_params_t img_params = {0,};
#if 0
	char paramValue[64] = {0,};
	HANDLE *handle = conf->handle;
	int ret;

	memset(&img_params, 0, sizeof(img_params));
	sensorGetImageParams(handle, &img_params);
	conf->width = img_params.width;
	conf->height = img_params.height;
	memset(paramValue, 0, 64);
	ret = sizeof(paramValue);
	sensorGetParameterEx(handle, SENSOR_PARAM_CODE_IMAGE_BUFFER_SIZE, paramValue, &ret);
	conf->fp_buf_size = *((int *)paramValue);

	printf("width=%d, heigth=%d\n", conf->width, conf->height);

	// Manufacturer
	memset(paramValue, 0, 64);
	ret = sizeof(paramValue);
	sensorGetParameterEx(handle, SENSOR_PARAM_CODE_MANUFACTURER, paramValue, &ret);
	printf("Manufacturer=%s\n", paramValue);

	// Product
	memset(paramValue, 0, 64);
	ret = sizeof(paramValue);
	sensorGetParameterEx(handle, SENSOR_PARAM_CODE_PRODUCT_NAME, paramValue, &ret);
	printf("Product name=%s\n", paramValue);

	// SerialNumber
	memset(paramValue, 0, 64);
	ret = sizeof(paramValue);
	sensorGetParameterEx(handle, SENSOR_PARAM_CODE_SERIAL_NUMBER, paramValue, &ret);
	printf("Serial number=%s\n", paramValue);
#endif
	// Encryption
	printf("Image Encryption: ");
	switch (img_params.enc_type) {
		case SILK_ENC_TYPE_ECB:
			printf("ECB");
			break;
		case SILK_ENC_TYPE_CBC:
			printf("CBC");
			break;
		default:
		case SILK_ENC_TYPE_NONE:
			printf("NONE");
			break;
	}
	printf("\n");

}
