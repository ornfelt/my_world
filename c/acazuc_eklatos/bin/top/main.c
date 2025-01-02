#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

struct entry
{
	char name[33];
	pid_t ppid;
	pid_t pid;
	struct timespec utime;
	struct timespec stime;
};

struct env
{
	const char *progname;
	struct entry *entries;
	size_t entries_nb;
	size_t entries_size;
	size_t prev_entries_nb;
};

static struct entry *add_entry(struct env *env)
{
	if (env->entries_nb >= env->entries_size)
	{
		size_t new_size = env->entries_size * 2;
		if (new_size < 32)
			new_size = 32;
		struct entry *entries = realloc(env->entries,
		                                sizeof(*entries) * new_size);
		if (!entries)
		{
			fprintf(stderr, "%s: malloc: %s\n", env->progname,
			        strerror(errno));
			return NULL;
		}
		env->entries = entries;
		env->entries_size = new_size;
	}
	return &env->entries[env->entries_nb++];
}

static int get_entries(struct env *env)
{
	int ret = 1;
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	fp = fopen("/sys/procinfo", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		goto end;
	}
	env->entries_nb = 0;
	while ((getline(&line, &size, fp)) > 0)
	{
		struct entry *entry = add_entry(env);
		if (!entry)
			goto end;
		size_t ums;
		size_t sms;
		if (sscanf(line, "%d %d %*s %32s %zd.%zdu %zd.%zds",
		           &entry->pid,
		           &entry->ppid,
		           entry->name,
		           &entry->utime.tv_sec,
		           &ums,
		           &entry->stime.tv_sec,
		           &sms) != 7)
		{
			fprintf(stderr, "%s: invalid line\n", env->progname);
			goto end;
		}
		entry->utime.tv_nsec = ums * 1000000;
		entry->stime.tv_nsec = sms * 1000000;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	free(line);
	return ret;
}

static int display(struct env *env)
{
	static const char equals[] = "======================================================================================================================================================";
	if (env->prev_entries_nb)
	{
		printf("\033[F\033[A");
		for (size_t i = 0; i < env->prev_entries_nb; ++i)
			printf("\033[A");
	}
	printf("%-10s %-10s %-32.32s %-10s %-10s\n",
	       "pid", "ppid", "name", "user time", "sys time");
	printf("%.76s\n", equals);
	for (size_t i = 0; i < env->entries_nb; ++i)
	{
		struct entry *entry = &env->entries[i];
		printf("%-10" PRId32 " %-10" PRId32 " %-32.32s %6" PRId64 ".%03" PRId64 " %6" PRId64 ".%03" PRId64 "\n",
		       entry->pid,
		       entry->ppid,
		       entry->name,
		       entry->utime.tv_sec,
		       entry->utime.tv_nsec / 1000000,
		       entry->stime.tv_sec,
		       entry->stime.tv_nsec / 1000000);
	}
	env->prev_entries_nb = env->entries_nb;
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
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
				usage(argv[1]);
				return EXIT_FAILURE;
		}
	}
	while (1)
	{
		int ret = get_entries(&env);
		if (ret)
			return EXIT_FAILURE;
		display(&env);
		sleep(1);
	}
	return EXIT_SUCCESS;
}
