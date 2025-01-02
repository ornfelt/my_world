#include "sh.h"

#include <sys/wait.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

static int build_args(struct sh *sh, struct cmd *cmd, int *argc, char ***argv)
{
	*argc = 0;
	*argv = malloc(sizeof(**argv));
	if (!*argv)
	{
		fprintf(stderr, "%s: malloc: %s\n", sh->progname,
		        strerror(errno));
		return 1;
	}
	**argv = NULL;
	for (size_t i = 0; i < cmd->args_nb; ++i)
	{
		size_t exp_nb;
		char **exp = evalwords(sh, cmd->args[i], &exp_nb);
		if (!exp)
			return 1;
		char **newargv = realloc(*argv, sizeof(**argv) * (*argc + exp_nb + 1));
		if (!newargv)
		{
			fprintf(stderr, "%s: malloc: %s\n", sh->progname,
			        strerror(errno));
			for (size_t j = 0; j < exp_nb; ++j)
				free(exp[j]);
			free(exp);
			return 1;
		}
		for (size_t j = 0; j < exp_nb; ++j)
			newargv[(*argc)++] = exp[j];
		newargv[*argc] = NULL;
		free(exp);
		*argv = newargv;
	}
	return 0;
}

static void free_argv(char **argv)
{
	if (!argv)
		return;
	for (size_t i = 0; argv[i]; ++i)
		free(argv[i]);
	free(argv);
}

static void launch_child(struct sh *sh, struct cmd *cmd, pid_t pgid, char **argv)
{
	if (setpgid(0, pgid) == -1)
	{
		fprintf(stderr, "%s: setpgid: %s\n", sh->progname,
		        strerror(errno));
		return;
	}
	struct cmd *pipe_next = TAILQ_NEXT(cmd, chain);
	struct cmd *pipe_prev = TAILQ_PREV(cmd, cmd_head, chain);
	if (pipe_next)
	{
		if (dup2(cmd->pipefd[1], 1) == -1)
		{
			fprintf(stderr, "%s: dup2: %s\n", sh->progname,
			        strerror(errno));
			return;
		}
		if (cmd->pipe2 && dup2(cmd->pipefd[1], 2) == -1)
		{
			fprintf(stderr, "%s: dup2: %s\n", sh->progname,
			        strerror(errno));
			return;
		}
		close(cmd->pipefd[0]);
		close(cmd->pipefd[1]);
	}
	if (pipe_prev)
	{
		if (dup2(pipe_prev->pipefd[0], 0) == -1)
		{
			fprintf(stderr, "%s: dup2: %s\n", sh->progname,
			        strerror(errno));
			return;
		}
		close(pipe_prev->pipefd[0]);
		close(pipe_prev->pipefd[1]);
	}
	for (size_t i = 0; i < cmd->redir_nb; ++i)
	{
		const struct cmd_redir *redir = &cmd->redir[i];
		switch (redir->type)
		{
			case CMD_REDIR_CLOSE:
				if (close(redir->fd))
				{
					fprintf(stderr, "%s: close: %s\n",
					        sh->progname, strerror(errno));
					return;
				}
				break;
			case CMD_REDIR_FD:
				if (dup2(redir->src.fd, redir->fd) == -1)
				{
					fprintf(stderr, "%s: dup2: %s\n",
					        sh->progname, strerror(errno));
					return;
				}
				break;
			case CMD_REDIR_FILE:
			{
				int flags = 0;
				if (redir->inout == CMD_REDIR_IN)
					flags = O_RDONLY;
				else if (redir->inout == CMD_REDIR_OUT)
					flags = O_WRONLY | O_CREAT;
				else if (redir->inout == CMD_REDIR_INOUT)
					flags = O_RDWR | O_CREAT;
				else
					return;
				if (redir->inout == CMD_REDIR_OUT
				 || redir->inout == CMD_REDIR_INOUT)
				{
					if (redir->append)
						flags |= O_APPEND;
					else
						flags |= O_TRUNC;
				}
				int fd = open(redir->src.file, flags, 0666);
				if (fd == -1)
				{
					fprintf(stderr, "%s: open(%s): %s\n",
					        sh->progname, redir->src.file,
					        strerror(errno));
					return;
				}
				if (dup2(fd, redir->fd) == -1)
				{
					fprintf(stderr, "%s: dup2: %s\n",
					        sh->progname, strerror(errno));
					return;
				}
				break;
			}
		}
	}
	execvp(argv[0], argv);
	fprintf(stderr, "%s: %s: %s\n", sh->progname, argv[0],
	        strerror(errno));
	return;
}

