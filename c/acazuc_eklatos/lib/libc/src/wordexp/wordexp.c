#include <wordexp.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <glob.h>
#include <pwd.h>

#define WE_NOQUOTE 0
#define WE_SQUOTE  1
#define WE_DQUOTE  2

struct we_ctx
{
	wordexp_t *we;
	int flags;
	const char *word;
	const char *ifs;
	const char *it;
	char buf[4096]; /* XXX dynamic */
	size_t buf_pos;
	int word_started;
	int word_glob;
	int quote;
};

static int we_putc(struct we_ctx *ctx, char c)
{
	if (ctx->buf_pos + 1 >= sizeof(ctx->buf))
		return WRDE_NOSPACE;
	ctx->buf[ctx->buf_pos++] = c;
	ctx->word_started = 1;
	return 0;
}

static int we_puts(struct we_ctx *ctx, const char *str, size_t len)
{
	if (ctx->buf_pos + len >= sizeof(ctx->buf))
		return WRDE_NOSPACE;
	memcpy(&ctx->buf[ctx->buf_pos], str, len);
	ctx->buf_pos += len;
	ctx->word_started = 1;
	return 0;
}

static int we_putw(struct we_ctx *ctx, const char *str, size_t len)
{
	size_t off = ctx->we->we_offs + ctx->we->we_wordc;
	char **wordv = realloc(ctx->we->we_wordv,
	                       sizeof(*wordv) * (off + 2));
	if (!wordv)
		return WRDE_NOSPACE;
	ctx->we->we_wordv = wordv;
	wordv[off] = malloc(len + 1);
	if (!wordv[off])
		return WRDE_NOSPACE;
	memcpy(wordv[off], str, len + 1);
	wordv[off + 1] = NULL;
	ctx->we->we_wordc++;
	return 0;
}

static int we_endw(struct we_ctx *ctx)
{
	ctx->buf[ctx->buf_pos] = '\0';
	if (ctx->word_glob)
	{
		glob_t gl;
		int ret = glob(ctx->buf, GLOB_NOCHECK, NULL, &gl);
		if (ret)
		{
			globfree(&gl);
			return WRDE_NOSPACE;
		}
		for (size_t i = 0; i < gl.gl_pathc; ++i)
		{
			ret = we_putw(ctx, gl.gl_pathv[i], strlen(gl.gl_pathv[i]));
			if (ret)
			{
				globfree(&gl);
				return ret;
			}
		}
		globfree(&gl);
	}
	else
	{
		/* we can't have unquoted glob char here */
		for (size_t i = 0, j = 0; i <= ctx->buf_pos; ++i, ++j)
		{
			if (ctx->buf[i] == '\\'
			 && (ctx->buf[i + 1] == '['
			  || ctx->buf[i + 1] == '?'
			  || ctx->buf[i + 1] == '*'))
				i++;
			ctx->buf[j] = ctx->buf[i];
		}
		int ret = we_putw(ctx, ctx->buf, ctx->buf_pos);
		if (ret)
			return ret;
	}
	ctx->word_started = 0;
	ctx->word_glob = 0;
	ctx->buf_pos = 0;
	return 0;
}

static int we_putexp_quote(struct we_ctx *ctx, const char *str, size_t len,
                           int quote)
{
	switch (quote)
	{
		case WE_DQUOTE:
		case WE_SQUOTE:
			return we_puts(ctx, str, len);
		case WE_NOQUOTE:
		{
			int ret;
			for (size_t i = 0; i < len; ++i)
			{
				if (strchr(ctx->ifs, str[i]))
				{
					while (strchr(ctx->ifs, str[i + 1]))
						i++;
					ret = we_endw(ctx);
					if (ret)
						return ret;
					continue;
				}
				switch (str[i])
				{
					case '[':
					case '?':
					case '*':
						ctx->word_glob = 1;
						break;
				}
				ret = we_putc(ctx, str[i]);
				if (ret)
					return ret;
			}
			return 0;
		}
		default:
			return WRDE_SYNTAX;
	}
}

static int we_putexp(struct we_ctx *ctx, const char *str, size_t len)
{
	return we_putexp_quote(ctx, str, len, ctx->quote);
}

