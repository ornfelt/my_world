#include "nmap.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>

static int
build_addr(struct env *env, struct host *host)
{
	host->addrlen = sizeof(struct sockaddr_in);
	host->addr = malloc(host->addrlen);
	if (!host->addr)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	memset(host->addr, 0, host->addrlen);
	host->addr->sa_family = AF_INET;
	if (!inet_aton(host->ip, &((struct sockaddr_in*)host->addr)->sin_addr))
	{
		fprintf(stderr, "%s: inet_aton failed\n", env->progname);
		return 1;
	}
	return 0;
}

static int
resolve_ip(struct env *env, struct host *host)
{
	struct hostent *hostent;
	struct in_addr *tmp;

	hostent = gethostbyname(host->host);
	if (!hostent)
	{
		fprintf(stderr, "%s: failed to resolve host\n", env->progname);
		return 1;
	}
	if (hostent->h_addrtype != AF_INET)
	{
		fprintf(stderr, "%s: host ip isn't ipv4\n", env->progname);
		return 1;
	}
	if (hostent->h_length < (int)sizeof(struct in_addr))
	{
		fprintf(stderr, "%s: invalid host ip length\n", env->progname);
		return 1;
	}
	tmp = (struct in_addr*)hostent->h_addr_list[0];
	host->ip = inet_ntoa(*tmp);
	if (!host->ip)
	{
		fprintf(stderr, "%s: failed to get ip string\n", env->progname);
		return 1;
	}
	host->ip = strdup(host->ip);
	if (!host->ip)
	{
		fprintf(stderr, "%s: strdup: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
push_host(struct env *env, struct host *host)
{
	struct host **hosts;

	hosts = realloc(env->hosts, sizeof(*hosts) * (env->hosts_count + 1));
	if (!hosts)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	hosts[env->hosts_count++] = host;
	env->hosts = hosts;
	return 0;
}

int
build_hosts(struct env *env)
{
	for (size_t i = 0; i < env->ips_count; ++i)
	{
		struct host *host = malloc(sizeof(*host));
		if (!host)
		{
			fprintf(stderr, "%s: malloc: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		memset(host, 0, sizeof(*host));
		TAILQ_INIT(&host->packets_tcp);
		TAILQ_INIT(&host->packets_icmp);
		host->host = env->ips[i];
		if (resolve_ip(env, host)
		 || build_addr(env, host)
		 || push_host(env, host))
		{
			free(host->ip);
			free(host);
			return 1;
		}
	}
	return 0;
}
