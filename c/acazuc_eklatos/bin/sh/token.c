#include "sh.h"

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

enum tokenize_err
{
	TOKENIZE_OK,
	TOKENIZE_ERR,
	TOKENIZE_END,
};

struct tokenizer
{
	const char *progname;
	enum tokenizer_state state_stack[64];
	size_t state_pos;
	struct token_head *tokens;
	const char *data;
	size_t len;
	size_t pos;
	char str_buf[1024];
	size_t str_pos;
	int redir_fd;
};

static int parse_fd(const char *str, int *fd)
{
	errno = 0;
	char *endptr;
	long res = strtol(str, &endptr, 10);
	if (res < 0 || res > INT_MAX || errno || *endptr)
		return 0;
	*fd = res;
	return 1;
}

static int nextc(struct tokenizer *tokenizer)
{
	if (tokenizer->pos >= tokenizer->len)
		return EOF;
	return (unsigned char)tokenizer->data[tokenizer->pos++];
}

static int peekc(struct tokenizer *tokenizer)
{
	if (tokenizer->pos >= tokenizer->len)
		return EOF;
	return (unsigned char)tokenizer->data[tokenizer->pos];
}

static int arg_putc(struct tokenizer *tokenizer, char c)
{
	if (tokenizer->str_pos >= sizeof(tokenizer->str_buf) - 1)
	{
		fprintf(stderr, "%s: arg too long\n", tokenizer->progname);
		return 1;
	}
	tokenizer->str_buf[tokenizer->str_pos++] = c;
	return 0;
}

static void skip_spaces(struct tokenizer *tokenizer)
{
	int c;
	while ((c = peekc(tokenizer)) != EOF)
	{
		if (!isspace(c))
			return;
		tokenizer->pos++;
	}
}

static int state_enter(struct tokenizer *tokenizer, enum tokenizer_state state)
{
	if (tokenizer->state_pos + 1 >= sizeof(tokenizer->state_stack) / sizeof(*tokenizer->state_stack))
	{
		fprintf(stderr, "%s: tokenizer state stack overflow\n",
		        tokenizer->progname);
		return 1;
	}
	tokenizer->state_stack[++tokenizer->state_pos] = state;
	return 0;
}

static int state_leave(struct tokenizer *tokenizer)
{
	if (!tokenizer->state_pos)
	{
		fprintf(stderr, "%s: tokenizer state stack underflow\n",
		        tokenizer->progname);
		return 1;
	}
	tokenizer->state_pos--;
	return 0;
}

static struct token *emit_token(struct tokenizer *tokenizer,
                                enum token_type type)
{
	struct token *token = malloc(sizeof(*token));
	if (!token)
	{
		fprintf(stderr, "%s: malloc: %s\n", tokenizer->progname,
		        strerror(errno));
		return NULL;
	}
	token->type = type;
	TAILQ_INSERT_TAIL(tokenizer->tokens, token, chain);
	return token;
}

static struct token *emit_token_str(struct tokenizer *tokenizer,
                                    const char *str, size_t len)
{
	struct token *token = emit_token(tokenizer, TOKEN_STR);
	if (!token)
		return NULL;
	token->str = strndup(str, len);
	if (!token->str)
	{
		fprintf(stderr, "%s: malloc: %s\n", tokenizer->progname,
		        strerror(errno));
		token_free(token);
		return NULL;
	}
	return token;
}

static struct token *emit_token_fdredir_out(struct tokenizer *tokenizer,
                                            int fd, int append)
{
	struct token *token = emit_token(tokenizer, TOKEN_FDREDIR);
	if (!token)
		return NULL;
	token->fdredir.inout = CMD_REDIR_OUT;
	token->fdredir.fd = fd;
	token->fdredir.append = append;
	return token;
}

static struct token *emit_token_fdredir_in(struct tokenizer *tokenizer, int fd)
{
	struct token *token = emit_token(tokenizer, TOKEN_FDREDIR);
	if (!token)
		return NULL;
	token->fdredir.inout = CMD_REDIR_IN;
	token->fdredir.fd = fd;
	return token;
}

static struct token *emit_token_fdredir_inout(struct tokenizer *tokenizer,
                                              int fd)
{
	struct token *token = emit_token(tokenizer, TOKEN_FDREDIR);
	if (!token)
		return NULL;
	token->fdredir.inout = CMD_REDIR_INOUT;
	token->fdredir.fd = fd;
	return token;
}

static struct token *emit_token_fdclose(struct tokenizer *tokenizer, int fd)
{
	struct token *token = emit_token(tokenizer, TOKEN_FDCLOSE);
	if (!token)
		return NULL;
	token->fdclose.fd = fd;
	return token;
}

static struct token *emit_token_pipe(struct tokenizer *tokenizer, int pipe2)
{
	struct token *token = emit_token(tokenizer, TOKEN_PIPE);
	if (!token)
		return NULL;
	token->pipe.pipe2 = pipe2;
	return token;
}

