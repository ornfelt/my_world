#ifndef JKSSL_CMD_COMMON_H
#define JKSSL_CMD_COMMON_H

#include <stdio.h>

enum cmd_format
{
	CMD_FORMAT_PEM,
	CMD_FORMAT_DER,
};

int cmd_handle_format(const char *progname, const char *arg,
                      enum cmd_format *format);
int cmd_handle_in(const char *progname, const char *arg, FILE **fp);
int cmd_handle_out(const char *progname, const char *arg, FILE **fp);
int cmd_handle_pass(const char *progname, const char *arg, char **pass);
char *cmd_ask_password(void);
char *cmd_ask_password_confirm(void);
int cmd_ask_pass(char *buf, int len, int rw, void *userdata);

#endif
