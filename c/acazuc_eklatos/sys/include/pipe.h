#ifndef PIPE_H
#define PIPE_H

#include <pipebuf.h>
#include <poll.h>

struct file_op;
struct file;

struct pipe
{
	struct pipebuf pipebuf;
	struct waitq rwaitq;
	struct waitq wwaitq;
	struct mutex mutex;
	struct node *node;
	struct poller_head poll_entries;
};

int pipe_alloc(struct pipe **pipep, struct node *node);
int fifo_alloc(struct pipe **pipe, struct node **nodep,
               struct file **rfilep, struct file **wfilep);
void pipe_free(struct pipe *pipe);

extern const struct file_op g_pipe_fop;

#endif
