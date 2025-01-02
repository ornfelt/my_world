#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define OPT_m (1 << 0)
#define OPT_q (1 << 1)
#define OPT_s (1 << 2)
#define OPT_i (1 << 3)
#define OPT_l (1 << 4)
#define OPT_p (1 << 5)
#define OPT_t (1 << 6)
#define OPT_c (1 << 7)

struct env
{
	const char *progname;
	int opt;
	int id;
};

struct ipc_limits
{
	unsigned long shmall;
	unsigned long shmmin;
	unsigned long shmmax;
	unsigned long shmlba;
	unsigned long shmseg;
	unsigned long shmmni;
	unsigned long semopm;
	unsigned long semmsl;
	unsigned long semmni;
	unsigned long semmns;
	unsigned long semvmx;
	unsigned long msgmni;
	unsigned long msgmax;
	unsigned long msgmnb;
};

typedef void (*print_msg_t)(int msgid, struct msgid_ds *ds);
typedef void (*print_shm_t)(int shmid, struct shmid_ds *ds);
typedef void (*print_sem_t)(int semid, struct semid_ds *ds);

static const char *time_str(time_t t)
{
	static char buf[1024];
	if (!t)
	{
		strlcpy(buf, "No set", sizeof(buf));
		return buf;
	}
	struct tm tm;
	if (!localtime_r(&t, &tm))
		return NULL;
	if (!strftime(buf, sizeof(buf), "%c", &tm))
		return NULL;
	return buf;
}

static const char *uid_str(uid_t uid)
{
	static char buf[128];
	struct passwd *pw = getpwuid(uid);
	if (pw)
		strlcpy(buf, pw->pw_name, sizeof(buf));
	else
		snprintf(buf, sizeof(buf), "%" PRId32, uid);
	return buf;
}

static const char *gid_str(gid_t gid)
{
	static char buf[128];
	struct group *gr = getgrgid(gid);
	if (gr)
		strlcpy(buf, gr->gr_name, sizeof(buf));
	else
		snprintf(buf, sizeof(buf), "%" PRId32, gid);
	return buf;
}

static int print_single_msg(struct env *env)
{
	struct msgid_ds ds;
	int ret = msgctl(env->id, IPC_STAT, &ds);
	if (ret == -1)
	{
		fprintf(stderr, "%s: msgctl: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Message queue msgid=%d\n", env->id);
	printf("uid=%" PRId32 "\tgid=%" PRId32 "\t"
	       "cuid=%" PRId32 "\tcgid=%" PRId32 "\n",
	       ds.msg_perm.uid, ds.msg_perm.gid,
	       ds.msg_perm.cuid, ds.msg_perm.cgid);
	printf("mode=%o\tseq=%u\tkey=0x%08zx\n",
	       ds.msg_perm.mode, ds.msg_perm.seq, ds.msg_perm.key);
	printf("cbytes: %lu\tqnum: %zu\tqbytes: %zu\n",
	       ds.msg_cbytes, ds.msg_qnum, ds.msg_qbytes);
	printf("lspid: %" PRId32 "\tlrpid: %" PRId32 "\n",
	       ds.msg_lspid, ds.msg_lrpid);
	printf("stime: %s\n", time_str(ds.msg_stime));
	printf("rtime: %s\n", time_str(ds.msg_rtime));
	printf("ctime: %s\n", time_str(ds.msg_ctime));
	return 0;
}

static void print_msg(int msgid, struct msgid_ds *ds)
{
	printf("0x%08zx ", ds->msg_perm.key);
	printf("%-10d ", msgid);
	printf("%-10s ", uid_str(ds->msg_perm.uid));
	printf("%-10o ", ds->msg_perm.mode);
	printf("%-10lu ", ds->msg_cbytes);
	printf("%zu\n", ds->msg_qnum);
}

static void print_msg_pid(int msgid, struct msgid_ds *ds)
{
	printf("%-10d ", msgid);
	printf("%-10s ", uid_str(ds->msg_perm.uid));
	printf("%-10" PRId32 " ", ds->msg_lspid);
	printf("%" PRId32 "\n", ds->msg_lrpid);
}

static void print_msg_time(int msgid, struct msgid_ds *ds)
{
	printf("%-10d ", msgid);
	printf("%-10s ", uid_str(ds->msg_perm.uid));
	printf("%-25s ", time_str(ds->msg_stime));
	printf("%-25s ", time_str(ds->msg_rtime));
	printf("%s\n", time_str(ds->msg_ctime));
}

static void print_msg_ugid(int msgid, struct msgid_ds *ds)
{
	printf("%-10d ", msgid);
	printf("%-10o ", ds->msg_perm.mode);
	printf("%-10s ", uid_str(ds->msg_perm.cuid));
	printf("%-10s ", gid_str(ds->msg_perm.cgid));
	printf("%-10s ", uid_str(ds->msg_perm.uid));
	printf("%s\n", gid_str(ds->msg_perm.gid));
}

static void print_msgs(struct env *env, print_msg_t print_fn)
{
	FILE *fp = fopen("/sys/sysv/msglist", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return;
	}
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) > 0)
	{
		char *endptr;
		errno = 0;
		long msgid = strtol(line, &endptr, 10);
		if (errno || *endptr != '\n')
		{
			fprintf(stderr, "%s: invalid msg line\n",
			        env->progname);
			break;
		}
		struct msgid_ds ds;
		int ret = msgctl(msgid, IPC_STAT, &ds);
		if (ret == -1)
		{
			fprintf(stderr, "%s: msgctl: %s\n", env->progname,
			        strerror(errno));
			break;
		}
		print_fn(msgid, &ds);
	}
	fclose(fp);
}

