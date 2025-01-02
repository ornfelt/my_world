#ifndef SH_H
#define SH_H

#include <sys/queue.h>
#include <sys/types.h>

#include <stddef.h>

#define CMD_REDIR_IN    (1 << 0)
#define CMD_REDIR_OUT   (1 << 1)
#define CMD_REDIR_INOUT (CMD_REDIR_IN | CMD_REDIR_OUT)

TAILQ_HEAD(node_head, node);
TAILQ_HEAD(token_head, token);
TAILQ_HEAD(cmd_head, cmd);
TAILQ_HEAD(pipeline_head, pipeline);
TAILQ_HEAD(case_entry_head, case_entry);
TAILQ_HEAD(cond_entry_head, cond_entry);

enum token_type
{
	TOKEN_STR,
	TOKEN_PIPE,
	TOKEN_FDREDIR,
	TOKEN_FDCLOSE,
	TOKEN_LOGICAL_AND,
	TOKEN_LOGICAL_OR,
	TOKEN_SEMICOLON,
	TOKEN_DOUBLE_SC,
	TOKEN_AMPERSAND,
	TOKEN_LPARENTHESIS,
	TOKEN_RPARENTHESIS,
	TOKEN_NEWLINE,
};

enum parse_status
{
	PARSE_OK,
	PARSE_ERR,
	PARSE_NEED_SQUOTE,
	PARSE_NEED_DQUOTE,
	PARSE_NEED_BQUOTE,
	PARSE_NEED_NEWLINE,
	PARSE_NEED_WHILE,
	PARSE_NEED_UNTIL,
	PARSE_NEED_FOR,
	PARSE_NEED_IF,
	PARSE_NEED_CASE,
	PARSE_NEED_GROUP,
	PARSE_NEED_FN,
};

enum tokenizer_state
{
	TOKENIZER_NONE,
	TOKENIZER_ERR,
	TOKENIZER_DQUOTE,
	TOKENIZER_SQUOTE,
	TOKENIZER_BQUOTE,
	TOKENIZER_REDIR_OUT, /* should probably be merged into parser_none */
	TOKENIZER_REDIR_IN, /* should probably be merged into parser_none */
	TOKENIZER_NEWLINE,
};

enum nodifier_state
{
	NODIFIER_NONE,
	NODIFIER_ERR,
	NODIFIER_NEED_WHILE,
	NODIFIER_NEED_UNTIL,
	NODIFIER_NEED_FOR,
	NODIFIER_NEED_IF,
	NODIFIER_NEED_CASE,
	NODIFIER_NEED_GROUP,
	NODIFIER_NEED_FN,
};

enum cmd_redir_type
{
	CMD_REDIR_FD,
	CMD_REDIR_FILE,
	CMD_REDIR_CLOSE,
};

enum logical_type
{
	LOGICAL_ALWAYS,
	LOGICAL_AND,
	LOGICAL_OR,
};

enum cond_type
{
	COND_IF,
	COND_WHILE,
	COND_UNTIL,
};

enum node_type
{
	NODE_JOB,
	NODE_COND,
	NODE_CASE,
	NODE_GROUP,
	NODE_FN,
	NODE_FOR,
};

enum group_type
{
	GROUP_BRACES,
	GROUP_PARENTHESIS,
};

struct tokenizer;
struct nodifier;
struct parser;
struct sh;

struct token
{
	enum token_type type;
	union
	{
		char *str;
		struct
		{
			int fd;
			int append;
			int inout;
		} fdredir;
		struct
		{
			int fd;
		} fdclose;
		struct
		{
			int pipe2;
		} pipe;
	};
	TAILQ_ENTRY(token) chain;
};

struct cmd_redir
{
	enum cmd_redir_type type;
	int fd;
	union
	{
		int fd;
		char *file;
	} src;
	int append;
	int inout;
};

struct cmd
{
	char **args;
	size_t args_nb;
	struct cmd_redir *redir;
	size_t redir_nb;
	int async;
	pid_t pid;
	int pipefd[2];
	int pipe2;
	TAILQ_ENTRY(cmd) chain;
};

