#include "sh.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

enum nodify_err
{
	NODIFY_OK,
	NODIFY_ERR,
	NODIFY_END,
};

struct nodifier
{
	const char *progname;
	struct token_head *tokens;
	struct node_head *nodes;
	struct node *current;
};

static int parse_fd(const char *str, int *fd)
{
	if (!isdigit(str[0]))
		return 0;
	errno = 0;
	char *endptr;
	long res = strtol(str, &endptr, 10);
	if (res < 0 || res > INT_MAX || errno || *endptr)
		return 0;
	*fd = res;
	return 1;
}

static struct token *nextt(struct nodifier *nodifier)
{
	struct token *token = TAILQ_FIRST(nodifier->tokens);
	if (!token)
		return NULL;
	TAILQ_REMOVE(nodifier->tokens, token, chain);
	return token;
}

static struct token *peekt(struct nodifier *nodifier)
{
	return TAILQ_FIRST(nodifier->tokens);
}

static int token_is_keyword(struct token *token, const char *keyword)
{
	if (token->type != TOKEN_STR)
		return 0;
	if (strcmp(token->str, keyword))
		return 0;
	return 1;
}

static int is_valid_fn_name(struct token *token)
{
	const char *s = token->str;
	if (!isalpha(*s) && *s != '_')
		return 0;
	s++;
	while (*s)
	{
		if (!isalnum(*s) && *s != '_')
			return 0;
		s++;
	}
	return 1;
}

static void commit_node(struct nodifier *nodifier)
{
	if (nodifier->current->parent)
	{
		nodifier->current = nodifier->current->parent;
		if (nodifier->current->type == NODE_FN)
			commit_node(nodifier);
		return;
	}
	TAILQ_INSERT_TAIL(nodifier->nodes, nodifier->current, chain);
	nodifier->current = NULL;
}

static struct node *emit_node(struct nodifier *nodifier, enum node_type type)
{
	struct node *node = calloc(1, sizeof(*node));
	if (!node)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		return NULL;
	}
	node->type = type;
	node->parent = nodifier->current;
	return node;
}

static struct node *emit_cond(struct nodifier *nodifier, enum cond_type type)
{
	struct node *node = emit_node(nodifier, NODE_COND);
	if (!node)
		return NULL;
	node->node_cond.type = type;
	TAILQ_INIT(&node->node_cond.entries);
	return node;
}

static struct node *emit_case(struct nodifier *nodifier)
{
	struct node *node = emit_node(nodifier, NODE_CASE);
	if (!node)
		return NULL;
	TAILQ_INIT(&node->node_case.entries);
	return node;
}

static struct node *emit_for(struct nodifier *nodifier)
{
	struct node *node = emit_node(nodifier, NODE_FOR);
	if (!node)
		return NULL;
	TAILQ_INIT(&node->node_for.body);
	return node;
}

static struct cmd *mk_cmd(struct nodifier *nodifier, struct token *token)
{
	struct cmd *cmd = calloc(1, sizeof(*cmd));
	if (!cmd)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		return NULL;
	}
	cmd->args_nb = token ? 1 : 0;
	cmd->args = malloc(sizeof(*cmd->args) * 2);
	if (!cmd->args)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		free(cmd);
		return NULL;
	}
	if (token)
	{
		cmd->args[0] = strdup(token->str);
		if (!cmd->args[0])
		{
			fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
			        strerror(errno));
			free(cmd->args);
			free(cmd);
			return NULL;
		}
	}
	else
	{
		cmd->args[0] = NULL;
	}
	cmd->args[1] = NULL;
	return cmd;
}

static struct pipeline *mk_pipeline(struct nodifier *nodifier,
                                    enum logical_type logical)
{
	struct pipeline *pipeline = calloc(1, sizeof(*pipeline));
	if (!pipeline)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		return NULL;
	}
	pipeline->logical = logical;
	TAILQ_INIT(&pipeline->cmds);
	return pipeline;
}

static void case_entry_free(struct case_entry *entry)
{
	if (!entry)
		return;
	for (size_t i = 0; i < entry->values_count; ++i)
		free(entry->values[i]);
	free(entry->values);
	struct node *node;
	TAILQ_FOREACH(node, &entry->nodes, chain)
		node_free(node);
	free(entry);
}

static struct case_entry *mk_case_entry(struct nodifier *nodifier,
                                        const char *value)
{
	struct case_entry *case_entry = calloc(1, sizeof(*case_entry));
	if (!case_entry)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		return NULL;
	}
	TAILQ_INIT(&case_entry->nodes);
	case_entry->values = malloc(sizeof(*case_entry->values) * 2);
	if (!case_entry->values)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		case_entry_free(case_entry);
		return NULL;
	}
	case_entry->values[0] = strdup(value);
	if (!case_entry->values[0])
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		free(case_entry->values);
		case_entry_free(case_entry);
		return NULL;
	}
	case_entry->values[1] = NULL;
	case_entry->values_count = 1;
	return case_entry;
}