static int print_single_shm(struct env *env)
{
	struct shmid_ds ds;
	int ret = shmctl(env->id, IPC_STAT, &ds);
	if (ret == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Shared memory segment shmid=%d\n", env->id);
	printf("uid=%" PRId32 "\tgid=%" PRId32 "\t"
	       "cuid=%" PRId32 "\tcgid=%" PRId32 "\n",
	       ds.shm_perm.uid, ds.shm_perm.gid,
	       ds.shm_perm.cuid, ds.shm_perm.cgid);
	printf("mode=%o\tseq=%u\tkey=0x%08zx\n",
	       ds.shm_perm.mode, ds.shm_perm.seq, ds.shm_perm.key);
	printf("segsz: %zu\tnattch: %zu\n", ds.shm_segsz, ds.shm_nattch);
	printf("cpid: %" PRId32 "\tlpid: %" PRId32 "\n",
	       ds.shm_cpid, ds.shm_lpid);
	printf("atime: %s\n", time_str(ds.shm_atime));
	printf("dtime: %s\n", time_str(ds.shm_dtime));
	printf("ctime: %s\n", time_str(ds.shm_ctime));
	return 0;
}

static void print_shm(int shmid, struct shmid_ds *ds)
{
	printf("0x%08zx ", ds->shm_perm.key);
	printf("%-10d ", shmid);
	printf("%-10s ", uid_str(ds->shm_perm.uid));
	printf("%-10o ", ds->shm_perm.mode);
	printf("%-10zu ", ds->shm_segsz);
	printf("%zu\n", ds->shm_nattch);
}

static void print_shm_pid(int shmid, struct shmid_ds *ds)
{
	printf("%-10u ", shmid);
	printf("%-10s ", uid_str(ds->shm_perm.uid));
	printf("%-10" PRId32 " ", ds->shm_cpid);
	printf("%" PRId32 "\n", ds->shm_lpid);
}

static void print_shm_time(int shmid, struct shmid_ds *ds)
{
	printf("%-10u ", shmid);
	printf("%-10s ", uid_str(ds->shm_perm.uid));
	printf("%-25s ", time_str(ds->shm_atime));
	printf("%-25s ", time_str(ds->shm_dtime));
	printf("%s\n", time_str(ds->shm_ctime));
}

static void print_shm_ugid(int shmid, struct shmid_ds *ds)
{
	printf("%-10u ", shmid);
	printf("%-10o ", ds->shm_perm.mode);
	printf("%-10s ", uid_str(ds->shm_perm.cuid));
	printf("%-10s ", gid_str(ds->shm_perm.cgid));
	printf("%-10s ", uid_str(ds->shm_perm.uid));
	printf("%s\n", gid_str(ds->shm_perm.gid));
}

static void print_shms(struct env *env, print_shm_t print_fn)
{
	FILE *fp = fopen("/sys/sysv/shmlist", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return;
	}
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) > 0)
	{
		char *endptr;
		errno = 0;
		long shmid = strtol(line, &endptr, 10);
		if (errno || *endptr != '\n')
		{
			fprintf(stderr, "%s: invalid shm line\n",
			        env->progname);
			break;
		}
		struct shmid_ds ds;
		int ret = shmctl(shmid, IPC_STAT, &ds);
		if (ret == -1)
		{
			fprintf(stderr, "%s: shmctl: %s\n", env->progname,
			        strerror(errno));
			break;
		}
		print_fn(shmid, &ds);
	}
	fclose(fp);
}