struct pipeline
{
	enum logical_type logical;
	pid_t pgid;
	int negate;
	struct cmd_head cmds;
	TAILQ_ENTRY(pipeline) chain;
};

struct node_job
{
	struct pipeline_head pipelines;
};

enum cond_state
{
	COND_COND,
	COND_START_KW,
	COND_BODY,
};

struct cond_entry
{
	struct node *cond;
	struct node_head body;
	TAILQ_ENTRY(cond_entry) chain;
};

struct node_cond
{
	enum cond_type type;
	enum cond_state state;
	struct cond_entry_head entries;
};

enum case_state
{
	CASE_VALUE,
	CASE_IN,
	CASE_ENTRY_FIRST_VALUE,
	CASE_ENTRY_FIRST_VALUE_LP,
	CASE_ENTRY_NEXT_VALUE,
	CASE_ENTRY_VALUE,
	CASE_ENTRY_BODY,
};

struct case_entry
{
	char **values;
	size_t values_count;
	struct node_head nodes;
	TAILQ_ENTRY(case_entry) chain;
};

struct node_case
{
	char *value;
	enum case_state state;
	struct case_entry_head entries;
};

struct node_group
{
	enum group_type type;
	struct node_head nodes;
};

struct node_fn
{
	char *name;
	struct node *child;
};

enum for_state
{
	FOR_VAR,
	FOR_IN,
	FOR_VALUES,
	FOR_DO,
	FOR_BODY,
};

struct node_for
{
	enum for_state state;
	char *var;
	char **values;
	size_t values_nb;
	struct node_head body;
};

struct node
{
	enum node_type type;
	struct node *parent;
	TAILQ_ENTRY(node) chain;
	union
	{
		struct node_job node_job;
		struct node_cond node_cond;
		struct node_case node_case;
		struct node_group node_group;
		struct node_fn node_fn;
		struct node_for node_for;
	};
};

struct builtin
{
	const char *name;
	int (*fn)(struct sh *sh, int argc, char **argv);
};

struct alias
{
	char *name;
	char *cmd;
	TAILQ_ENTRY(alias) chain;
};

struct function
{
	char *name;
	struct node *child;
	TAILQ_ENTRY(function) chain;
};

struct sh
{
	const char *progname;
	struct token_head tokens;
	struct node_head nodes;
	struct tokenizer *tokenizer;
	struct nodifier *nodifier;
	int last_exit_code;
	pid_t last_bg_pid;
	struct pipeline *fg_pipeline;
	int tty_input;
	int argc;
	char **argv;
	char prompt[512];
	int is_ps2;
	TAILQ_HEAD(, alias) aliases;
	TAILQ_HEAD(, function) functions;
};

int parse_init(struct sh *sh);
void parse_destroy(struct sh *sh);
enum parse_status parse(struct sh *sh, const char *line);
void parse_reset(struct sh *sh);

enum tokenizer_state tokenize(struct tokenizer *tokenizer, const char *line,
                              struct token_head *tokens);
void tokenizer_reset(struct tokenizer *tokenizer);
struct tokenizer *tokenizer_new(const char *progname);
void tokenizer_free(struct tokenizer *tokenizer);
void token_free(struct token *token);
void tokens_clean(struct token_head *tokens);
void print_token(struct token *token);
void print_tokens(struct token_head *tokens);

enum nodifier_state nodify(struct nodifier *nodifier, struct token_head *tokens,
                           struct node_head *nodes);
void nodifier_reset(struct nodifier *nodifier);
struct nodifier *nodifier_new(const char *progname);
void nodifier_free(struct nodifier *nodifier);
int node_exec(struct sh *sh, struct node *node);
void node_free(struct node *node);
void nodes_clean(struct node_head *nodes);
void print_node(struct node *node, size_t indent);
void print_nodes(struct node_head *nodes, size_t indent);

int pipeline_exec(struct sh *sh, struct pipeline *pipeline, int *exit_code);
void pipeline_free(struct pipeline *pipeline);
void cmd_free(struct cmd *cmd);

char *evalword(struct sh *sh, const char *word);
char **evalwords(struct sh *sh, const char *word, size_t *count);

extern const struct builtin g_builtins[];

#endif
