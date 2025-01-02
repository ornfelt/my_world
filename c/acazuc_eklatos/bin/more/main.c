#include <readline/readline.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
};

static int more_fp(struct env *env, FILE *fp)
{
	int ret = 1;
	size_t line_len = 0;
	char *line = NULL;
	Keymap keymap = rl_make_bare_keymap();
	if (!keymap)
	{
		fprintf(stderr, "%s: keymap allocation failed\n",
		        env->progname);
		goto end;
	}
	rl_bind_key_in_map('\n', rl_on_new_line, keymap);
	rl_set_keymap(keymap);
	rl_tty_set_echoing(0);
	while (1)
	{
		char *rl = readline("");
		if (!rl)
		{
			fprintf(stderr, "%s: input read failed\n",
			        env->progname);
			goto end;
		}
		free(rl);
		ssize_t rd = getline(&line, &line_len, fp);
		if (!rd || feof(fp))
			break;
		if (rd < 0)
		{
			fprintf(stderr, "%s: line read failed\n",
			        env->progname);
			goto end;
		}
		fputs(line, stdout);
	}
	ret = 0;

end:
	free(line);
	rl_discard_keymap(keymap);
	return ret;
}

static int more_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	int ret = more_fp(env, fp);
	fclose(fp);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-h] [file]\n", progname);
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
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	switch (argc - optind)
	{
		case 0:
			if (more_fp(&env, stdin))
				return EXIT_FAILURE;
			break;
		case 1:
			if (more_file(&env, argv[optind]))
				return EXIT_FAILURE;
			break;
		default:
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
