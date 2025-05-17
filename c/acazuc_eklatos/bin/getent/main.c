#include <arpa/inet.h>

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>

struct env
{
	const char *progname;
};

static void
print_services_line(struct servent *servent)
{
	printf("%s %" PRIu16 "/%s\n",
	       servent->s_name,
	       htons(servent->s_port),
	       servent->s_proto);
}

static void
print_services(char **keys)
{
	struct servent *servent;

	if (*keys)
	{
		do
		{
			servent = getservbyname(*keys, NULL);
			if (servent)
				print_services_line(servent);
			keys++;
		} while (*keys);
		return;
	}
	while ((servent = getservent()))
		print_services_line(servent);
}

static void
print_hosts_line(struct hostent *hostent)
{
	char buf[256];

	printf("%s", hostent->h_name);
	for (size_t i = 0; hostent->h_addr_list[i]; ++i)
	{
		printf(" %s", inet_ntop(hostent->h_addrtype,
		                        hostent->h_addr_list[i],
		                        buf, sizeof(buf)));
	}
	printf("\n");
}

static void
print_hosts(char **keys)
{
	struct hostent *hostent;

	if (*keys)
	{
		do
		{
			hostent = gethostbyname(*keys);
			if (hostent)
				print_hosts_line(hostent);
			keys++;
		} while (*keys);
		return;
	}
	while ((hostent = gethostent()))
		print_hosts_line(hostent);
}

static void
print_protocols_line(const struct protoent *protoent)
{
	printf("%s %d\n", protoent->p_name, protoent->p_proto);
}

static void 
print_protocols(char **keys)
{
	struct protoent *protoent;

	if (*keys)
	{
		do
		{
			protoent = getprotobyname(*keys);
			if (protoent)
				print_protocols_line(protoent);
			keys++;
		} while (*keys);
		return;
	}
	while ((protoent = getprotoent()))
		print_protocols_line(protoent);
}

static void
print_group_line(const struct group *group)
{
	printf("%s:%s:%" PRId32 ":\n", group->gr_name, group->gr_passwd,
	       group->gr_gid);
}

static void
print_group(char **keys)
{
	struct group *group;

	if (*keys)
	{
		do
		{
			group = getgrnam(*keys);
			if (group)
				print_group_line(group);
			keys++;
		} while (*keys);
		return;
	}
	while ((group = getgrent()))
		print_group_line(group);
}

static void
print_passwd_line(const struct passwd *passwd)
{
	printf("%s:%s:%" PRId32 ":%" PRId32 ":%s:%s:%s\n",
	       passwd->pw_name,
	       passwd->pw_passwd,
	       passwd->pw_uid,
	       passwd->pw_gid,
	       passwd->pw_gecos,
	       passwd->pw_dir,
	       passwd->pw_shell);
}

static void
print_passwd(char **keys)
{
	struct passwd *passwd;

	if (*keys)
	{
		do
		{
			passwd = getpwnam(*keys);
			if (passwd)
				print_passwd_line(passwd);
			keys++;
		} while (*keys);
		return;
	}
	while ((passwd = getpwent()))
		print_passwd_line(passwd);
}

static void
print_networks_line(const struct netent *netent)
{
	printf("%s %s\n",
	       netent->n_name,
	       inet_ntoa(*(struct in_addr*)&netent->n_net));
}

static void
print_networks(char **keys)
{
	struct netent *netent;

	if (*keys)
	{
		do
		{
			netent = getnetbyname(*keys);
			if (netent)
				print_networks_line(netent);
			keys++;
		} while (*keys);
		return;
	}
	while ((netent = getnetent()))
		print_networks_line(netent);
}

static void
usage(const char *progname)
{
	printf("%s [-h] database [KEYS]\n", progname);
	printf("-h      : display this help\n");
	printf("database: one of group, hosts, networks, passwd, protocols, services\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!strcmp(argv[optind], "group"))
	{
		print_group(&argv[optind + 1]);
	}
	else if (!strcmp(argv[optind], "hosts"))
	{
		print_hosts(&argv[optind + 1]);
	}
	else if (!strcmp(argv[optind], "networks"))
	{
		print_networks(&argv[optind + 1]);
	}
	else if (!strcmp(argv[optind], "passwd"))
	{
		print_passwd(&argv[optind + 1]);
	}
	else if (!strcmp(argv[optind], "protocols"))
	{
		print_protocols(&argv[optind + 1]);
	}
	else if (!strcmp(argv[optind], "services"))
	{
		print_services(&argv[optind + 1]);
	}
	else
	{
		fprintf(stderr, "%s: unknown database\n", argv[0]);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
