#include <libgccjit.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

typedef float (*sqr_t)(float v);

static void usage(const char *progname)
{
	printf("%s [-h] [-v] [-Olevel]\n", progname);
	printf("-h: display this help\n");
	printf("-v: enable generated code verbose\n");
	printf("-V: enable gimple code verbose\n");
	printf("-O level: set the optimization level\n");
}

static void generate(gcc_jit_context *ctx)
{
	gcc_jit_type *float_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_FLOAT);
	gcc_jit_param *param = gcc_jit_context_new_param(ctx, NULL, float_type, "v");
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, float_type, "sqr", 1, &param, 0);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, float_type, gcc_jit_param_as_rvalue(param), gcc_jit_param_as_rvalue(param));
	gcc_jit_block_end_with_return(block, NULL, expr);
}

int main(int argc, char **argv)
{
	gcc_jit_context *ctx;
	gcc_jit_result *res;
	bool verbose_asm = false;
	bool verbose_gimple = false;
	int optim = 0;
	int c;

	while ((c = getopt(argc, argv, "hvVO:")) != -1)
	{
		switch (c)
		{
			case 'v':
				verbose_asm = true;
				break;
			case 'V':
				verbose_gimple = true;
				break;
			case 'O':
			{
				char *endptr;
				errno = 0;
				optim = strtoul(optarg, &endptr, 10);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid optimization level\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}

	ctx = gcc_jit_context_acquire();
	if (!ctx)
	{
		fprintf(stderr, "failed to acquire ctx\n");
		return EXIT_FAILURE;
	}
	if (verbose_asm)
		gcc_jit_context_set_bool_option(ctx, GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE, 1);
	if (verbose_gimple)
		gcc_jit_context_set_bool_option(ctx, GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE, 1);
	gcc_jit_context_set_int_option(ctx, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, optim);
	generate(ctx);
	res = gcc_jit_context_compile(ctx);
	gcc_jit_context_release(ctx);
	if (!res)
	{
		fprintf(stderr, "failed to compile\n");
		return EXIT_FAILURE;
	}
	sqr_t sqr = gcc_jit_result_get_code(res, "sqr");
	if (!sqr)
	{
		fprintf(stderr, "failed to get sqr ptr\n");
		return EXIT_FAILURE;
	}
	printf("%f\n", sqr(6));
	return EXIT_SUCCESS;
}
