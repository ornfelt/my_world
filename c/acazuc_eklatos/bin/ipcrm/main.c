#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define OPT_M (1 << 0)
#define OPT_m (1 << 1)
#define OPT_Q (1 << 2)
#define OPT_q (1 << 3)
#define OPT_S (1 << 4)
#define OPT_s (1 << 5)

struct env
{
	const char *progname;
	int opt;
	key_t key;
	int id;
};

static int
rm_shm_id(struct env *env)
{
	if (shmctl(env->id, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
rm_shm_key(struct env *env)
{
	env->id = shmget(env->key, 0, 0);
	if (env->id == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return rm_shm_id(env);
}

static int
rm_sem_id(struct env *env)
{
	if (semctl(env->id, 0, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: semctl: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
rm_sem_key(struct env *env)
{
	env->id = semget(env->key, 0, 0);
	if (env->id == -1)
	{
		fprintf(stderr, "%s: semget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return rm_sem_id(env);
}

static int
rm_msg_id(struct env *env)
{
	if (msgctl(env->id, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: msgctl: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
rm_msg_key(struct env *env)
{
	env->id = msgget(env->key, 0);
	if (env->id == -1)
	{
		fprintf(stderr, "%s: msgget: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return rm_msg_id(env);
}

static int
parse_key(struct env *env, const char *value)
{
	char *endptr;

	errno = 0;
	env->key = strtoul(value, &endptr, 0);
	if (errno || *endptr || !env->key)
	{
		fprintf(stderr, "%s: invalid key value\n", env->progname);
		return 1;
	}
	return 0;
}

static int
parse_id(struct env *env, const char *value)
{
	char *endptr;

	errno = 0;
	env->id = strtol(value, &endptr, 0);
	if (errno || *endptr || env->id < 0)
	{
		fprintf(stderr, "%s: invalid key value\n", env->progname);
		return 1;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-M key] [-m id] [-Q key] [-q id] [-S key] [-s id] [-h]\n", progname);
	printf("-M key: remove the shared memory segment with the given key\n");
	printf("-m id : remove the shared memory segment with the given id\n");
	printf("-Q key: remove the message queue with the given key\n");
	printf("-q id : remove the message queue with the given id\n");
	printf("-S key: remove the semaphore with the given key\n");
	printf("-s id : remove the semaphore with the given id\n");
	printf("-h    : show this help\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "M:m:Q:q:S:s:h")) != -1)
	{
		switch (c)
		{
			case 'M':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_key(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_M;
				break;
			case 'm':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_id(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_m;
				break;
			case 'Q':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_key(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_Q;
				break;
			case 'q':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_id(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_q;
				break;
			case 'S':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_key(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_S;
				break;
			case 's':
				if (env.opt & (OPT_M | OPT_m | OPT_Q | OPT_q | OPT_S | OPT_s))
				{
					fprintf(stderr, "%s: only one operation can be given\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (parse_id(&env, optarg))
					return EXIT_FAILURE;
				env.opt |= OPT_s;
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
		if (rm_shm_key(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_m)
	{
		if (rm_shm_id(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_Q)
	{
		if (rm_msg_key(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_q)
	{
		if (rm_msg_id(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_S)
	{
		if (rm_sem_key(&env))
			return EXIT_FAILURE;
	}
	if (env.opt & OPT_s)
	{
		if (rm_sem_id(&env))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
