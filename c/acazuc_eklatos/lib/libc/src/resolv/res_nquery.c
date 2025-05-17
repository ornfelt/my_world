#include <arpa/nameser.h>

#include <resolv.h>

int
res_nquery(res_state state,
           const char *name,
           int class,
           int type,
           uint8_t *answer,
           int answer_len)
{
	uint8_t msg[512];
	int ret;

	ret = res_nmkquery(state,
	                   QUERY,
	                   name,
	                   class,
	                   type,
	                   NULL,
	                   0,
	                   NULL,
	                   msg,
	                   sizeof(msg));
	if (ret == -1)
		return -1;
	/* XXX check returned id */
	return res_nsend(state, msg, ret, answer, answer_len);
}
