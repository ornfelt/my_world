#include "sh.h"

#include <inttypes.h>
#include <wordexp.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

static int exec_nodes(struct sh *sh, struct node_head *nodes)
{
	struct node *node;
	TAILQ_FOREACH(node, nodes, chain)
	{
		if (node_exec(sh, node))
			return 1;
	}
	return 0;
}

static int exec_job(struct sh *sh, struct node *node, int *exit_code)
{
	struct pipeline *pipeline;
	TAILQ_FOREACH(pipeline, &node->node_job.pipelines, chain)
	{
		switch (pipeline->logical)
		{
			case LOGICAL_ALWAYS:
				break;
			case LOGICAL_AND:
				if (*exit_code)
					continue;
				break;
			case LOGICAL_OR:
				if (!*exit_code)
					continue;
				break;
			default:
				fprintf(stderr, "%s: unknown logical type\n",
				        sh->progname);
				return 1;
		}
		*exit_code = 0;
		if (pipeline_exec(sh, pipeline, exit_code))
			return 1;
		if (pipeline->negate)
			*exit_code = !*exit_code;
	}
	return 0;
}

static int exec_cond(struct sh *sh, struct node *node)
{
	int cond_exit_code = 0;
	if (node->node_cond.type == COND_IF)
	{
		struct cond_entry *entry;
		TAILQ_FOREACH(entry, &node->node_cond.entries, chain)
		{
			if (entry->cond)
			{
				if (exec_job(sh, entry->cond, &cond_exit_code))
					return 1;
				if (cond_exit_code)
					continue;
			}
			return exec_nodes(sh, &entry->body);
		}
		return 0;
	}
	struct cond_entry *entry = TAILQ_FIRST(&node->node_cond.entries);
	while (1)
	{
		if (exec_job(sh, entry->cond, &cond_exit_code))
			return 1;
		switch (node->node_cond.type)
		{
			case COND_WHILE:
				if (cond_exit_code)
					return 0;
				break;
			case COND_UNTIL:
				if (!cond_exit_code)
					return 0;
				break;
			default:
				fprintf(stderr, "%s: unknown condition type\n",
				        sh->progname);
				return 1;
		}
		if (exec_nodes(sh, &entry->body))
			return 1;
	}
	return 0;
}

static int exec_case(struct sh *sh, struct node *node)
{
	char *value = evalword(sh, node->node_case.value);
	if (!value)
		return 1;
	struct case_entry *entry;
	TAILQ_FOREACH(entry, &node->node_case.entries, chain)
	{
		for (size_t i = 0; i < entry->values_count; ++i)
		{
			char *str = evalword(sh, entry->values[i]);
			if (!str)
			{
				free(value);
				return 1;
			}
			switch (fnmatch(str, value, 0))
			{
				case 0:
					free(value);
					free(str);
					return exec_nodes(sh, &entry->nodes);
				case FNM_NOMATCH:
					free(str);
					break;
				default:
					free(str);
					return 1;
			}
		}
	}
	free(value);
	return 0;
}

static int exec_for(struct sh *sh, struct node *node)
{
	if (!node->node_for.values_nb)
	{
		for (int i = 1; i < sh->argc; ++i)
		{
			setenv(node->node_for.var, sh->argv[i], 1); /* XXX should be just a var, not an env */
			if (exec_nodes(sh, &node->node_for.body))
				return 1;
		}
		return 0;
	}
	for (size_t i = 0; i < node->node_for.values_nb; ++i)
	{
		size_t exp_nb;
		char **exp = evalwords(sh, node->node_for.values[i], &exp_nb);
		if (!exp)
			return 1;
		if (!exp_nb)
			continue;
		int ret;
		for (size_t j = 0; j < exp_nb; ++j)
		{
			setenv(node->node_for.var, exp[j], 1); /* XXX should be just a var, not an env */
			ret = exec_nodes(sh, &node->node_for.body);
			if (ret)
				break;
		}
		for (size_t j = 0; j < exp_nb; ++j)
			free(exp[j]);
		free(exp);
		if (ret)
			return ret;
	}
	return 0;
}

static int exec_group(struct sh *sh, struct node *node)
{
	struct node *child;
	TAILQ_FOREACH(child, &node->node_group.nodes, chain)
	{
		if (node_exec(sh, child) != PARSE_OK)
			return 1;
	}
	return 0;
}