static int we_np_get_var(struct we_ctx *ctx, const char *name, size_t len)
{
	char **values;
	int ret;

	values = ctx->we->we_np_get_var(ctx->we, name, len);
	if (!values)
	{
		if (ctx->flags & WRDE_UNDEF)
			return WRDE_BADVAL;
		return 0;
	}
	if (len == 1 && name[0] == '@'
	 && ctx->quote == WE_DQUOTE)
	{
		for (size_t i = 0; values[i]; ++i)
		{
			ret = we_puts(ctx, values[i], strlen(values[i]));
			if (ret)
				goto end;
			if (values[i + 1])
				we_endw(ctx);
		}
		ctx->quote = WE_DQUOTE;
	}
	else
	{
		for (size_t i = 0; values[i]; ++i)
		{
			ret = we_putexp(ctx, values[i], strlen(values[i]));
			if (ret)
				goto end;
			if (values[i + 1])
			{
				if (ctx->quote == WE_NOQUOTE)
					we_endw(ctx);
				else
					we_putc(ctx, ctx->ifs[0]);
			}
		}
	}
end:
	for (size_t i = 0; values[i]; ++i)
		free(values[i]);
	free(values);
	return ret;
}

static int we_var(struct we_ctx *ctx)
{
	if (isdigit(ctx->it[1]))
	{
		if (!(ctx->flags & WRDE_NP_GET_VAR))
			return 0;
		ctx->it++;
		return we_np_get_var(ctx, ctx->it, 1);
	}
	if (!isalpha(ctx->it[1]))
	{
		if (!(ctx->flags & WRDE_NP_GET_VAR))
			return we_putc(ctx, '$');
		ctx->it++;
		return we_np_get_var(ctx, ctx->it, 1);
	}
	const char *name = ctx->it + 1;
	while (isalnum(ctx->it[1]) || ctx->it[1] == '_')
		ctx->it++;
	if (ctx->flags & WRDE_NP_GET_VAR)
		return we_np_get_var(ctx, name, ctx->it - name + 1);
	char *tmp = strndup(name, ctx->it - name + 1);
	if (!tmp)
		return WRDE_NOSPACE;
	const char *value = getenv(tmp);
	free(tmp);
	if (!value)
	{
		if (ctx->flags & WRDE_UNDEF)
			return WRDE_BADVAL;
		return 0;
	}
	return we_putexp(ctx, value, strlen(value));
}

static int we_home(struct we_ctx *ctx)
{
	char user[4096];
	size_t user_len = 0;
	const char *it = ctx->it;
	while (1)
	{
		if (!it[1])
			break;
		if (it[1] == '/' || isspace(it[1]))
			break;
		if (user_len + 1 >= sizeof(user))
			return WRDE_SYNTAX;
		user[user_len++] = it[1];
		it++;
	}
	if (!user_len)
	{
		const char *value = getenv("HOME");
		if (value)
			return we_puts(ctx, value, strlen(value));
		return 0;
	}
	if (user_len == 1 && user[0] == '+')
	{
		ctx->it++;
		const char *value = getenv("PWD");
		if (value)
			return we_puts(ctx, value, strlen(value));
		return 0;
	}
	if (user_len == 1 && user[0] == '-')
	{
		ctx->it++;
		const char *value = getenv("OLDPWD");
		if (value)
			return we_puts(ctx, value, strlen(value));
		return 0;
	}
	user[user_len] = '\0';
	struct passwd *pwd = getpwnam(user);
	if (!pwd)
		return we_putc(ctx, '~');
	ctx->it = it;
	return we_puts(ctx, pwd->pw_dir, strlen(pwd->pw_dir));
}

static int we_np_cmd_exp(struct we_ctx *ctx)
{
	const char *cmd = ++ctx->it;
	while (*ctx->it != '`')
	{
		if (!*ctx->it)
			return WRDE_SYNTAX;
		ctx->it++;
	}
	if (!(ctx->flags & WRDE_NP_CMD_EXP))
	{
		/* XXX call sh ? */
		return we_puts(ctx, cmd, ctx->it - cmd);
	}
	char *value = ctx->we->we_np_cmd_exp(ctx->we, cmd, ctx->it - cmd);
	if (!value)
	{
		if (ctx->flags & WRDE_UNDEF)
			return WRDE_BADVAL;
		return 0;
	}
	int ret = we_putexp(ctx, value, strlen(value));
	free(value);
	return ret;
}

