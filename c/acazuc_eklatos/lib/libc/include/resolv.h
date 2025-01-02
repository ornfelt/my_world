#ifndef RESOLV_H
#define RESOLV_H

#include <sys/socket.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _PATH_RESCONF "/etc/resolv.conf"

struct __res_state
{
	int fd;
};

typedef struct __res_state *res_state;

int res_ninit(res_state state);
void res_nclose(res_state state);

int res_nsend(res_state state, const uint8_t *msg, int len, uint8_t *answer,
              int answer_len);
int res_nquery(res_state state, const char *name, int class, int type,
               uint8_t *answer, int answer_len);
int res_nmkquery(res_state state, int op, const char *name, int class, int type,
                 const uint8_t *data, int len, const uint8_t *newrr,
                 uint8_t *buf, int buflen);

int dn_comp(const char *dst, uint8_t *dn, int len, uint8_t **dnptrs,
            uint8_t **lastdnptr);
int dn_expand(const uint8_t *msg, const uint8_t *eom, const uint8_t *dn,
              char *str, int str_len);

#ifdef __cplusplus
}
#endif

#endif