static int print_single_sem(struct env *env)
{
	struct semid_ds ds;
	int ret = semctl(env->id, 0, IPC_STAT, &ds);
	if (ret == -1)
	{
		fprintf(stderr, "%s: semctl: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	printf("Semaphore semid=%d\n", env->id);
	printf("uid=%" PRId32 "\tgid=%" PRId32 "\t"
	       "cuid=%" PRId32 "\tcgid=%" PRId32 "\n",
	       ds.sem_perm.uid, ds.sem_perm.gid,
	       ds.sem_perm.cuid, ds.sem_perm.cgid);
	printf("mode=%o\tseq=%u\tkey=0x%08zx\n",
	       ds.sem_perm.mode, ds.sem_perm.seq, ds.sem_perm.key);
	printf("nsems: %lu\n", ds.sem_nsems);
	printf("otime: %s\n", time_str(ds.sem_otime));
	printf("ctime: %s\n", time_str(ds.sem_ctime));
	return 0;
}

static void print_sem(int semid, struct semid_ds *ds)
{
	printf("0x%08zx ", ds->sem_perm.key);
	printf("%-10d ", semid);
	printf("%-10s ", uid_str(ds->sem_perm.uid));
	printf("%-10o ", ds->sem_perm.mode);
	printf("%lu\n", ds->sem_nsems);
}

static void print_sem_time(int semid, struct semid_ds *ds)
{
	printf("%-10d ", semid);
	printf("%-10s ", uid_str(ds->sem_perm.uid));
	printf("%-25s ", time_str(ds->sem_otime));
	printf("%s\n", time_str(ds->sem_ctime));
}

static void print_sem_ugid(int semid, struct semid_ds *ds)
{
	printf("%-10d ", semid);
	printf("%-10o ", ds->sem_perm.mode);
	printf("%-10s ", uid_str(ds->sem_perm.cuid));
	printf("%-10s ", gid_str(ds->sem_perm.cgid));
	printf("%-10s ", uid_str(ds->sem_perm.uid));
	printf("%s\n", gid_str(ds->sem_perm.gid));
}

static void print_sems(struct env *env, print_sem_t print_fn)
{
	FILE *fp = fopen("/sys/sysv/semlist", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return;
	}
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) > 0)
	{
		char *endptr;
		errno = 0;
		long semid = strtol(line, &endptr, 10);
		if (errno || *endptr != '\n')
		{
			fprintf(stderr, "%s: invalid sem line\n",
			        env->progname);
			break;
		}
		struct semid_ds ds;
		int ret = semctl(semid, 0, IPC_STAT, &ds);
		if (ret == -1)
		{
			fprintf(stderr, "%s: semctl: %s\n", env->progname,
			        strerror(errno));
			break;
		}
		print_fn(semid, &ds);
	}
	fclose(fp);
}

