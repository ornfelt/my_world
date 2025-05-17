#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>

#define OPT_l (1 << 0)

struct env
{
	const char *progname;
	struct sockaddr_in src;
	struct sockaddr_in dst;
	const char *src_host;
	const char *src_port;
	const char *dst_host;
	const char *dst_port;
	int sock;
	int opt;
};

static int
resolve_addr(struct env *env,
             struct sockaddr_in *sin,
             const char *host,
             const char *port)
{
	struct addrinfo *addrs;
	int ret;

	if (!host && !port)
	{
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = INADDR_ANY;
		sin->sin_port = 0;
		return 0;
	}
	ret = getaddrinfo(host, port, NULL, &addrs);
	if (ret)
	{
		fprintf(stderr, "%s: getaddrinfo: %s\n",
		        env->progname,
		        gai_strerror(ret));
		return 1;
	}
	if (!addrs)
	{
		fprintf(stderr, "%s: no address found\n", env->progname);
		return 1;
	}
	ret = 1;
	if (addrs->ai_family != AF_INET)
	{
		fprintf(stderr, "%s: no AF_INET found\n", env->progname);
		goto end;
	}
	*sin = *(struct sockaddr_in*)addrs->ai_addr;
	ret = 0;

end:
	freeaddrinfo(addrs);
	return ret;
}

static int
open_sock(struct env *env)
{
	env->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (env->sock == -1)
	{
		fprintf(stderr, "%s: socket: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
handle_stdin(struct env *env, int fd)
{
	char buf[4096];
	ssize_t rd;

	rd = read(0, buf, sizeof(buf));
	if (!rd)
		return 1;
	if (send(fd, buf, rd, 0) == -1)
	{
		fprintf(stderr, "%s: send: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
handle_sock(struct env *env, int fd)
{
	char buf[4096];
	ssize_t rd;

	rd = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
	if (rd == -1)
	{
		if (errno == EAGAIN)
			return 0;
		fprintf(stderr, "%s: recv: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (!rd)
		return 1;
	write(1, buf, rd);
	return 0;
}

static int
handle_conn(struct env *env, int fd)
{
	while (1)
	{
		struct pollfd fds[2];
		int ret;

		fds[0].fd = 0;
		fds[0].events = POLLIN;
		fds[1].fd = fd;
		fds[1].events = POLLIN;
		ret = poll(fds, sizeof(fds) / sizeof(*fds), -1);
		if (ret == -1)
		{
			fprintf(stderr, "%s: poll: %s\n",
			        env->progname,
			        strerror(errno));
			return 1;
		}
		if (fds[0].revents & POLLIN)
		{
			if (handle_stdin(env, fd))
				return 1;
		}
		if (fds[1].revents & POLLIN)
		{
			if (handle_sock(env, fd))
				return 1;
		}
		else if (fds[1].revents & POLLHUP)
		{
			if (handle_sock(env, fd))
				return 1;
		}
	}
	return 0;
}

static int
run_listen(struct env *env)
{
	if (resolve_addr(env, &env->src, env->src_host, env->src_port))
		return 1;
	if (open_sock(env))
		return 1;
	if (bind(env->sock,
	         (struct sockaddr*)&env->src,
	         sizeof(env->src)) == -1)
	{
		fprintf(stderr, "%s: bind: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (listen(env->sock, 256) == -1)
	{
		fprintf(stderr, "%s: listen: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	while (1)
	{
		struct sockaddr_in sin;
		socklen_t len = sizeof(sin);
		int fd;

		fd = accept(env->sock, (struct sockaddr*)&sin, &len);
		if (fd == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return 1;
		}
		return handle_conn(env, fd);
	}
	return 0;
}

static int
run_connect(struct env *env)
{
	if (resolve_addr(env, &env->dst, env->dst_host, env->dst_port))
		return 1;
	if (open_sock(env))
		return 1;
	if (connect(env->sock, (struct sockaddr*)&env->dst,
	            sizeof(env->dst)) == -1)
	{
		fprintf(stderr, "%s: connect: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return handle_conn(env, env->sock);
}

static void
usage(const char *progname)
{
	printf("%s [-h] [-s source] [-p port] [-l] [destination] [port]\n", progname);
	printf("-h: show this help\n");
	printf("-s source: set the source address\n");
	printf("-p port  : set the source port\n");
	printf("-l       : enable listen mode\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "s:p:lh")) != -1)
	{
		switch (c)
		{
			case 's':
				env.src_host = optarg;
				break;
			case 'p':
				env.src_port = optarg;
				break;
			case 'l':
				env.opt |= OPT_l;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (env.opt & OPT_l)
	{
		if (optind != argc)
		{
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
		}
		if (run_listen(&env))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	if (argc - optind < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc - optind > 2)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	env.dst_host = argv[optind + 0];
	env.dst_port = argv[optind + 1];
	if (run_connect(&env))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
