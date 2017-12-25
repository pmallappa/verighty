/*
 * Copyright (C) 2017 SilkID
 * test_led.c: Tests the LED's on the SilkID Device
 *
 * Authors: Prem Mallappa <prem@silkid.com
 *           Aravind
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "tests.h"
#include "common.h"
#include "cmd.h"

#include "libfpsensor.h"
#include "libfpsensordef.h"

static void help(void)
{
	fprintf(stderr, "Usage: %s [-n <iter>] [-l led_color]\n"
		"n - number of times to test pattern [default 10]\n"
                "l - test led\n"
                "led_color - w/g/r/f/b\n"
                "w - white led on\n"
                "g - green led on\n"
                "r - red led on\n"
                "f - flash white led alternatively\n"
                "b - white led breathing\n\",\n"
		"", "test_led");
}

static int test_led(cmd_t *cmd, conf_t *conf, int argc, char *argv[])
{
	int ntimes = 10;
	int opt, i, led;
	HANDLE *handle = conf->handle;

	while ((opt = getopt(argc, argv, "n:l:")) != -1) {
		switch (opt) {
		case 'n':
			ntimes = atoi(optarg);
			break;
                case 'l':
			led = optarg[0];
			break;
		default: /* '?' */
			help();
			goto err_out;
		}
	}

	for (i = 0; i < ntimes; i++) {
		switch(led) {
		case 'w':
			sensorLedOn(handle, WHITE_LED, 2000);
			break;
		case 'g':
			sensorLedOn(handle, GREEN_LED, 2000);
			break;
		case 'r':
			sensorLedOn(handle, RED_LED, 2000);
			break;
		case 'f':
			sensorLedOn(handle, WHITE_BLINK, 2000);
			break;
		case 'b':
			sensorLedOn(handle, WHITE_BREATH, 2000);
			break;
		default:
			help();
                        goto err_out;
		}

		sleep(3);
	}

	return 0;

err_out:
	return -1;
}

int test_led_register(void)
{
	cmd_ops_t test_cmd_ops = {
		.func = test_led,
		.help = help,
	};

	cmd_t test_cmd = {
		.name = "test_led",
		.desc = "Blink the Red/Green/White and White-breath LEDs",
		.ops = test_cmd_ops,
	};

	cmd_register(&test_cmd);

	return 0;
}

INIT_FUNCTION(test_led_register);
