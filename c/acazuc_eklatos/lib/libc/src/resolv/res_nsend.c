#include <resolv.h>

int res_nsend(res_state state, const uint8_t *msg, int len,
              uint8_t *answer, int answer_len)
{
	if (state->fd < 0)
		return -1;
	ssize_t ret = send(state->fd, msg, len, 0);
	if (ret < 0)
		return -1;
	ret = recv(state->fd, answer, answer_len, 0);
	if (ret < 0)
		return -1;
	return ret;
}
