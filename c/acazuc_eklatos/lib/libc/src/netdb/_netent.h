#ifndef _NETENT_H
#define _NETENT_H

#include <netdb.h>
#include <stdio.h>

extern FILE *netent_fp;
extern int netent_stayopen;

struct netent *next_netent(void);

#endif
