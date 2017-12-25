/*
 * Copyright (C) 2017 SilkID
 * cmd.h:  Command Processor interface
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */

#ifndef __CMD_H__
#define __CMD_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>                              /* all 3 are for optind,optarg */

/* Max supported options including opt:value pair */
#define MAX_ARGV        256

/* Total commands in this system */
#define MAX_CMDS        256

typedef struct cmd_ops cmd_ops_t;
typedef struct cmd cmd_t;

/*
 * func() and help() are mandatory
 */
struct cmd_ops {
    /* Actual command call back */
    int (*func)(cmd_t *cmd, conf_t *conf, int argc, char *argv[]);
    /* Help function */
    void (*help)(void);
};

struct cmd {
    char     name[64];                          /* Command name */
    char     desc[256];                         /*  */
    int      name_len;                          /*  */
    int      argc;                              /* number of args in argv */
    char    *argv;          /* list of arguments for this command */
    struct cmd_ops  ops;    /* list of supported operations func() and help() */
};

/* Register API */
int cmd_register(cmd_t *cmd);

/* Start the command interpreter */
int cmd_start(conf_t *conf);

/* Batch mode only, just runs one command and exits */
int cmd_run_batch(conf_t *conf, const char *cmd, int argc, char *argv[]);

/* runs a given command */
int cmd_run(const char *cmd, conf_t *conf, int argc, char *argv[]);

#endif  /* __CMD_H__ */