static int get_limits(struct env *env, struct ipc_limits *limits)
{
	FILE *fp = fopen("/sys/sysv/limits", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	memset(limits, 0, sizeof(*limits));
	char *line = NULL;
	size_t size = 0;
	while (getline(&line, &size, fp) > 0)
	{
#define TEST_LIMIT(name, entry) \
do \
{ \
		if (memcmp(#name " ", line, 7)) \
			break; \
		errno = 0; \
		char *endptr; \
		limits->entry = strtoul(line + 7, &endptr, 10); \
		if (errno || *endptr != '\n') \
		{ \
			fprintf(stderr, "%s: invalid limit value: %s\n", \
			        env->progname, line); \
			fclose(fp); \
			return 1; \
		} \
} while (0)
		TEST_LIMIT(SHMALL, shmall);
		TEST_LIMIT(SHMMIN, shmmin);
		TEST_LIMIT(SHMMAX, shmmax);
		TEST_LIMIT(SHMLBA, shmlba);
		TEST_LIMIT(SHMSEG, shmseg);
		TEST_LIMIT(SHMMNI, shmmni);
		TEST_LIMIT(SEMOPM, semopm);
		TEST_LIMIT(SEMMSL, semmsl);
		TEST_LIMIT(SEMMNI, semmni);
		TEST_LIMIT(SEMMNS, semmns);
		TEST_LIMIT(SEMVMX, semvmx);
		TEST_LIMIT(MSGMNI, msgmni);
		TEST_LIMIT(MSGMAX, msgmax);
		TEST_LIMIT(MSGMNB, msgmnb);
#undef TEST_LIMIT
	}
	fclose(fp);
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-m] [-q] [-s] [-a] [-i id] [-l] [-p] [-t] [-c] [-h]\n", progname);
	printf("-m   : display shared memory\n");
	printf("-q   : display message queues\n");
	printf("-s   : display semaphores\n");
	printf("-a   : display all the above\n");
	printf("-i id: display detailed informations about the given id (required -m, -q or -s)\n");
	printf("-l   : display limits\n");
	printf("-p   : display PIDs of creator and last operator\n");
	printf("-t   : display attach, detach and change time\n");
	printf("-c   : display perms, creators and owners\n");
	printf("-h   : display this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hmqsai:lptc")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			case 'm':
				env.opt |= OPT_m;
				break;
			case 'q':
				env.opt |= OPT_q;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 'a':
				env.opt |= OPT_m | OPT_q | OPT_s;
				break;
			case 'i':
			{
				env.opt |= OPT_i;
				char *endptr;
				errno = 0;
				env.id = strtol(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid id\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'l':
				env.opt &= ~(OPT_l | OPT_p | OPT_t | OPT_c);
				env.opt |= OPT_l;
				break;
			case 'p':
				env.opt &= ~(OPT_l | OPT_p | OPT_t | OPT_c);
				env.opt |= OPT_p;
				break;
			case 't':
				env.opt &= ~(OPT_l | OPT_p | OPT_t | OPT_c);
				env.opt |= OPT_t;
				break;
			case 'c':
				env.opt &= ~(OPT_l | OPT_p | OPT_t | OPT_c);
				env.opt |= OPT_c;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!(env.opt & (OPT_m | OPT_q | OPT_s)))
		env.opt |= OPT_m | OPT_q | OPT_s;
	if (env.opt & OPT_i)
	{
		switch (env.opt & (OPT_m | OPT_q | OPT_s))
		{
			case OPT_q:
				printf("\n");
				if (print_single_msg(&env))
					return EXIT_FAILURE;
				printf("\n");
				return EXIT_SUCCESS;
			case OPT_m:
				printf("\n");
				if (print_single_shm(&env))
					return EXIT_FAILURE;
				printf("\n");
				return EXIT_SUCCESS;
			case OPT_s:
				printf("\n");
				if (print_single_sem(&env))
					return EXIT_FAILURE;
				printf("\n");
				return EXIT_SUCCESS;
			default:
				fprintf(stderr, "%s: when using an ID, a single resource must be specified\n", argv[0]);
				return EXIT_FAILURE;
		}
	}
	printf("\n");
	if (env.opt & OPT_l)
	{
		struct ipc_limits limits;
		if (get_limits(&env, &limits))
			return EXIT_FAILURE;
		if (env.opt & OPT_q)
		{
			printf("------ Messages Limits --------\n");
			printf("max queues system wide = %lu\n",
			       limits.msgmni);
			printf("max size of message (bytes) = %lu\n",
			       limits.msgmax);
			printf("default max size of queue (bytes) = %lu\n",
			       limits.msgmnb);
			printf("\n");
		}
		if (env.opt & OPT_m)
		{
			printf("------ Shared Memory Limits --------\n");
			printf("max number of segments = %lu\n",
			       limits.shmmni);
			printf("max seg size (kbytes) = %lu\n",
			       limits.shmmax / 1024);
			printf("max total shared memory (kbytes) = %lu\n",
			       limits.shmall);
			printf("min seg size (bytes) = %lu\n",
			       limits.shmmin);
			printf("\n");
		}
		if (env.opt & OPT_s)
		{
			printf("------ Semaphore Limits --------\n");
			printf("max number of arrays = %lu\n",
			       limits.semmni);
			printf("max semaphores per array = %lu\n",
			       limits.semmsl);
			printf("max semaphores system wide = %lu\n",
			       limits.semmns);
			printf("max ops per semop call = %lu\n",
			       limits.semopm);
			printf("semaphore max value = %lu\n",
			       limits.semvmx);
			printf("\n");
		}
	}
	else if (env.opt & OPT_p)
	{
		if (env.opt & OPT_q)
		{
			printf("------ Message Queues PIDs --------\n");
			printf("%-10s %-10s %-10s %s\n",
			       "msgqid", "owner", "lspid", "lrpid");
			print_msgs(&env, print_msg_pid);
			printf("\n");
		}
		if (env.opt & OPT_m)
		{
			printf("------ Shared Memory Creator/Last-op PIDs --------\n");
			printf("%-10s %-10s %-10s %s\n",
			       "shmid", "owner", "cpid", "lpid");
			print_shms(&env, print_shm_pid);
			printf("\n");
		}
	}
	else if (env.opt & OPT_t)
	{
		if (env.opt & OPT_q)
		{
			printf("------ Message Queues Send/Recv/Change Times --------\n");
			printf("%-10s %-10s %-25s %-25s %s\n",
			       "msqid", "owner", "send", "recv", "change");
			print_msgs(&env, print_msg_time);
			printf("\n");
		}
		if (env.opt & OPT_m)
		{
			printf("------ Shared Memory Attach/Detach/Change Times --------\n");
			printf("%-10s %-10s %-25s %-25s %s\n",
			       "shmid", "owner", "attached", "detached", "changed");
			print_shms(&env, print_shm_time);
			printf("\n");
		}
		if (env.opt & OPT_s)
		{
			printf("------ Semaphore Operation/Change Times --------\n");
			printf("%-10s %-10s %-25s %s\n",
			       "semid", "owner", "last-op", "last-changed");
			print_sems(&env, print_sem_time);
			printf("\n");
		}
	}
	else if (env.opt & OPT_c)
	{
		if (env.opt & OPT_q)
		{
			printf("------ Message Queues Creators/Owners --------\n");
			printf("%-10s %-10s %-10s %-10s %-10s %s\n",
			       "msqid", "perms", "cuid", "cgid", "uid", "gid");
			print_msgs(&env, print_msg_ugid);
			printf("\n");
		}
		if (env.opt & OPT_m)
		{
			printf("------ Shared Memory Segment Creators/Owners --------\n");
			printf("%-10s %-10s %-10s %-10s %-10s %s\n",
			       "shmid", "perms", "cuid", "cgid", "uid", "gid");
			print_shms(&env, print_shm_ugid);
			printf("\n");
		}
		if (env.opt & OPT_s)
		{
			printf("------ Semaphore Arrays Creators/Owners --------\n");
			printf("%-10s %-10s %-10s %-10s %-10s %s\n",
			       "semid", "perms", "cuid", "cgid", "uid", "gid");
			print_sems(&env, print_sem_ugid);
			printf("\n");
		}
	}
	else
	{
		if (env.opt & OPT_q)
		{
			printf("------ Message Queues --------\n");
			printf("%-10s %-10s %-10s %-10s %-10s %s\n",
			       "key", "msqid", "owner", "perms", "used-bytes", "messages");
			print_msgs(&env, print_msg);
			printf("\n");
		}
		if (env.opt & OPT_m)
		{
			printf("------ Shared Memory Segments --------\n");
			printf("%-10s %-10s %-10s %-10s %-10s %s\n",
			       "key", "shmid", "owner", "perms", "bytes", "nattch");
			print_shms(&env, print_shm);
			printf("\n");
		}
		if (env.opt & OPT_s)
		{
			printf("------ Semaphore Arrays --------\n");
			printf("%-10s %-10s %-10s %-10s %s\n",
			       "key", "semid", "owner", "perms", "nsems");
			print_sems(&env, print_sem);
			printf("\n");
		}
	}
	return EXIT_SUCCESS;
}
