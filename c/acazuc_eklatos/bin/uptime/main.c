#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define OPT_s (1 << 0)

struct env
{
	const char *progname;
	int opt;
};

static char *get_file_line(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return NULL;
	}
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, fp) < 0)
	{
		fprintf(stderr, "%s: read: %s\n", env->progname,
		        strerror(errno));
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	return line;
}

static int get_uptime(struct env *env, time_t *t)
{
	int ret = 1;
	char *line = get_file_line(env, "/sys/uptime");
	if (!line)
		return 1;
	char *endptr;
	errno = 0;
	*t = strtoul(line, &endptr, 10);
	if (errno || *endptr != '.')
	{
		fprintf(stderr, "%s: invalid uptime\n", env->progname);
		goto end;
	}
	ret = 0;

end:
	free(line);
	return ret;
}

static int get_loadavg(struct env *env, uint8_t *loadavg)
{
	double avg[3];
	if (getloadavg(avg, 3) == -1)
	{
		fprintf(stderr, "%s: failed to get loadavg\n", env->progname);
		return 1;
	}
	loadavg[0] = avg[0];
	loadavg[1] = (int)(avg[0] * 100) % 100;
	loadavg[2] = avg[1];
	loadavg[3] = (int)(avg[1] * 100) % 100;
	loadavg[4] = avg[2];
	loadavg[5] = (int)(avg[2] * 100) % 100;
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-s]\n", progname);
	printf("-h: display this help\n");
	printf("-s: display boot time in yyyy-mm-dd HH:MM:SS format\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hs")) != -1)
	{
		switch (c)
		{
			case 's':
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
	if (env.opt & OPT_s)
	{
		time_t t = time(NULL);
		time_t uptime;
		if (get_uptime(&env, &uptime))
			return 1;
		t -= uptime;
		struct tm *tm = localtime(&t);
		char buf[64];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
		printf("%s\n", buf);
	}
	else
	{
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		uint8_t loadavg[6];
		if (get_loadavg(&env, loadavg))
			return 1;
		printf("%02u:%02u:%02u, 1 user, load averages: %u.%02u, %u.%02u, %u.%02u\n",
		       tm->tm_hour, tm->tm_min, tm->tm_sec,
		       loadavg[0], loadavg[1],
		       loadavg[2], loadavg[3],
		       loadavg[4], loadavg[5]);
	}
	return EXIT_SUCCESS;
}
