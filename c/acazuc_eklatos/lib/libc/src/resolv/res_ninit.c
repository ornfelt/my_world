#include <arpa/inet.h>

#include <sys/socket.h>

#include <resolv.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

static int
load_nameserver(struct in_addr *dst)
{
	FILE *fp = NULL;
	int ret = 1;
	char buf[128];

	fp = fopen(_PATH_RESCONF, "r");
	if (!fp)
		goto end;
	if (!fgets(buf, sizeof(buf), fp))
		goto end;
	if (strncmp(buf, "nameserver ", 11))
		goto end;
	if (buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = '\0';
	if (inet_pton(AF_INET, &buf[11], dst) != 1)
		goto end;
	ret = 0;

end:
	if (fp)
		fclose(fp);
	return ret;
}

int
res_ninit(res_state state)
{
	struct sockaddr_in dst;
	struct timeval tv;

	dst.sin_family = AF_INET;
	dst.sin_port = htons(53);
	if (load_nameserver(&dst.sin_addr))
		return -1;
	memset(state, 0, sizeof(*state));
	state->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (state->fd == -1)
		return -1;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (connect(state->fd, (struct sockaddr*)&dst, sizeof(dst)) == -1
	 || setsockopt(state->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1
	 || setsockopt(state->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1)
		goto err;
	return 0;

err:
	close(state->fd);
	state->fd = -1;
	return -1;
}