static int exec_fn(struct sh *sh, struct node *node)
{
	struct function *fn;
	TAILQ_FOREACH(fn, &sh->functions, chain)
	{
		if (strcmp(fn->name, node->node_fn.name))
			continue;
		node_free(fn->child);
		fn->child = node->node_fn.child;
		node->node_fn.child = NULL;
		return 0;
	}
	fn = malloc(sizeof(*fn));
	if (!fn)
	{
		fprintf(stderr, "%s: malloc: %s\n", sh->progname,
		        strerror(errno));
		return 1;
	}
	fn->name = node->node_fn.name;
	fn->child = node->node_fn.child;
	node->node_fn.name = NULL;
	node->node_fn.child = NULL;
	TAILQ_INSERT_TAIL(&sh->functions, fn, chain);
	return 0;
}

int node_exec(struct sh *sh, struct node *node)
{
	switch (node->type)
	{
		case NODE_JOB:
		{
			int exit_code = 0;
			if (exec_job(sh, node, &exit_code))
				return PARSE_ERR;
			break;
		}
		case NODE_COND:
			if (exec_cond(sh, node))
				return PARSE_ERR;
			break;
		case NODE_CASE:
			if (exec_case(sh, node))
				return PARSE_ERR;
			break;
		case NODE_FOR:
			if (exec_for(sh, node))
				return PARSE_ERR;
			break;
		case NODE_GROUP:
			if (exec_group(sh, node))
				return PARSE_ERR;
			break;
		case NODE_FN:
			if (exec_fn(sh, node))
				return PARSE_ERR;
			break;
		default:
			break;
	}
	return PARSE_OK;
}

enum parse_status parse(struct sh *sh, const char *line)
{
	switch (tokenize(sh->tokenizer, line, &sh->tokens))
	{
		case TOKENIZER_NONE:
			break;
		case TOKENIZER_ERR:
			return PARSE_ERR;
		case TOKENIZER_SQUOTE:
			return PARSE_NEED_SQUOTE;
		case TOKENIZER_DQUOTE:
			return PARSE_NEED_DQUOTE;
		case TOKENIZER_BQUOTE:
			return PARSE_NEED_BQUOTE;
		case TOKENIZER_NEWLINE:
			return PARSE_NEED_NEWLINE;
		default:
			fprintf(stderr, "%s: unexpected EOF\n",
			        sh->progname);
			return PARSE_ERR;
	}
	switch (nodify(sh->nodifier, &sh->tokens, &sh->nodes))
	{
		case NODIFIER_NONE:
			break;
		case NODIFIER_ERR:
			return PARSE_ERR;
		case NODIFIER_NEED_WHILE:
			return PARSE_NEED_WHILE;
		case NODIFIER_NEED_UNTIL:
			return PARSE_NEED_UNTIL;
		case NODIFIER_NEED_FOR:
			return PARSE_NEED_FOR;
		case NODIFIER_NEED_IF:
			return PARSE_NEED_IF;
		case NODIFIER_NEED_CASE:
			return PARSE_NEED_CASE;
		case NODIFIER_NEED_GROUP:
			return PARSE_NEED_GROUP;
		case NODIFIER_NEED_FN:
			return PARSE_NEED_FN;
		default:
			fprintf(stderr, "%s: unexpected EOF\n",
			        sh->progname);
			return PARSE_ERR;
	}
	while (!TAILQ_EMPTY(&sh->nodes))
	{
		struct node *node = TAILQ_FIRST(&sh->nodes);
		if (node_exec(sh, node))
			return PARSE_ERR;
		TAILQ_REMOVE(&sh->nodes, node, chain);
	}
	return PARSE_OK;
}

void parse_reset(struct sh *sh)
{
	tokens_clean(&sh->tokens);
	nodes_clean(&sh->nodes);
	tokenizer_reset(sh->tokenizer);
}

int parse_init(struct sh *sh)
{
	sh->tokenizer = tokenizer_new(sh->progname);
	if (!sh->tokenizer)
		return 1;
	sh->nodifier = nodifier_new(sh->progname);
	if (!sh->nodifier)
	{
		tokenizer_free(sh->tokenizer);
		return 1;
	}
	TAILQ_INIT(&sh->tokens);
	TAILQ_INIT(&sh->nodes);
	return 0;
}

