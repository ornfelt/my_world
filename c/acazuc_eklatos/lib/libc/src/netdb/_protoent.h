#ifndef _PROTOENT_H
#define _PROTOENT_H

#include <stdio.h>
#include <netdb.h>

extern FILE *protoent_fp;
extern int protoent_stayopen;

struct protoent *next_protoent(void);

#endif
