/*
 * Copyright (C) 2017 SilkID
 * main.c: Driver for all tests.
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "cmd.h"
#include "tests.h"

#include "libfpsensor.h"		/* for sensorClose() in signal handling */

extern unsigned char *imageBuffer;

conf_t conf = (conf_t) {
	.dpi = 500,
	.mode = 0,
	.encrypt = 0,
	.antifake = 1,
};

static void sighandler(int signo)
{
	if(imageBuffer)
	{
		printf("Freeing imageBuffer\n");
		free(imageBuffer);
	}
	if(conf.handle)	{
		sensorClose(conf.handle);
		sensorFree();
	}
	printf("sighandler\n");
	exit(signo);
}

static void setup_exit_signal_handler(void)
{
	int index;
	struct sigaction sa;
	int sigs[] = {
		SIGILL, SIGFPE, SIGABRT, SIGBUS,
		SIGSEGV, SIGHUP, SIGINT, SIGQUIT,
		SIGTERM
	};

	sa.sa_handler = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND;
	for(index = 0; index < sizeof(sigs)/sizeof(sigs[0]); ++index) {
		if (sigaction(sigs[index], &sa, NULL) == -1) {
			perror("Could not set signal handler");
		}
	}
}

char ** fixup_argv_batchmode(char *cmd, int argc, char *argv[])
{
	char **cmdargs;
	int i;

	/* we need extra room for argv[0] to hold the command line and last entry of
	 * argv[argc] to be a NULL pointer. Hence +3
	 */
	cmdargs = calloc(sizeof(char*), (argc + 3));
	if (!cmdargs) {
		perror("malloc cmdargs\n");
		exit(0);
	}

	cmdargs[0] = cmd;

	for (i = 1; i < argc; i++) {
		printf("copying %s\n", argv[i]);
		cmdargs[i] = argv[i];
	}

	cmdargs[argc] = NULL;

	return cmdargs;
}

enum {
	ECB = 0x11,
	CBC,
};

int main(int argc, char *argv[])
{
        int ret = 0;
	int opt, mode;
	char *cmd, **cmdargv;
	int cmdargc;
	bool batchmode = false;
	silk_image_params_t img_params;

	while ((opt = getopt(argc, argv, "ae:m:d:c:")) != -1) {
		switch (opt) {
		case 'a':
			conf.antifake = 1;
			break;
		case 'm':
			mode = atoi(optarg);
			if (mode != 0 && mode != 1) {
				printf("Unknown mode %d\n", conf.mode);
				exit(0);
			}
			conf.mode = mode;
			break;
		case 'd':
			conf.dpi = atoi(optarg);
			switch (conf.dpi) {
			case 750: case 500: case 1000:
				break;
			default:
				printf("Invalid dpi %d\n", conf.dpi);
			break;
			}
			break;
		case 'e':
			conf.encrypt = ECB; 
			if (optarg[0] == 'c')
				conf.encrypt = CBC;

			break;
                case 'c':
			cmd = optarg;
			batchmode = true;
			cmdargc = argc - optind;
			cmdargv = fixup_argv_batchmode(cmd, cmdargc, &argv[optind]);
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-a] [-d dpi] [-m mode]\n"
					"a - enable antifake (default off)\n"
					"d dpi - set dpi to 'dpi' 500/750/1000\n"
					"m mode - set mode to 'mode' \n"
					" \t\t 0-detect (default)\n"
					" \t\t 1-stream (default)\n"
					"-c cmd - batchmode; executes 'cmd and exits\n"
					" \targs to cmd can be provided after a -- \n",
					argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	printf("optarg:%s optind:%d argc:%d\n", optarg, optind, argc);

	setup_exit_signal_handler();

	ret = sensorInit();
	if(0 != ret)
	{
		printf("sensorInit failed, ret=%d\n", ret);
		goto err_out;
	}

	conf.handle = sensorOpen(0);	/* Open the default device */

	if (!conf.handle) {
		printf("Unable to get handle for device\n");
		goto err_out;
	}

	img_params.dpi_value = (silk_dpi_values_t)conf.dpi;
	img_params.enable_antifake = (bool)conf.antifake;
	img_params.capture_mode = (silk_capture_mode_t)conf.mode;

	switch (conf.encrypt) {
		default:
			img_params.enc_type = SILK_ENC_TYPE_NONE;
			break;
		case ECB:
			img_params.enc_type = SILK_ENC_TYPE_ECB;
			break;
		case CBC:
			img_params.enc_type = SILK_ENC_TYPE_CBC;
			break;
	}

	sensorSetImageParams(conf.handle, &img_params);

	get_default_params(&conf);

	init_tests();

	if (batchmode) {
		ret = cmd_run_batch(&conf, cmd, argc - optind + 1, cmdargv);
		goto out;
	}

        while(1) {

            int ret = 0;

            printf("for help type help\n");

	    /*
	     * argc now should be +1 as we are adding the command name
	     */
            ret = cmd_start(&conf);

            if (ret < 0 || batchmode)
                break;
        }
out:
        printf("main finished\n");
	return EXIT_SUCCESS;

err_out:
	return EXIT_FAILURE;
}