void parse_destroy(struct sh *sh)
{
	tokens_clean(&sh->tokens);
	nodes_clean(&sh->nodes);
	tokenizer_free(sh->tokenizer);
	nodifier_free(sh->nodifier);
}

static char **get_single_var(const char *str)
{
	char **ret = malloc(sizeof(*ret) * 2);
	if (!ret)
		return NULL;
	ret[0] = strdup(str);
	if (!ret[0])
	{
		free(ret);
		return NULL;
	}
	ret[1] = NULL;
	return ret;
}

static char **get_var(wordexp_t *we, const char *name, size_t len)
{
	static char buf[4096];
	struct sh *sh = we->we_ptr;
	if (len == 1)
	{
		switch (name[0])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				int n = name[0] - '0';
				if (n >= sh->argc)
					return NULL;
				return get_single_var(sh->argv[n]);
			}
			case '#':
			{
				snprintf(buf, sizeof(buf), "%d",
				         sh->argc ? sh->argc - 1 : 0);
				return get_single_var(buf);
			}
			case '@':
			case '*':
			{
				if (sh->argc < 2)
					return NULL;
				char **ret = malloc(sizeof(*ret) * sh->argc);
				if (!ret)
					return ret;
				for (int i = 1; i < sh->argc; ++i)
				{
					ret[i - 1] = strdup(sh->argv[i]);
					if (!ret[i - 1])
					{
						for (int j = 1; j < i; ++i)
							free(ret[j - 1]);
						free(ret);
						return NULL;
					}
				}
				ret[sh->argc - 1] = NULL;
				return ret;
			}
			case '?':
			{
				snprintf(buf, sizeof(buf), "%d",
				         sh->last_exit_code);
				return get_single_var(buf);
			}
			case '$':
			{
				snprintf(buf, sizeof(buf), "%" PRId32, getpid());
				return get_single_var(buf);
			}
			case '!':
			{
				if (sh->last_bg_pid != -1)
					snprintf(buf, sizeof(buf), "%" PRId32,
					         sh->last_bg_pid);
				else
					buf[0] = '\0';
				return get_single_var(buf);
			}
			default:
				break;
		}
	}
	char *value = getenv(name);
	if (value)
		return get_single_var(value);
	return NULL;
}

static char *cmd_exp(wordexp_t *we, const char *cmd, size_t len)
{
	/* XXX */
	(void)we;
	(void)len;
	return strdup(cmd);
}

char *evalword(struct sh *sh, const char *word)
{
	wordexp_t we;
	we.we_ptr = sh;
	we.we_np_get_var = get_var;
	we.we_np_cmd_exp = cmd_exp;
	int ret = wordexp(word, &we, WRDE_NP_NOBK | WRDE_NP_GET_VAR | WRDE_NP_CMD_EXP);
	if (ret)
	{
		fprintf(stderr, "%s: wordexp: %d\n", sh->progname,
		        ret);
		return NULL;
	}
	if (we.we_wordc > 1)
	{
		fprintf(stderr, "%s: wordexp returned more than 1 word\n",
		        sh->progname);
		wordfree(&we);
		return NULL;
	}
	if (we.we_wordc == 0)
	{
		wordfree(&we);
		char *s = strdup("");
		if (!s)
		{
			fprintf(stderr, "%s: malloc: %s\n", sh->progname,
			        strerror(errno));
			return NULL;
		}
		return s;
	}
	char *s = strdup(we.we_wordv[0]);
	wordfree(&we);
	if (!s)
	{
		fprintf(stderr, "%s: malloc: %s\n", sh->progname,
		        strerror(errno));
		return NULL;
	}
	return s;
}

char **evalwords(struct sh *sh, const char *word, size_t *count)
{
	wordexp_t we;
	we.we_ptr = sh;
	we.we_np_get_var = get_var;
	int ret = wordexp(word, &we, WRDE_NP_GET_VAR);
	if (ret)
	{
		fprintf(stderr, "%s: wordexp: %d\n", sh->progname,
		        ret);
		return NULL;
	}
	char **values = we.we_wordv;
	if (count)
		*count = we.we_wordc;
	we.we_wordv = NULL;
	we.we_wordc = 0;
	wordfree(&we);
	return values;
}
