#include <resolv.h>
#include <unistd.h>

void
res_nclose(res_state state)
{
	close(state->fd);
}
