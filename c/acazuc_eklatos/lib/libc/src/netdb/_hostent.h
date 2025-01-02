#ifndef _HOSTENT_H
#define _HOSTENT_H

#include <stdio.h>
#include <netdb.h>

extern FILE *hostent_fp;
extern int hostent_stayopen;

struct hostent *next_hostent(void);

#endif
