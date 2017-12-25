/*
 * Copyright (C) 2017 SilkID
 *
 * test_thread.c: Wakes a app_slk thread which was waiting on a
 * semaphore/variable.
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

static void help(void)
{
	fprintf(stderr, "Usage: %s [-n <iter>]\n"
		"n - number of times to test pattern [default 10]"
		""
		""
		"", "test_thread");
}

static int test_thread(cmd_t *cmd, conf_t *conf, int argc, char *argv[])
{
	int ntimes = 10;
	int opt, i;
	HANDLE *handle = conf->handle;

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

	for (i = 0; i < ntimes; i++) {
		sensorAppSlkSetMode(handle, 1); // 1 printing
		sleep(1);
		sensorAppSlkSetMode(handle, 0); // 0 idle
	}


	return 0;

err_out:
	return -1;
}

int test_thread_register(void)
{
	cmd_ops_t test_cmd_ops = {
		.func = test_thread,
		.help = help,
	};

	cmd_t test_cmd = {
		.name = "test_thread",
		.desc = "Run the app_slk thread, by releasing a semaphore that app_slk is waiting on",
		.ops = test_cmd_ops,
	};

	cmd_register(&test_cmd);

	return 0;
}

INIT_FUNCTION(test_thread_register);

