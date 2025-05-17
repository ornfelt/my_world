#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define OPT_M (1 << 0)
#define OPT_Q (1 << 1)
#define OPT_S (1 << 2)

struct env
{
	const char *progname;
	mode_t mode;
	unsigned arg;
	int opt;
};

static int
create_shm(struct env *env)
{
	int shmid;

	shmid = shmget(IPC_PRIVATE, env->arg, env->mode);
	if (shmid == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Shared memory id: %d\n", shmid);
	return 0;
}

static int
create_msg(struct env *env)
{
	int msgid;

	msgid = msgget(IPC_PRIVATE, env->mode);
	if (msgid == -1)
	{
		fprintf(stderr, "%s: msgget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Message queue id: %d\n", msgid);
	return 0;
}

static int
create_sem(struct env *env)
{
	int semid;

	semid = semget(IPC_PRIVATE, env->arg, env->mode);
	if (semid == -1)
	{
		fprintf(stderr, "%s: semget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Semaphore id: %d\n", semid);
	return 0;
}

static int
parse_mode(const char *progname, const char *str, mode_t *mode)
{
	*mode = 0;
	for (size_t i = 0; str[i]; ++i)
	{
		if (str[i] < '0' || str[i] > '7')
		{
			fprintf(stderr, "%s: invalid mode\n", progname);
			return 1;
		}
		*mode = *mode * 8 + str[i] - '0';
		if (*mode > 07777)
		{
			fprintf(stderr, "%s: mode out of bounds\n", progname);
			return 1;
		}
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-M size] [-Q] [-S number] [-p mode] [-h]\n", progname);
	printf("-M size  : create a shared memory segment with the given size\n");
	printf("-Q       : create a message queue\n");
	printf("-S number: create a semaphore the the given number of elements\n");
	printf("-p mode  : create an object with the given mode\n");
	printf("-h       : display this help\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.mode = 0644;
	while ((c = getopt(argc, argv, "M:QS:p:h")) != -1)
	{
		switch (c)
		{
			case 'M':
			{
				if (env.opt & (OPT_M | OPT_Q | OPT_S))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				char *endptr;
				errno = 0;
				env.arg = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid size\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				env.opt |= OPT_M;
				break;
			}
			case 'Q':
				if (env.opt & (OPT_M | OPT_Q | OPT_S))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				env.opt |= OPT_Q;
				break;
			case 'S':
			{
				if (env.opt & (OPT_M | OPT_Q | OPT_S))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				char *endptr;
				errno = 0;
				env.arg = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid number\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				env.opt |= OPT_S;
				break;
			}
			case 'p':
				if (parse_mode(argv[0], optarg, &env.mode))
					return EXIT_FAILURE;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (env.opt & OPT_M)
	{
		if (create_shm(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_Q)
	{
		if (create_msg(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_S)
	{
		if (create_sem(&env))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
