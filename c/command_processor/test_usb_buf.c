/*
 * Copyright (C) 2017 SilkID
 *
 * test_usb_buf.c: USB bulk send test, sets buffer size and pattern to read back
 * and check
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "cmd.h"
#include "tests.h"

#include "libfpsensor.h"
#include "libfpsensor_internal.h"
#include "common.h"

static void cmp_dump_buffer(uint8_t *buf, int size, uint8_t pattern)
{
	int i;
	bool notgood = false;

	for (i = 0; i < size; i++) {
		if (buf[i] != pattern) {
			printf("index %d got:%x wanted:%x\n ", i, buf[i], pattern);
			notgood = true;
			break;
		}
	}

        if (notgood) {
		dump_buffer(buf, size, 20);
		printf("\n");
	}
}

static void help(void)
{
	fprintf(stderr, "Usage: %s [-s <size>] [-t]\n"
		"s - size of buffer in power of 2, \n"
		"\t\tfor eg -s 10 will send buffer of size 2^10 = 1024 bytes\n"
		"n - number of times to test pattern"
		""
		""
		"", "test_usb");
}

static int test_usb_buf(cmd_t *cmd, conf_t *conf, int argc, char *argv[])
{
	HANDLE *handle = conf->handle;
	uint8_t *buf;
	int buffersize = 6;
	int ret = 0;
	int opt, i;
	int ntimes = 10;
	int newbuffersize = 0;
	printf("argc: %d\n", argc);
	while ((opt = getopt(argc, argv, "s:n:")) != -1) {
		switch (opt) {
		case 's':
			buffersize = atoi(optarg);
			break;
		case 'n':
			ntimes = atoi(optarg);
			if (ntimes > 10) {
				printf("ntimes > 10, setting back to 10\n");
				ntimes = 10;
			}
			break;
		default: /* '?' */
			help();
			goto err_out;
		}
	}

	newbuffersize = 1 << buffersize;
	buf = malloc(newbuffersize);
	if (!buf) {
		perror("malloc: buf");
		ret = -1;
		goto err_out;
	}

	printf("buffersize:%d times:%d\n", buffersize, ntimes);
	//sensorSetPowerMgmt(handle, 0);	/* Disable low-power mode */
	sensorSetBufferPatternSize(handle, buffersize);

	for (i = 0; i < ntimes; i++) {
		int pattern;

		srand(get_random_num());
		pattern = rand() & 0xff;

		ret = sensorSetBufferPattern(handle, pattern);
		if(ret < 0) {
			printf("Unable to set pattern to : %x\n", pattern);
                        break;
		}

		sleep(1);

		ret = sensorGetPatternBuffer(handle, buf, newbuffersize);
		if(ret < 0) {
			printf("unable to get buffer ret:%d\n", ret);
                        break;
		} else {
			cmp_dump_buffer(buf, newbuffersize, pattern);
		}
		//prevpattern = pattern;

		sleep(1);
	}

err_out:
	free(buf);
	return ret;
}

int test_usb_register(void)
{
	cmd_ops_t test_cmd_ops = {
		.func = test_usb_buf,
		.help = help,
	};

	cmd_t test_cmd = {
		.name = "test_usb",
		.desc = "Test USB Bulk transfer with randomized pattern",
		.ops = test_cmd_ops,
	};

	cmd_register(&test_cmd);

	return 0;
}

INIT_FUNCTION(test_usb_register);