static void clean_str(struct tokenizer *tokenizer)
{
	tokenizer->str_pos = 0;
}

static int commit_str(struct tokenizer *tokenizer)
{
	if (!tokenizer->str_pos)
		return  0;
	struct token *token = emit_token_str(tokenizer, tokenizer->str_buf,
	                                     tokenizer->str_pos);
	tokenizer->str_pos = 0;
	return token ? 0 : 1;
}

static enum tokenize_err process_none(struct tokenizer *tokenizer, int c)
{
	switch (c)
	{
		case EOF:
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			if (!emit_token(tokenizer, TOKEN_NEWLINE))
				return TOKENIZE_ERR;
			return TOKENIZE_END;
		case '(':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			if (!emit_token(tokenizer, TOKEN_LPARENTHESIS))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case ')':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			if (!emit_token(tokenizer, TOKEN_RPARENTHESIS))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '#':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_END;
		case '\'':
			if (state_enter(tokenizer, TOKENIZER_SQUOTE))
				return TOKENIZE_ERR;
			if (arg_putc(tokenizer, c))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '"':
			if (state_enter(tokenizer, TOKENIZER_DQUOTE))
				return TOKENIZE_ERR;
			if (arg_putc(tokenizer, c))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '`':
			if (state_enter(tokenizer, TOKENIZER_BQUOTE))
				return TOKENIZE_ERR;
			if (arg_putc(tokenizer, c))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '|':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			switch (peekc(tokenizer))
			{
				case '|':
					nextc(tokenizer);
					if (!emit_token(tokenizer,
					                TOKEN_LOGICAL_OR))
						return TOKENIZE_ERR;
					break;
				case '&':
					nextc(tokenizer);
					if (!emit_token_pipe(tokenizer, 1))
						return TOKENIZE_ERR;
					break;
				default:
					if (!emit_token_pipe(tokenizer, 0))
						return TOKENIZE_ERR;
					break;
			}
			return TOKENIZE_OK;
		case ';':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			if (peekc(tokenizer) == ';')
			{
				nextc(tokenizer);
				if (!emit_token(tokenizer, TOKEN_DOUBLE_SC))
					return TOKENIZE_ERR;
			}
			else
			{
				if (!emit_token(tokenizer, TOKEN_SEMICOLON))
					return TOKENIZE_ERR;
			}
			return TOKENIZE_OK;
		case '&':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			switch (peekc(tokenizer))
			{
				case '&':
					nextc(tokenizer);
					if (!emit_token(tokenizer,
					                TOKEN_LOGICAL_AND))
						return TOKENIZE_ERR;
					break;
				case '>':
					nextc(tokenizer);
					tokenizer->redir_fd = -2;
					if (state_enter(tokenizer, TOKENIZER_REDIR_OUT))
						return TOKENIZE_ERR;
					break;
				default:
					if (!emit_token(tokenizer,
					                TOKEN_AMPERSAND))
						return TOKENIZE_ERR;
					break;
			}
			return TOKENIZE_OK;
		case '>':
		{
			int fd = -1;
			if (tokenizer->str_pos)
			{
				tokenizer->str_buf[tokenizer->str_pos] = '\0';
				if (parse_fd(tokenizer->str_buf, &fd))
				{
					if (commit_str(tokenizer))
						return TOKENIZE_ERR;
				}
				else
				{
					clean_str(tokenizer);
				}
			}
			tokenizer->redir_fd = fd;
			if (state_enter(tokenizer, TOKENIZER_REDIR_OUT))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
		case '<':
		{
			int fd = -1;
			if (tokenizer->str_pos)
			{
				tokenizer->str_buf[tokenizer->str_pos] = '\0';
				if (parse_fd(tokenizer->str_buf, &fd))
				{
					if (commit_str(tokenizer))
						return TOKENIZE_ERR;
				}
				else
				{
					clean_str(tokenizer);
				}
			}
			tokenizer->redir_fd = fd;
			if (state_enter(tokenizer, TOKENIZER_REDIR_IN))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
		case '\n':
			if (commit_str(tokenizer))
				return TOKENIZE_ERR;
			if (!emit_token(tokenizer, TOKEN_NEWLINE))
				return TOKENIZE_ERR;
			return TOKENIZE_END;
		case '\\':
		{
			int nxt = nextc(tokenizer);
			if (nxt == EOF || nxt == '\n')
			{
				if (state_enter(tokenizer, TOKENIZER_NEWLINE))
					return TOKENIZE_ERR;
				return TOKENIZE_END;
			}
			if (arg_putc(tokenizer, c)
			 || arg_putc(tokenizer, nxt))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
		case '$':
		{
			int nxt = peekc(tokenizer);
			if (nxt == '#')
			{
				nextc(tokenizer);
				if (arg_putc(tokenizer, c)
				 || arg_putc(tokenizer, nxt))
					return TOKENIZE_ERR;
			}
			if (arg_putc(tokenizer, c))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
	}
	if (isspace(c))
	{
		if (commit_str(tokenizer))
			return TOKENIZE_ERR;
		skip_spaces(tokenizer);
		return TOKENIZE_OK;
	}
	if (arg_putc(tokenizer, c))
		return TOKENIZE_ERR;
	return TOKENIZE_OK;
}

static enum tokenize_err process_squote(struct tokenizer *tokenizer, int c)
{
	if (c == EOF)
	{
		if (arg_putc(tokenizer, '\n'))
			return TOKENIZE_ERR;
		return TOKENIZE_END;
	}
	if (c == '\'')
	{
		if (state_leave(tokenizer))
			return TOKENIZE_ERR;
	}
	if (arg_putc(tokenizer, c))
		return TOKENIZE_ERR;
	return TOKENIZE_OK;
}

static enum tokenize_err process_dquote(struct tokenizer *tokenizer, int c)
{
	if (c == EOF)
	{
		if (arg_putc(tokenizer, '\n'))
			return TOKENIZE_ERR;
		return TOKENIZE_END;
	}
	if (c == '`')
	{
		if (state_enter(tokenizer, TOKENIZER_BQUOTE))
			return TOKENIZE_ERR;
	}
	else if (c == '"')
	{
		if (state_leave(tokenizer))
			return TOKENIZE_ERR;
	}
	if (arg_putc(tokenizer, c))
		return TOKENIZE_ERR;
	return TOKENIZE_OK;
}

static enum tokenize_err process_bquote(struct tokenizer *tokenizer, int c)
{
	if (c == EOF)
	{
		if (arg_putc(tokenizer, '\n'))
			return TOKENIZE_ERR;
		return TOKENIZE_END;
	}
	if (c == '`')
	{
		if (state_leave(tokenizer))
			return TOKENIZE_ERR;
	}
	if (arg_putc(tokenizer, c))
		return TOKENIZE_ERR;
	return TOKENIZE_OK;
}

static enum tokenize_err process_redir_out(struct tokenizer *tokenizer, int c)
{
	switch (c)
	{
		case EOF:
			if (!emit_token_fdredir_out(tokenizer,
			                            tokenizer->redir_fd, 0))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_END;
		case '>':
			c = peekc(tokenizer);
			if (c == '&')
			{
				fprintf(stderr, "%s: unexpected '&'\n",
				        tokenizer->progname);
				return TOKENIZE_ERR;
			}
			if (!emit_token_fdredir_out(tokenizer,
			                            tokenizer->redir_fd, 1))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '|':
			/* XXX noclobber */
			return TOKENIZE_OK;
		case '&':
		{
			if (tokenizer->redir_fd == -2)
			{
				fprintf(stderr, "%s: unexpected '&'\n",
				        tokenizer->progname);
				return TOKENIZE_ERR;
			}
			int nxt = peekc(tokenizer);
			if (nxt == '-')
			{
				nextc(tokenizer);
				if (state_leave(tokenizer))
					return TOKENIZE_ERR;
				if (!emit_token_fdclose(tokenizer,
				                        tokenizer->redir_fd))
					return TOKENIZE_ERR;
				return TOKENIZE_OK;
			}
			if (tokenizer->redir_fd == -1)
				tokenizer->redir_fd = -2;
			if (!emit_token_fdredir_out(tokenizer,
			                            tokenizer->redir_fd, 0))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
		default:
			if (!emit_token_fdredir_out(tokenizer,
			                            tokenizer->redir_fd, 0))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return process_none(tokenizer, c);
	}
}

static enum tokenize_err process_redir_in(struct tokenizer *tokenizer, int c)
{
	switch (c)
	{
		case EOF:
			if (!emit_token_fdredir_out(tokenizer,
			                            tokenizer->redir_fd, 0))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_END;
		case '>':
			if (!emit_token_fdredir_inout(tokenizer,
			                              tokenizer->redir_fd))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		case '<':
			/* XXX heredoc */
			return TOKENIZE_OK;
		case '&':
			if (!emit_token_fdredir_in(tokenizer,
			                           tokenizer->redir_fd))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return process_none(tokenizer, c);
		default:
		{
			int nxt = peekc(tokenizer);
			if (nxt == '-')
			{
				nextc(tokenizer);
				if (state_leave(tokenizer))
					return TOKENIZE_ERR;
				if (!emit_token_fdclose(tokenizer,
				                        tokenizer->redir_fd))
					return TOKENIZE_ERR;
				return TOKENIZE_OK;
			}
			if (!emit_token_fdredir_in(tokenizer,
			                           tokenizer->redir_fd))
				return TOKENIZE_ERR;
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			return TOKENIZE_OK;
		}
	}
}

static enum tokenize_err process_char(struct tokenizer *tokenizer)
{
	int c = nextc(tokenizer);
	switch (tokenizer->state_stack[tokenizer->state_pos])
	{
		case TOKENIZER_NEWLINE:
			if (state_leave(tokenizer))
				return TOKENIZE_ERR;
			/* FALLTHROUGH */
		case TOKENIZER_NONE:
			return process_none(tokenizer, c);
		case TOKENIZER_SQUOTE:
			return process_squote(tokenizer, c);
		case TOKENIZER_DQUOTE:
			return process_dquote(tokenizer, c);
		case TOKENIZER_BQUOTE:
			return process_bquote(tokenizer, c);
		case TOKENIZER_REDIR_OUT:
			return process_redir_out(tokenizer, c);
		case TOKENIZER_REDIR_IN:
			return process_redir_in(tokenizer, c);
		default:
			fprintf(stderr, "%s: unknown parser state\n",
			        tokenizer->progname);
			return TOKENIZE_ERR;
	}
}

enum tokenizer_state tokenize(struct tokenizer *tokenizer, const char *line,
                              struct token_head *tokens)
{
	tokenizer->data = line;
	tokenizer->len = strlen(line);
	tokenizer->pos = 0;
	tokenizer->tokens = tokens;
	while (1)
	{
		switch (process_char(tokenizer))
		{
			case TOKENIZE_OK:
				continue;
			case TOKENIZE_ERR:
				tokenizer->state_stack[1] = TOKENIZER_ERR;
				tokenizer->state_pos = 1;
				goto end;
			case TOKENIZE_END:
				goto end;
		}
	}
end:
	print_tokens(tokenizer->tokens);
	return tokenizer->state_stack[tokenizer->state_pos];
}

void tokenizer_reset(struct tokenizer *tokenizer)
{
	tokenizer->state_stack[0] = TOKENIZER_NONE;
	tokenizer->state_pos = 0;
	clean_str(tokenizer);
}

struct tokenizer *tokenizer_new(const char *progname)
{
	struct tokenizer *tokenizer = malloc(sizeof(*tokenizer));
	if (!tokenizer)
		return NULL;
	tokenizer->progname = progname;
	tokenizer_reset(tokenizer);
	return tokenizer;
}

void tokenizer_free(struct tokenizer *tokenizer)
{
	if (!tokenizer)
		return;
	free(tokenizer);
}

void token_free(struct token *token)
{
	switch (token->type)
	{
		case TOKEN_STR:
			free(token->str);
			break;
		default:
			break;
	}
	free(token);
}

void tokens_clean(struct token_head *tokens)
{
	while (!TAILQ_EMPTY(tokens))
	{
		struct token *token = TAILQ_FIRST(tokens);
		TAILQ_REMOVE(tokens, token, chain);
		token_free(token);
	}
}

void print_token(struct token *token)
{
	switch (token->type)
	{
		case TOKEN_STR:
			printf("TOKEN_STR: %s\n", token->str);
			break;
		case TOKEN_PIPE:
			printf("TOKEN_PIPE\n");
			break;
		case TOKEN_FDREDIR:
			printf("TOKEN_FDREDIR %d%s %s%s\n", token->fdredir.fd,
			       token->fdredir.append ? " append" : "",
			       token->fdredir.inout & CMD_REDIR_IN ? "in" : "",
			       token->fdredir.inout & CMD_REDIR_OUT ? "out" : "");
			break;
		case TOKEN_FDCLOSE:
			printf("TOKEN_FDCLOSE %d\n", token->fdclose.fd);
			break;
		case TOKEN_LOGICAL_AND:
			printf("TOKEN_LOGICAL_AND\n");
			break;
		case TOKEN_LOGICAL_OR:
			printf("TOKEN_LOGICAL_OR\n");
			break;
		case TOKEN_SEMICOLON:
			printf("TOKEN_SEMICOLON\n");
			break;
		case TOKEN_DOUBLE_SC:
			printf("TOKEN_DOUBLE_SC\n");
			break;
		case TOKEN_AMPERSAND:
			printf("TOKEN_AMPERSAND\n");
			break;
		case TOKEN_LPARENTHESIS:
			printf("TOKEN_LPARENTHESIS\n");
			break;
		case TOKEN_RPARENTHESIS:
			printf("TOKEN_RPARENTHESIS\n");
			break;
		case TOKEN_NEWLINE:
			printf("TOKEN_NEWLINE\n");
			break;
		default:
			printf("unknown token type %d\n", token->type);
			break;
	}
}

void print_tokens(struct token_head *tokens)
{
	if (1)
		return;
	struct token *token;
	TAILQ_FOREACH(token, tokens, chain)
		print_token(token);
}