static struct node *emit_job(struct nodifier *nodifier, struct token *token)
{
	struct node *node = emit_node(nodifier, NODE_JOB);
	if (!node)
		return NULL;
	struct pipeline *pipeline = mk_pipeline(nodifier, LOGICAL_ALWAYS);
	if (!pipeline)
	{
		free(node);
		return NULL;
	}
	if (token_is_keyword(token, "!"))
	{
		pipeline->negate = 1;
		token = NULL;
	}
	struct cmd *cmd = mk_cmd(nodifier, token);
	if (!cmd)
	{
		pipeline_free(pipeline);
		free(node);
		return NULL;
	}
	TAILQ_INSERT_TAIL(&pipeline->cmds, cmd, chain);
	TAILQ_INIT(&node->node_job.pipelines);
	TAILQ_INSERT_TAIL(&node->node_job.pipelines, pipeline, chain);
	return node;
}

static struct node *emit_group(struct nodifier *nodifier, enum group_type type)
{
	struct node *node = emit_node(nodifier, NODE_GROUP);
	if (!node)
		return NULL;
	node->node_group.type = type;
	TAILQ_INIT(&node->node_group.nodes);
	return node;
}

static struct node *emit_fn(struct nodifier *nodifier, struct token *token)
{
	struct node *node = emit_node(nodifier, NODE_FN);
	if (!node)
		return NULL;
	node->node_fn.name = strdup(token->str);
	if (!node->node_fn.name)
	{
		fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
		        strerror(errno));
		free(node);
		return NULL;
	}
	return node;
}

static void print_unexpected_token(struct nodifier *nodifier,
                                   enum token_type type)
{
	const char *str;
	switch (type)
	{
		case TOKEN_STR:
			str = "";
			break;
		case TOKEN_PIPE:
			str = "|";
			break;
		case TOKEN_FDREDIR:
			str = ">";
			break;
		case TOKEN_FDCLOSE:
			str = ">";
			break;
		case TOKEN_LOGICAL_AND:
			str = "&&";
			break;
		case TOKEN_LOGICAL_OR:
			str = "||";
			break;
		case TOKEN_SEMICOLON:
			str = ";";
			break;
		case TOKEN_DOUBLE_SC:
			str = ";;";
			break;
		case TOKEN_AMPERSAND:
			str = "&";
			break;
		case TOKEN_LPARENTHESIS:
			str = "(";
			break;
		case TOKEN_RPARENTHESIS:
			str = ")";
			break;
		case TOKEN_NEWLINE:
			str = "\\n";
			break;
		default:
			str = "unknown";
			break;
	}
	fprintf(stderr, "%s: unexpected '%s'\n", nodifier->progname, str);
}

