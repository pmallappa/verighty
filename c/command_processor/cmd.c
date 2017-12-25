/*
 * Copyright (C) 2017 SilkID
 * cmd.c: command processor implementation
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>			/* for memcpy() */
#include <wordexp.h>

#include "common.h"
#include "cmd.h"

static cmd_t cmd_table[MAX_CMDS];
static int cmd_index = 0;

#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

int cmd_register(cmd_t *cmd)
{
	cmd_t *new_idx = &cmd_table[cmd_index];
	int len;

	if (cmd_index >= ARRAY_SIZE(cmd_table)) {
		printf("No more space for new commands index:%d \n", cmd_index);
		goto err_out;
	}

	/* Sanity check */
	len = strlen(cmd->name);
	if (len < 3) { /* min len for a command is 3 chars */
		printf("Command name should be at least 3 characters\n");
		goto err_out;
	}

	if (!cmd->ops.func  || !cmd->ops.help) {
		printf("func() and help() are mandatory\n");
		goto err_out;
	}

	strncpy(new_idx->name, cmd->name, sizeof(cmd->name));
	strncpy(new_idx->desc, cmd->desc, sizeof(cmd->desc));
	memcpy(&new_idx->ops, &cmd->ops, sizeof(cmd->ops));

	new_idx->name_len = len;

	cmd_index++;

	return 0;

err_out:
	return -1;
}

cmd_t *
cmd_find(const char *cmd_name, conf_t *conf,
         cmd_t *cmd_table, int tbl_size)
{
    int i;
    for (i = 0; i < tbl_size; i++) {
        cmd_t *cmd = &cmd_table[i];

        if (i > cmd_index)	/* Dont over run */
            break;

        if (!cmd->name_len)
            break;

        if (strncmp(cmd->name, cmd_name, cmd->name_len) == 0) {
            /* String matched */
            printf("command found :%s\n", cmd_name);

            return cmd;
        }
    }

    return NULL;
}

/* Crude implementation, hashing would be a better option */
int cmd_run(const char *cmd_name, conf_t *conf, int argc, char *argv[])
{
	int ret = -1;


        cmd_t *cmd = cmd_find(cmd_name, conf, &cmd_table[0],
                              ARRAY_SIZE(cmd_table));
        if (cmd) {
            ret = cmd->ops.func(cmd, conf, argc, argv);
        } else {
            printf("command %s not found.\n", cmd_name);
        }

	return ret;
}

/* Should print all commands, with description */
static void cmd_help(void)
{
	int i;
	printf("%s:%s\n", "help", "Print this Help command");

	for (i = 0; i < cmd_index; i++) {
		cmd_t *cmd = &cmd_table[i];
		if (cmd->name_len)
			printf("%s: %s\n", cmd->name, cmd->desc);
	}
	printf("\n");
}

static int run_one_cmd(conf_t *conf, char *line)
{
	char **argv;
	char *tmp = NULL;
	int argc;
	int ret = 0;
	wordexp_t p = {0,};

	// check if it is help
	if (strncmp(line, "help", 4) == 0) {
		cmd_help();
		goto out;
	}
	ret = wordexp(line, &p, WRDE_NOCMD |  WRDE_SHOWERR);
	if (ret < 0) {
		printf("something happened while processing line:%s\n", line);
		perror("wordexp:");
		goto out;
	}

	argv = p.we_wordv;
	argc = p.we_wordc;

	if (argv == NULL) {		/* adjustment */
		tmp = line;
		argv = &tmp;
		argc = 1;
	}
	printf("calling %s with argc:%d\n", argv[0], argc);
	// Reset the getopt scanning
	optind = 1;

	ret = cmd_run(argv[0], conf, argc, argv);
	if (ret < 0) {
		printf("cmd:%s exited with :%d\n", argv[0], ret);
	}

	wordfree(&p);
out:
	return ret;
}

int cmd_run_batch(conf_t *conf, const char *cmd, int argc, char *argv[])
{
	int ret;
	if (!conf)
		return -1;

	printf("cmd:%s argc:%d argv[1]:%s\n", cmd, argc, argv[1]);

	// Reset the getopt scanning
	optind = 1;

	ret = cmd_run(cmd, conf, argc, argv);

	return ret;
}

int cmd_start(conf_t *conf)
{
	int ret;
	size_t len = 0;


	while(1) {
		int nread;
		char *line;
		printf("=> ");
		fflush(stdout);
		fflush(stdin);
		nread = getline(&line, &len, stdin);
		if (nread != -1) {
			printf("read length %u:\n", nread);
			// Strip off the \n
			line[nread - 1] = '\0';
			if (nread >= 3) /* min len for a command is 3 chars */
				ret = run_one_cmd(conf, line);
		} else {
			/* EOF */
			ret = -1;
			goto out;
		}

		//free(line);
	}
out:
	return ret;
}

