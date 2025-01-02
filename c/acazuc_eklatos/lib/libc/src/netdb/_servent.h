#ifndef _SERVENT_H
#define _SERVENT_H

#include <stdio.h>
#include <netdb.h>

extern FILE *servent_fp;
extern int servent_stayopen;

struct servent *next_servent(void);

#endif