static enum nodify_err build_node(struct nodifier *nodifier,
                                  struct token *token, struct node **node)
{
	switch (token->type)
	{
		case TOKEN_SEMICOLON:
		case TOKEN_NEWLINE:
			*node = NULL;
			return NODIFY_OK;
		case TOKEN_STR:
		{
			if (token_is_keyword(token, "if"))
			{
				*node = emit_cond(nodifier, COND_IF);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			if (token_is_keyword(token , "while"))
			{
				*node = emit_cond(nodifier, COND_WHILE);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			if (token_is_keyword(token, "until"))
			{
				*node = emit_cond(nodifier, COND_UNTIL);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			if (token_is_keyword(token, "case"))
			{
				*node = emit_case(nodifier);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			if (token_is_keyword(token, "for"))
			{
				*node = emit_for(nodifier);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			if (token_is_keyword(token, "{"))
			{
				*node = emit_group(nodifier, GROUP_BRACES);
				return *node ? NODIFY_OK : NODIFY_ERR;
			}
			struct token *nxt = peekt(nodifier);
			if (nxt && nxt->type == TOKEN_LPARENTHESIS)
			{
				nextt(nodifier);
				nxt = peekt(nodifier);
				if (!nxt || nxt->type != TOKEN_RPARENTHESIS)
				{
					fprintf(stderr, "%s: ')' expected after '('\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
				nextt(nodifier);
				if (!is_valid_fn_name(token))
				{
					fprintf(stderr, "%s: invalid function name\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
				*node = emit_fn(nodifier, token);
			}
			else
			{
				*node = emit_job(nodifier, token);
			}
			return *node ? NODIFY_OK : NODIFY_ERR;
		}
		case TOKEN_LPARENTHESIS:
			*node = emit_group(nodifier, GROUP_PARENTHESIS);
			return *node ? NODIFY_OK : NODIFY_ERR;
		default:
			print_unexpected_token(nodifier, token->type);
			return NODIFY_ERR;
	}
}

static enum nodify_err handle_fdclose(struct nodifier *nodifier,
                                      struct token *token,
                                      struct cmd_redir *redir,
                                      size_t *redir_nb)
{
	(void)nodifier;
	redir->type = CMD_REDIR_CLOSE;
	if (token->fdclose.fd == -1)
	{
		if (token->fdredir.inout == CMD_REDIR_IN)
			redir->fd = 0;
		else if (token->fdredir.inout == CMD_REDIR_OUT)
			redir->fd = 1;
		else if (token->fdredir.inout == CMD_REDIR_INOUT)
			redir->fd = 0;
	}
	else
	{
		redir->fd = token->fdclose.fd;
	}
	(*redir_nb)++;
	return NODIFY_OK;
}

static enum nodify_err handle_fdredir(struct nodifier *nodifier,
                                      struct token *token,
                                      struct cmd_redir *redir,
                                      size_t *redir_nb)
{
	struct token *nxt = nextt(nodifier);
	if (!nxt)
	{
		fprintf(stderr, "%s: unexpected EOF after '>'\n",
		        nodifier->progname);
		return NODIFY_ERR;
	}
	if (nxt->type != TOKEN_STR)
	{
		fprintf(stderr, "%s: unexpected token after '>'\n",
		        nodifier->progname);
		return NODIFY_ERR;
	}
	int fd;
	if (parse_fd(nxt->str, &fd))
	{
		redir->type = CMD_REDIR_FD;
		redir->src.fd = fd;
	}
	else
	{
		redir->type = CMD_REDIR_FILE;
		redir->src.file = strdup(nxt->str);
		if (!redir->src.file)
			return NODIFY_ERR;
	}
	if (token->fdredir.fd == -1)
	{
		if (token->fdredir.inout == CMD_REDIR_IN)
			redir->fd = 0;
		else if (token->fdredir.inout == CMD_REDIR_OUT)
			redir->fd = 1;
		else if (token->fdredir.inout == CMD_REDIR_INOUT)
			redir->fd = 0;
	}
	else if (token->fdredir.fd == -2)
	{
		redir->fd = 1;
	}
	else
	{
		redir->fd = token->fdredir.fd;
	}
	redir->append = token->fdredir.append;
	redir->inout = token->fdredir.inout;
	(*redir_nb)++;
	if (token->fdredir.fd == -2)
	{
		redir++;
		redir->type = CMD_REDIR_FD;
		redir->fd = 2;
		redir->src.fd = 1;
		redir->append = token->fdredir.append;
		redir->inout = CMD_REDIR_OUT;
		(*redir_nb)++;
	}
	return NODIFY_OK;
}

static enum nodify_err process_job(struct nodifier *nodifier,
                                   struct token *token)
{
	struct pipeline *pipeline = TAILQ_LAST(&nodifier->current->node_job.pipelines,
	                                       pipeline_head);
	struct cmd *cmd = TAILQ_LAST(&pipeline->cmds, cmd_head);
	switch (token->type)
	{
		case TOKEN_STR:
		{
			if (token_is_keyword(token, "!"))
			{
				if (pipeline != TAILQ_FIRST(&nodifier->current->node_job.pipelines)
				 || pipeline->negate)
				{
					fprintf(stderr, "%s: unexpected '!'\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
				pipeline->negate = 1;
				return NODIFY_OK;
			}
			char **args = realloc(cmd->args, sizeof(*args) * (cmd->args_nb + 2));
			if (!args)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			args[cmd->args_nb] = strdup(token->str);
			if (!args[cmd->args_nb])
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				free(args);
				return NODIFY_ERR;
			}
			cmd->args_nb++;
			args[cmd->args_nb] = NULL;
			cmd->args = args;
			break;
		}
		case TOKEN_FDCLOSE:
		{
			struct cmd_redir *redir = realloc(cmd->redir,
			                                  sizeof(*redir)
			                                * (cmd->redir_nb + 2));
			if (!redir)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			cmd->redir = redir;
			return handle_fdclose(nodifier, token,
			                      &redir[cmd->redir_nb],
			                      &cmd->redir_nb);
		}
		case TOKEN_FDREDIR:
		{
			struct cmd_redir *redir = realloc(cmd->redir,
			                                  sizeof(*redir)
			                                * (cmd->redir_nb + 2));
			if (!redir)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			cmd->redir = redir;
			return handle_fdredir(nodifier, token,
			                      &redir[cmd->redir_nb],
			                      &cmd->redir_nb);
		}
		case TOKEN_AMPERSAND:
			cmd->async = 1;
			break;
		case TOKEN_DOUBLE_SC:
			if (!nodifier->current->parent
			 || nodifier->current->parent->type != NODE_CASE)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			nodifier->current->parent->node_case.state = CASE_ENTRY_FIRST_VALUE; /* XXX shame ? */
			commit_node(nodifier);
			break;
		case TOKEN_RPARENTHESIS:
			if (!nodifier->current->parent
			 || nodifier->current->parent->type != NODE_GROUP)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			commit_node(nodifier);
			commit_node(nodifier); /* XXX ptdr */
			break;
		case TOKEN_SEMICOLON:
		case TOKEN_NEWLINE:
			commit_node(nodifier);
			break;
		case TOKEN_PIPE:
		{
			struct token *nxt = nextt(nodifier);
			if (!nxt)
			{
				fprintf(stderr, "%s: unexpected EOF after '|'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			if (nxt->type != TOKEN_STR)
			{
				fprintf(stderr, "%s: unexpected token after '|'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			struct cmd *nxtcmd = mk_cmd(nodifier, nxt);
			if (!nxtcmd)
				return NODIFY_ERR;
			TAILQ_INSERT_TAIL(&pipeline->cmds, nxtcmd, chain);
			break;
		}
		case TOKEN_LOGICAL_AND:
		case TOKEN_LOGICAL_OR:
		{
			struct token *nxt = nextt(nodifier);
			if (!nxt)
			{
				fprintf(stderr, "%s: unexpected EOF after '%s'\n",
				        nodifier->progname,
				        token->type == TOKEN_LOGICAL_AND ? "&&" : "||");
				return NODIFY_ERR;
			}
			if (nxt->type != TOKEN_STR)
			{
				fprintf(stderr, "%s: unexpected token after '%s'\n",
				        nodifier->progname,
				        token->type == TOKEN_LOGICAL_AND ? "&&" : "||");
				return NODIFY_ERR;
			}
			struct pipeline *nxt_pipeline = mk_pipeline(nodifier,
			                                            token->type == TOKEN_LOGICAL_AND ? LOGICAL_AND : LOGICAL_OR);
			if (!nxt_pipeline)
				return NODIFY_ERR;
			struct cmd *nxt_cmd = mk_cmd(nodifier, nxt);
			if (!nxt_cmd)
				return NODIFY_ERR;
			TAILQ_INSERT_TAIL(&nxt_pipeline->cmds, nxt_cmd, chain);
			TAILQ_INSERT_TAIL(&nodifier->current->node_job.pipelines,
			                  nxt_pipeline, chain);
			break;
		}
		default:
			print_unexpected_token(nodifier, token->type);
			return NODIFY_ERR;
	}
	return NODIFY_OK;
}

static enum nodify_err process_cond(struct nodifier *nodifier, struct token *token)
{
	struct node_cond *current = &nodifier->current->node_cond;
	switch (current->state)
	{
		case COND_COND:
		{
			if (token->type == TOKEN_NEWLINE)
				return NODIFY_OK;
			struct cond_entry *entry = calloc(1, sizeof(*entry));
			if (!entry)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			enum nodify_err err = build_node(nodifier, token,
			                                 &entry->cond);
			if (err != NODIFY_OK)
			{
				free(entry);
				return err;
			}
			if (!entry->cond)
			{
				fprintf(stderr, "%s: unexpected EOF after '%s'\n",
				        nodifier->progname,
				        TAILQ_EMPTY(&current->entries) ? "if" : "elif");
				free(entry);
				return NODIFY_ERR;
			}
			TAILQ_INIT(&entry->body);
			TAILQ_INSERT_TAIL(&current->entries, entry, chain);
			nodifier->current = entry->cond;
			current->state = COND_START_KW;
			return NODIFY_OK;
		}
		case COND_START_KW:
		{
			if (token->type == TOKEN_SEMICOLON
			 || token->type == TOKEN_NEWLINE)
				return NODIFY_OK;
			const char *start_keyword;
			switch (current->type)
			{
				case COND_IF:
					start_keyword = "then";
					break;
				case COND_WHILE:
				case COND_UNTIL:
					start_keyword = "do";
					break;
				default:
					assert(!"unknown cond type\n");
					return NODIFY_ERR;
			}
			if (!token_is_keyword(token, start_keyword))
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			current->state = COND_BODY;
			return NODIFY_OK;
		}
		case COND_BODY:
		{
			struct cond_entry *entry = TAILQ_LAST(&current->entries, cond_entry_head);
			if (TAILQ_EMPTY(&entry->body)
			 && token->type == TOKEN_SEMICOLON)
			{
				fprintf(stderr, "%s: unexpected token ';'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			if (token->type == TOKEN_NEWLINE)
				return NODIFY_OK;
			if (current->type == COND_IF)
			{
				if (token_is_keyword(token, "else"))
				{
					if (TAILQ_EMPTY(&entry->body))
					{
						fprintf(stderr, "%s: unexpected 'else'\n",
						        nodifier->progname);
						return NODIFY_ERR;
					}
					entry = calloc(1, sizeof(*entry));
					if (!entry)
					{
						fprintf(stderr, "%s: malloc: %s\n",
						        nodifier->progname, strerror(errno));
						return NODIFY_ERR;
					}
					TAILQ_INIT(&entry->body);
					TAILQ_INSERT_TAIL(&current->entries, entry, chain);
					return NODIFY_OK;
				}
				if (token_is_keyword(token, "elif"))
				{
					if (TAILQ_EMPTY(&entry->body))
					{
						fprintf(stderr, "%s: unexpected 'elif'\n",
						        nodifier->progname);
						return NODIFY_ERR;
					}
					current->state = COND_COND;
					return NODIFY_OK;
				}
				if (token_is_keyword(token, "fi"))
				{
					if (TAILQ_EMPTY(&entry->body))
					{
						fprintf(stderr, "%s: unexpected 'fi'\n",
						        nodifier->progname);
						return NODIFY_ERR;
					}
					commit_node(nodifier);
					return NODIFY_OK;
				}
			}
			else if (token_is_keyword(token, "done"))
			{
				if (TAILQ_EMPTY(&entry->body))
				{
					fprintf(stderr, "%s: unexpected 'done'\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
				commit_node(nodifier);
				return NODIFY_OK;
			}
			struct node *node;
			enum nodify_err err = build_node(nodifier, token, &node);
			if (err != NODIFY_OK)
				return err;
			if (!node)
				return NODIFY_OK;
			TAILQ_INSERT_TAIL(&entry->body, node, chain);
			nodifier->current = node;
			return NODIFY_OK;
		}
	}
	return NODIFY_ERR;
}

static enum nodify_err process_case(struct nodifier *nodifier, struct token *token)
{
	struct node_case *current = &nodifier->current->node_case;
	switch (current->state)
	{
		case CASE_VALUE:
			if (token->type != TOKEN_STR)
			{
				fprintf(stderr, "%s: word expected near 'in'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			current->value = strdup(token->str);
			if (!current->value)
			{
				fprintf(stderr, "%s: malloc: %s\n", nodifier->progname,
				        strerror(errno));
				return NODIFY_ERR;
			}
			current->state = CASE_IN;
			return NODIFY_OK;
		case CASE_IN:
			if (!token_is_keyword(token, "in"))
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			current->state = CASE_ENTRY_FIRST_VALUE;
			return NODIFY_OK;
		case CASE_ENTRY_FIRST_VALUE:
		case CASE_ENTRY_FIRST_VALUE_LP:
		{
			if (token_is_keyword(token, "esac"))
			{
				commit_node(nodifier);
				return NODIFY_OK;
			}
			if (current->state != CASE_ENTRY_FIRST_VALUE_LP
			 && token->type == TOKEN_LPARENTHESIS)
			{
				current->state = CASE_ENTRY_FIRST_VALUE_LP;
				return NODIFY_OK;
			}
			if (token->type == TOKEN_NEWLINE)
			{
				if (current->state == CASE_ENTRY_FIRST_VALUE)
					return NODIFY_OK;
				return NODIFY_ERR;
			}
			if (token->type != TOKEN_STR)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			struct case_entry *case_entry = mk_case_entry(nodifier,
			                                              token->str);
			if (!case_entry)
				return NODIFY_ERR;
			TAILQ_INSERT_TAIL(&current->entries, case_entry, chain);
			current->state = CASE_ENTRY_VALUE;
			return NODIFY_OK;
		}
		case CASE_ENTRY_VALUE:
		{
			if (token->type == TOKEN_PIPE)
			{
				current->state = CASE_ENTRY_NEXT_VALUE;
				return NODIFY_OK;
			}
			if (token->type != TOKEN_RPARENTHESIS)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			current->state = CASE_ENTRY_BODY;
			return NODIFY_OK;
		}
		case CASE_ENTRY_NEXT_VALUE:
		{
			if (token->type != TOKEN_STR)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			struct case_entry *case_entry = TAILQ_LAST(&current->entries, case_entry_head);
			char **values = realloc(case_entry->values, (sizeof(*case_entry->values) * case_entry->values_count + 2));
			if (!values)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			values[case_entry->values_count] = strdup(token->str);
			if (!values[case_entry->values_count])
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				free(values);
				return NODIFY_ERR;
			}
			case_entry->values_count++;
			case_entry->values[case_entry->values_count] = NULL;
			case_entry->values = values;
			current->state = CASE_ENTRY_VALUE;
			return NODIFY_OK;
		}
		case CASE_ENTRY_BODY:
		{
			if (token->type == TOKEN_DOUBLE_SC)
			{
				current->state = CASE_ENTRY_FIRST_VALUE;
				return NODIFY_OK;
			}
			if (token_is_keyword(token, "esac"))
			{
				commit_node(nodifier);
				return NODIFY_OK;
			}
			struct node *node;
			enum nodify_err err = build_node(nodifier, token, &node);
			if (err != NODIFY_OK)
				return err;
			if (!node)
				return NODIFY_OK;
			struct case_entry *case_entry = TAILQ_LAST(&current->entries, case_entry_head);
			TAILQ_INSERT_TAIL(&case_entry->nodes, node, chain);
			nodifier->current = node;
			return NODIFY_OK;
		}
	}
	return NODIFY_ERR;
}

static enum nodify_err process_group(struct nodifier *nodifier,
                                     struct token *token)
{
	struct node_group *current = &nodifier->current->node_group;
	switch (current->type)
	{
		case GROUP_BRACES:
			if (token_is_keyword(token, "}"))
			{
				commit_node(nodifier);
				return NODIFY_OK;
			}
			break;
		case GROUP_PARENTHESIS:
			if (token->type == TOKEN_RPARENTHESIS)
			{
				commit_node(nodifier);
				return NODIFY_OK;
			}
			break;
	}
	struct node *node;
	enum nodify_err err = build_node(nodifier, token, &node);
	if (err != NODIFY_OK)
		return err;
	if (!node)
		return NODIFY_OK;
	TAILQ_INSERT_TAIL(&current->nodes, node, chain);
	nodifier->current = node;
	return NODIFY_OK;
}

static enum nodify_err process_fn(struct nodifier *nodifier,
                                  struct token *token)
{
	struct node_fn *current = &nodifier->current->node_fn;
	struct node *node;
	if (token->type == TOKEN_NEWLINE)
		return NODIFY_OK;
	if (!token_is_keyword(token, "{"))
	{
		fprintf(stderr, "%s: { expected after function\n",
		        nodifier->progname);
		return NODIFY_ERR;
	}
	enum nodify_err err = build_node(nodifier, token, &node);
	if (err != NODIFY_OK)
		return err;
	if (!node)
		return NODIFY_OK;
	current->child = node;
	nodifier->current = node;
	return NODIFY_OK;
}

static enum nodify_err process_for(struct nodifier *nodifier,
                                   struct token *token)
{
	struct node_for *current = &nodifier->current->node_for;
	switch (current->state)
	{
		case FOR_VAR:
			if (token->type != TOKEN_STR)
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			if (!isalpha(token->str[0]) && token->str[0] != '_')
			{
				fprintf(stderr, "%s: invalid for name\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			for (size_t i = 1; token->str[i]; ++i)
			{
				if (!isalnum(token->str[i])
				 && token->str[i] != '_')
				{
					fprintf(stderr, "%s: invalid for name\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
			}
			current->var = strdup(token->str);
			if (!current->var)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			current->state = FOR_IN;
			return NODIFY_OK;
		case FOR_IN:
			if (token->type == TOKEN_SEMICOLON
			 || token->type == TOKEN_NEWLINE)
			{
				current->state = FOR_DO;
				return NODIFY_OK;
			}
			if (!token_is_keyword(token, "in"))
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			current->state = FOR_VALUES;
			return NODIFY_OK;
		case FOR_VALUES:
		{
			if (token->type == TOKEN_SEMICOLON
			 || token->type == TOKEN_NEWLINE)
			{
				current->state = FOR_DO;
				return NODIFY_OK;
			}
			if (token->type != TOKEN_STR)
			{
				fprintf(stderr, "%s: word expected near 'in'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			char **values = realloc(current->values,
			                        sizeof(*values) * (current->values_nb + 1));
			if (!values)
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			current->values = values;
			values[current->values_nb] = strdup(token->str);
			if (!values[current->values_nb])
			{
				fprintf(stderr, "%s: malloc: %s\n",
				        nodifier->progname, strerror(errno));
				return NODIFY_ERR;
			}
			current->values_nb++;
			return NODIFY_OK;
		}
		case FOR_DO:
			if (!token_is_keyword(token, "do"))
			{
				print_unexpected_token(nodifier, token->type);
				return NODIFY_ERR;
			}
			current->state = FOR_BODY;
			return NODIFY_OK;
		case FOR_BODY:
		{
			if (TAILQ_EMPTY(&current->body)
			 && token->type == TOKEN_SEMICOLON)
			{
				fprintf(stderr, "%s: unexpected token ';'\n",
				        nodifier->progname);
				return NODIFY_ERR;
			}
			if (token->type == TOKEN_NEWLINE)
				return NODIFY_OK;
			if (token_is_keyword(token, "done"))
			{
				if (TAILQ_EMPTY(&current->body))
				{
					fprintf(stderr, "%s: unexpected 'done'\n",
					        nodifier->progname);
					return NODIFY_ERR;
				}
				commit_node(nodifier);
				return NODIFY_OK;
			}
			struct node *node;
			enum nodify_err err = build_node(nodifier, token, &node);
			if (err != NODIFY_OK)
				return err;
			if (!node)
				return NODIFY_OK;
			TAILQ_INSERT_TAIL(&current->body, node, chain);
			nodifier->current = node;
			return NODIFY_OK;
		}
	}
	return NODIFY_ERR;
}

static enum nodify_err process_token(struct nodifier *nodifier)
{
	struct token *token = nextt(nodifier);
	if (!token)
		return NODIFY_END;
	if (!nodifier->current)
		return build_node(nodifier, token, &nodifier->current);
	switch (nodifier->current->type)
	{
		case NODE_JOB:
			return process_job(nodifier, token);
		case NODE_COND:
			return process_cond(nodifier, token);
		case NODE_CASE:
			return process_case(nodifier, token);
		case NODE_GROUP:
			return process_group(nodifier, token);
		case NODE_FN:
			return process_fn(nodifier, token);
		case NODE_FOR:
			return process_for(nodifier, token);
		default:
			assert(!"unknown node type\n");
			return NODIFY_ERR;
	}
}

enum nodifier_state nodify(struct nodifier *nodifier, struct token_head *tokens,
                           struct node_head *nodes)
{
	nodifier->tokens = tokens;
	nodifier->nodes = nodes;
	while (1)
	{
		switch (process_token(nodifier))
		{
			case NODIFY_OK:
				continue;
			case NODIFY_ERR:
				return NODIFIER_ERR;
			case NODIFY_END:
				goto end;
		}
	}
end:
	print_nodes(nodifier->nodes, 0);
	if (!nodifier->current)
		return NODIFIER_NONE;
	switch (nodifier->current->type)
	{
		case NODE_JOB:
			return NODIFIER_ERR; /* XXX */
		case NODE_COND:
			switch (nodifier->current->node_cond.type)
			{
				case COND_IF:
					return NODIFIER_NEED_IF;
				case COND_WHILE:
					return NODIFIER_NEED_WHILE;
				case COND_UNTIL:
					return NODIFIER_NEED_UNTIL;
				default:
					assert(!"unknown cond type\n");
					return NODIFIER_ERR;
			}
			break;
		case NODE_CASE:
			return NODIFIER_NEED_CASE;
		case NODE_GROUP:
			return NODIFIER_NEED_GROUP;
		case NODE_FN:
			return NODIFIER_NEED_FN;
		case NODE_FOR:
			return NODIFIER_NEED_FOR;
		default:
			assert(!"unknown node type\n");
			return NODIFIER_ERR;
	}
}

void nodifier_reset(struct nodifier *nodifier)
{
	node_free(nodifier->current);
	nodifier->current = NULL;
}

struct nodifier *nodifier_new(const char *progname)
{
	struct nodifier *nodifier = calloc(1, sizeof(*nodifier));
	if (!nodifier)
		return NULL;
	nodifier->progname = progname;
	nodifier_reset(nodifier);
	return nodifier;
}

void nodifier_free(struct nodifier *nodifier)
{
	if (!nodifier)
		return;
	node_free(nodifier->current);
	free(nodifier);
}

void node_free(struct node *node)
{
	if (!node)
		return;
	switch (node->type)
	{
		case NODE_JOB:
		{
			struct pipeline *pipeline, *nxt;
			TAILQ_FOREACH_SAFE(pipeline, &node->node_job.pipelines,
			                   chain, nxt)
				pipeline_free(pipeline);
			break;
		}
		case NODE_COND:
		{
			struct cond_entry *entry, *next;
			TAILQ_FOREACH_SAFE(entry, &node->node_cond.entries, chain, next)
			{
				node_free(entry->cond);
				nodes_clean(&entry->body);
				free(entry);
			}
			break;
		}
		case NODE_CASE:
			break;
		case NODE_GROUP:
			nodes_clean(&node->node_group.nodes);
			break;
		case NODE_FN:
			free(node->node_fn.name);
			node_free(node->node_fn.child);
			break;
		case NODE_FOR:
			free(node->node_for.var);
			for (size_t i = 0; i < node->node_for.values_nb; ++i)
				free(node->node_for.values[i]);
			free(node->node_for.values);
			nodes_clean(&node->node_for.body);
			break;
		default:
			assert(!"unknown node type\n");
			break;
	}
	free(node);
}

void nodes_clean(struct node_head *nodes)
{
	while (!TAILQ_EMPTY(nodes))
	{
		struct node *node = TAILQ_FIRST(nodes);
		TAILQ_REMOVE(nodes, node, chain);
		node_free(node);
	}
}

static void print_indent(size_t indent)
{
	for (size_t i = 0; i < indent; ++i)
		printf("  ");
}

void print_node(struct node *node, size_t indent)
{
	print_indent(indent);
	switch (node->type)
	{
		case NODE_JOB:
		{
			printf("NODE_JOB\n");
			print_indent(indent);
			printf("{\n");
			struct pipeline *pipeline;
			TAILQ_FOREACH(pipeline, &node->node_job.pipelines, chain)
			{
				switch (pipeline->logical)
				{
					case LOGICAL_ALWAYS:
						break;
					case LOGICAL_AND:
						print_indent(indent + 1);
						printf("&&\n");
						break;
					case LOGICAL_OR:
						print_indent(indent + 1);
						printf("||\n");
						break;
				}
				if (pipeline->negate)
				{
					print_indent(indent + 1);
					printf("!\n");
				}
				print_indent(indent + 1);
				printf("{\n");
				struct cmd *cmd;
				TAILQ_FOREACH(cmd, &pipeline->cmds, chain)
				{
					for (size_t i = 0; i < cmd->args_nb; ++i)
					{
						print_indent(indent + 2);
						printf("%s\n", cmd->args[i]);
					}
					if (TAILQ_NEXT(cmd, chain))
					{
						print_indent(indent + 2);
						printf("|\n");
					}
				}
				print_indent(indent + 1);
				printf("}\n");
			}
			print_indent(indent);
			printf("}\n");
			break;
		}
		case NODE_COND:
		{
			printf("NODE_COND:\n");
			struct cond_entry *entry;
			TAILQ_FOREACH(entry, &node->node_cond.entries, chain)
			{
				print_indent(indent);
				printf("{\n");
				if (entry->cond)
				{
					print_indent(indent + 1);
					printf("{\n");
					print_node(entry->cond, indent + 2);
					print_indent(indent + 1);
					printf("},\n");
				}
				print_indent(indent + 1);
				printf("{\n");
				print_nodes(&entry->body, indent + 2);
				print_indent(indent + 1);
				printf("}\n");
				print_indent(indent);
				printf("}\n");
			}
			break;
		}
		case NODE_CASE:
		{
			printf("NODE_CASE\n");
			print_indent(indent);
			printf("{\n");
			print_indent(indent + 1);
			printf("%s\n", node->node_case.value);
			print_indent(indent + 1);
			printf("{\n");
			struct case_entry *entry;
			TAILQ_FOREACH(entry, &node->node_case.entries, chain)
			{
				print_indent(indent + 2);
				printf("{\n");
				print_indent(indent + 3);
				printf("{\n");
				for (size_t i = 0; i < entry->values_count; ++i)
				{
					print_indent(indent + 4);
					printf("%s\n", entry->values[i]);
				}
				print_indent(indent + 3);
				printf("},\n");
				print_indent(indent + 3);
				printf("{\n");
				print_nodes(&entry->nodes, indent + 4);
				print_indent(indent + 3);
				printf("}\n");
				print_indent(indent + 2);
				if (TAILQ_NEXT(entry, chain))
					printf("},\n");
				else
					printf("}\n");
			}
			print_indent(indent);
			printf("}\n");
			break;
		}
		case NODE_GROUP:
		{
			printf("NODE_GROUP\n");
			print_indent(indent);
			printf("{\n");
			print_nodes(&node->node_group.nodes, indent + 1);
			printf("}\n");
			break;
		}
		case NODE_FN:
		{
			printf("NODE_FN\n");
			print_indent(indent);
			printf("{\n");
			print_indent(indent + 1);
			printf("\"%s\",\n", node->node_fn.name);
			print_node(node->node_fn.child, indent + 1);
			print_indent(indent);
			printf("}\n");
			break;
		}
		case NODE_FOR:
			printf("NODE_FOR\n");
			print_indent(indent);
			printf("{\n");
			print_indent(indent + 1);
			printf("%s\n", node->node_for.var);
			print_indent(indent);
			printf("},\n");
			print_indent(indent);
			printf("{\n");
			for (size_t i = 0; i < node->node_for.values_nb; ++i)
			{
				print_indent(indent + 1);
				printf("%s\n", node->node_for.values[i]);
			}
			print_indent(indent);
			printf("},\n");
			print_indent(indent);
			printf("{\n");
			print_nodes(&node->node_for.body, indent + 1);
			print_indent(indent);
			printf("}\n");
			break;
		default:
			printf("unknown node type\n");
			break;
	}
}

void print_nodes(struct node_head *nodes, size_t indent)
{
	if (1)
		return;
	struct node *node;
	TAILQ_FOREACH(node, nodes, chain)
		print_node(node, indent);
}