static int we_noquote(struct we_ctx *ctx)
{
	if (!(ctx->flags & WRDE_NP_NOBK) && strchr(ctx->ifs, *ctx->it))
	{
		while (strchr(ctx->ifs, ctx->it[1]))
			ctx->it++;
		if (ctx->buf_pos)
			return we_endw(ctx);
		return 0;
	}
	switch (*ctx->it)
	{
		case '|':
		case '&':
		case ';':
		case '<':
		case '>':
		case '(':
		case ')':
		case '{':
		case '}':
		case '\n':
			return WRDE_BADCHAR;
		case '\'':
			ctx->word_started = 1;
			ctx->quote = WE_SQUOTE;
			return 0;
		case '"':
			ctx->word_started = 1;
			ctx->quote = WE_DQUOTE;
			return 0;
		case '\\':
			ctx->it++;
			if (!*ctx->it)
				return WRDE_SYNTAX;
			return we_putc(ctx, *ctx->it);
		case '~':
			if (ctx->it == ctx->word)
				return we_home(ctx);
			return we_putc(ctx, *ctx->it);
		case '$':
			return we_var(ctx);
		case '[':
		case '?':
		case '*':
			ctx->word_glob = 1;
			return we_putc(ctx, *ctx->it);
		case '`':
			return we_np_cmd_exp(ctx);
		default:
			return we_putc(ctx, *ctx->it);
	}
}

static int we_dquote(struct we_ctx *ctx)
{
	switch (*ctx->it)
	{
		case '"':
			ctx->quote = WE_NOQUOTE;
			return 0;
		case '\\':
			ctx->it++;
			switch (*ctx->it)
			{
				case '\0':
					return WRDE_SYNTAX;
				case '\n':
				case '$':
				case '`':
				case '"':
				case '/':
					return we_putc(ctx, *ctx->it);
				default:
				{
					int ret = we_putc(ctx, '\\');
					if (ret)
						return ret;
					return we_putc(ctx, *ctx->it);
				}
			}
			break;
		case '$':
			return we_var(ctx);
		case '[':
		case '?':
		case '*':
		{
			int ret = we_putc(ctx, '\\');
			if (ret)
				return ret;
			return we_putc(ctx, *ctx->it);
		}
		default:
			return we_putc(ctx, *ctx->it);
	}
}

static int we_squote(struct we_ctx *ctx)
{
	switch (*ctx->it)
	{
		case '\'':
			ctx->quote = WE_NOQUOTE;
			return 0;
		case '[':
		case '?':
		case '*':
		{
			int ret = we_putc(ctx, '\\');
			if (ret)
				return ret;
			return we_putc(ctx, *ctx->it);
		}
		default:
			return we_putc(ctx, *ctx->it);
	}
}

int wordexp(const char *word, wordexp_t *we, int flags)
{
	if (!(flags & WRDE_APPEND))
	{
		we->we_wordc = 0;
		size_t n = 1;
		if (flags & WRDE_DOOFFS)
			n += we->we_offs;
		else
			we->we_offs = 0;
		we->we_wordv = calloc(n, sizeof(*we->we_wordv));
		if (!we->we_wordv)
			return WRDE_NOSPACE;
	}
	struct we_ctx ctx;
	ctx.we = we;
	ctx.flags = flags;
	ctx.word = word;
	ctx.ifs = getenv("IFS");
	if (!ctx.ifs)
		ctx.ifs = " \t\n";
	ctx.it = word;
	ctx.quote = WE_NOQUOTE;
	ctx.buf_pos = 0;
	ctx.word_glob = 0;
	ctx.word_started = 0;
	int ret = 0;
	while (!ret && *ctx.it)
	{
		switch (ctx.quote)
		{
			case WE_NOQUOTE:
				ret = we_noquote(&ctx);
				break;
			case WE_SQUOTE:
				ret = we_squote(&ctx);
				break;
			case WE_DQUOTE:
				ret = we_dquote(&ctx);
				break;
		}
		ctx.it++;
	}
	if (ret)
		return ret;
	if (ctx.quote != WE_NOQUOTE)
		return WRDE_SYNTAX;
	if (ctx.word_started)
		ret = we_endw(&ctx);
	return ret;
}
