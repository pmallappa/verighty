/*
 * Copyright (C) 2017 SilkID
 * test_capture.c: Tests the capturing image functionality
 *
 * Authors: Prem Mallappa <prem@silkid.com
 *           Aravind
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "cmd.h"
#include "libfpsensor.h"

unsigned char *imageBuffer = NULL;
unsigned char *encImageBuffer = NULL;

static void help(void)
{
	fprintf(stderr, "Usage : %s [-n <iter>]\n"
		"n - number of times to capture the image [default 10]\n"
		"", "test-capture");
}

static int test_capture(cmd_t *cmd, conf_t *conf, int argc, char *argv[])
{
	HANDLE *handle = conf->handle;
	int    ret = 0;
	char   filename[64];
	int    i, opt;
	int    ntimes = 10;
	int    count;

	while ((opt = getopt(argc, argv, "n:")) != -1) {
                switch (opt) {
                case 'n':
                        ntimes = atoi(optarg);
                        break;
                default: /* '?' */
                        help();
                        goto err_out;
                }
        }

        printf("ntimes : %d\n",ntimes);

	imageBuffer = (unsigned char *)malloc(conf->fp_buf_size);
	if (!imageBuffer) {
                perror("malloc: imageBuffer");
                ret = -1;
		goto err_out;
        }

	encImageBuffer = (unsigned char *)malloc(conf->fp_buf_size);
        if (!encImageBuffer) {
                perror("malloc: encImageBuffer");
                ret = -1;
		goto err_out;
        }
	
        count = 0;
	printf("Please place finger on the sensor\n");

	for(i = 0; i < ntimes;)
	{
		ret = sensorCapture(handle, conf->encrypt ? encImageBuffer: imageBuffer,
                                    conf->fp_buf_size);

		if(ret > 0)
		{
			printf("Captured size %d\n", ret);
			sprintf(filename, "image_%d%s", count, conf->encrypt?"_enc":"");
			//silk_write_bitmap(conf->encrypt ? encImageBuffer : imageBuffer,
                        //           conf->width, conf->height, filename);
			printf("Please place finger on the sensor\n");
			count++;
			i++;
		}

		usleep(100 * 1000);
        }

err_out:
	return ret;
}

int test_capture_register(void)
{
        cmd_ops_t test_cmd_ops = {
                .func = test_capture,
                .help = help,
        };

        cmd_t test_cmd = {
                .name = "test_capture",
                .desc = "Tests the capturing image functionality",
                .ops = test_cmd_ops,
        };

        cmd_register(&test_cmd);

        return 0;
} 