static pid_t launch_cmd(struct sh *sh, struct cmd *cmd, pid_t pgid)
{
	int argc;
	char **argv;
	if (build_args(sh, cmd, &argc, &argv))
	{
		free_argv(argv);
		return -1;
	}
	if (!argv[0])
	{
		free_argv(argv);
		return -1;
	}
	struct function *fn;
	TAILQ_FOREACH(fn, &sh->functions, chain)
	{
		if (strcmp(fn->name, argv[0]))
			continue;
		int old_argc = sh->argc;
		char **old_argv = sh->argv;
		char *old_arg0 = argv[0];
		argv[0] = sh->argv[0];
		sh->argc = argc;
		sh->argv = argv;
		node_exec(sh, fn->child);
		argv[0] = old_arg0;
		sh->argc = old_argc;
		sh->argv = old_argv;
		free_argv(argv);
		return 0;
	}
	for (size_t i = 0; g_builtins[i].name; ++i)
	{
		if (strcmp(argv[0], g_builtins[i].name))
			continue;
		g_builtins[i].fn(sh, argc - 1, argv);
		free_argv(argv);
		return 0;
	}

	pid_t pid = vfork();
	if (pid < 0)
	{
		fprintf(stderr, "%s: vfork: %s\n", sh->progname,
		        strerror(errno));
		free_argv(argv);
		return -1;
	}
	if (pid)
	{
		free_argv(argv);
		return pid;
	}
	launch_child(sh, cmd, pgid, argv);
	_exit(EXIT_FAILURE);
}

static int start_cmd(struct sh *sh, struct cmd *cmd, pid_t *pgid)
{
	int ret = 1;

	struct cmd *pipe_next = TAILQ_NEXT(cmd, chain);
	struct cmd *pipe_prev = TAILQ_PREV(cmd, cmd_head, chain);
	if (pipe_next)
	{
		if (start_cmd(sh, pipe_next, pgid))
			goto end;
	}
	if (pipe_prev)
	{
		if (pipe(pipe_prev->pipefd) == -1)
		{
			fprintf(stderr, "%s: pipe: %s\n", sh->progname,
			        strerror(errno));
			goto end;
		}
	}

	cmd->pid = launch_cmd(sh, cmd, *pgid);
	if (cmd->pid)
	{
		if (cmd->pid == -1)
			goto end;
		if (!*pgid)
			*pgid = cmd->pid;
	}
	ret = 0;

end:
	if (pipe_next)
	{
		close(cmd->pipefd[0]);
		close(cmd->pipefd[1]);
	}
	return ret;
}

int pipeline_exec(struct sh *sh, struct pipeline *pipeline, int *exit_code)
{
	int ret = 1;
	pipeline->pgid = 0;
	if (start_cmd(sh, TAILQ_FIRST(&pipeline->cmds), &pipeline->pgid))
		return 1;
	sh->fg_pipeline = pipeline;
	struct cmd *cmd;
	TAILQ_FOREACH(cmd, &pipeline->cmds, chain)
	{
		if (cmd->pid <= 0)
			continue;
		int wstatus;
		while (1)
		{
			if (waitpid(cmd->pid, &wstatus, 0) != -1)
				break;
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: waitpid: %s\n", sh->progname,
			        strerror(errno));
			goto end;
		}
		if (WIFSIGNALED(wstatus))
		{
			*exit_code = -1;
			if (WTERMSIG(wstatus) != SIGINT) /* XXX only if coming from sh */
				psignal(WTERMSIG(wstatus), sh->progname);
		}
		else if (WIFEXITED(wstatus))
		{
			*exit_code = WEXITSTATUS(wstatus);
		}
		else
		{
			fprintf(stderr, "%s: unknown child exit source\n",
			        sh->progname);
			*exit_code = -1;
		}
		sh->last_exit_code = *exit_code;
	}
	ret = 0;

end:
	if (sh->tty_input)
		tcsetpgrp(0, getpgrp());
	sh->fg_pipeline = NULL;
	return ret;
}

void cmd_free(struct cmd *cmd)
{
	if (!cmd)
		return;
	for (size_t i = 0; i < cmd->args_nb; ++i)
		free(cmd->args[i]);
	free(cmd->args);
	free(cmd);
}

void pipeline_free(struct pipeline *pipeline)
{
	if (!pipeline)
		return;
	struct cmd *cmd;
	TAILQ_FOREACH(cmd, &pipeline->cmds, chain)
		cmd_free(cmd);
}
