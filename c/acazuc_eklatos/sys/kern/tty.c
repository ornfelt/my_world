#include <signal.h>
#include <errno.h>
#include <ioctl.h>
#include <utf8.h>
#include <file.h>
#include <pipe.h>
#include <proc.h>
#include <tty.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <sma.h>
#include <cpu.h>
#include <mem.h>

struct tty *curtty;

static struct sma tty_sma;

void tty_init(void)
{
	sma_init(&tty_sma, sizeof(struct tty), NULL, NULL, "tty");
}

static struct tty *gettty(struct file *file)
{
	if (file->cdev)
		return file->cdev->userdata;
	if (file->node && S_ISCHR(file->node->attr.mode))
		return file->node->cdev->userdata;
	return NULL;
}

static int tty_fopen(struct file *file, struct node *node)
{
	struct tty *tty = gettty(file);
	if (!tty)
		return -EINVAL;
	(void)file;
	(void)node;
	return 0;
}

static ssize_t tty_fwrite(struct file *file, struct uio *uio)
{
	struct tty *tty = gettty(file);
	if (!tty)
		return -EINVAL;
	return tty_write(tty, uio);
}

static ssize_t tty_fread(struct file *file, struct uio *uio)
{
	struct tty *tty = gettty(file);
	if (!tty)
		return -EINVAL;
	return pipebuf_read(&tty->pipebuf, uio,
	                    file->flags & O_NONBLOCK ? 0 : 1, NULL);
}

static int tty_fioctl(struct file *file, unsigned long req, uintptr_t data)
{
	struct thread *thread = curcpu()->thread;
	struct tty *tty = gettty(file);
	int ret;

	if (!tty)
		return -EINVAL;
	if (tty->op->ioctl)
	{
		ret = tty->op->ioctl(tty, req, data);
		if (ret != -ENOSYS)
			return ret;
	}
	switch (req)
	{
		case TCGETS:
			tty_lock(tty);
			ret = vm_copyout(thread->proc->vm_space,
			                 (struct termios*)data, &tty->termios,
			                 sizeof(struct termios));
			tty_unlock(tty);
			return ret;
		case TCSETS:
			tty_lock(tty);
			ret = vm_copyin(thread->proc->vm_space, &tty->termios,
			                (struct termios*)data,
			                sizeof(struct termios));
			tty_unlock(tty);
			return ret;
		case TCSETSW:
			/* XXX */
			return -ENOSYS;
		case TCSETSF:
			/* XXX */
			return -ENOSYS;
		case TCSBRK:
			/* XXX */
			return -ENOSYS;
		case TCFLSH:
			/* XXX */
			return -ENOSYS;
		case TCXONC:
			/* XXX */
			return -ENOSYS;
		case TIOCGWINSZ:
			tty_lock(tty);
			ret = vm_copyout(thread->proc->vm_space,
			                 (struct winsize*)data,
			                 &tty->winsize,
			                 sizeof(struct winsize));
			tty_unlock(tty);
			return ret;
		case TIOCSWINSZ:
			/* XXX SIGWINCH */
			tty_lock(tty);
			ret = vm_copyin(thread->proc->vm_space,
			                &tty->winsize,
			                (struct winsize*)data,
			                sizeof(struct winsize));
			tty_unlock(tty);
			return ret;
		case TIOCGPGRP:
			return vm_copyout(thread->proc->vm_space, (int*)data,
			                  &tty->pgid, sizeof(int));
		case TIOCSPGRP:
		{
			struct pgrp *pgrp = getpgrp(data);
			if (!pgrp)
				return -EINVAL;
			if (pgrp->sess != thread->proc->pgrp->sess)
			{
				pgrp_free(pgrp);
				return -EPERM;
			}
			tty->pgid = pgrp->id;
			pgrp_free(pgrp);
			return 0;
		}
		case TIOCGSID:
		{
			struct pgrp *pgrp = getpgrp(data);
			if (!pgrp)
				return -EINVAL;
			ret = pgrp->sess->id;
			pgrp_free(pgrp);
			return ret;
		}
	}
	return -EINVAL;
}

static int tty_poll(struct file *file, struct poll_entry *entry)
{
	struct tty *tty = gettty(file);
	int ret;

	if (!tty)
		return -EINVAL;
	ret = 0;
	if (entry->events & POLLOUT)
		ret |= POLLOUT;
	tty_lock(tty);
	if (entry->events & POLLIN)
		ret |= pipebuf_poll_locked(&tty->pipebuf, entry->events & ~POLLOUT);
	if (ret)
		goto end;
	entry->file_head = &tty->poll_entries;
	ret = poller_add(entry);

end:
	tty_unlock(tty);
	return ret;
}

static const struct file_op g_tty_fop =
{
	.open = tty_fopen,
	.write = tty_fwrite,
	.read = tty_fread,
	.ioctl = tty_fioctl,
	.poll = tty_poll,
};

int tty_alloc(const char *name, dev_t rdev, const struct tty_op *op,
              struct tty **ttyp)
{
	struct tty *tty = sma_alloc(&tty_sma, M_ZERO);
	if (!tty)
		return -ENOMEM;
	tty->pgid = -1;
	mutex_init(&tty->mutex, MUTEX_RECURSIVE);
	waitq_init(&tty->rwaitq);
	waitq_init(&tty->wwaitq);
	int ret = pipebuf_init(&tty->pipebuf, PAGE_SIZE * 2, &tty->mutex,
	                       &tty->rwaitq, &tty->wwaitq);
	if (ret)
	{
		mutex_destroy(&tty->mutex);
		waitq_destroy(&tty->rwaitq);
		waitq_destroy(&tty->wwaitq);
		sma_free(&tty_sma, tty);
		return ret;
	}
	tty->pipebuf.nwriters = 1;
	tty->pipebuf.nreaders = 1;
	TAILQ_INIT(&tty->poll_entries);
	tty->op = op;
	tty->ctrl_state = 0;
	tty->termios.c_iflag = IXON;
	tty->termios.c_oflag = OPOST;
	tty->termios.c_lflag = ISIG | ICANON | ECHO;
	tty->termios.c_cc[VEOF] = 4; /* EOT, ctrl+D */
	tty->termios.c_cc[VEOL] = 0; /* NUL */
	tty->termios.c_cc[VERASE] = 8; /* \b, ctrl+H */
	tty->termios.c_cc[VINTR] = 3; /* ETX, ctrl+C */
	tty->termios.c_cc[VKILL] = 21; /* NAK, ctrl+U */
	tty->termios.c_cc[VMIN] = 0;
	tty->termios.c_cc[VQUIT] = 28; /* FS, ctrl+\ */
	tty->termios.c_cc[VSTART] = 17; /* DC1, ctrl+Q */
	tty->termios.c_cc[VSTOP] = 19; /* DC3, ctrl+S */
	tty->termios.c_cc[VSUSP] = 26; /* SUB, ctrl+Z */
	tty->termios.c_cc[VTIME] = 0;
	for (size_t i = 0; i < sizeof(tty->args) / sizeof(*tty->args); ++i)
		tty->args[i] = 0;
	tty->args_nb = 0;
	ret = cdev_alloc(name, 0, 0, 0666, rdev, &g_tty_fop, &tty->cdev);
	if (ret)
	{
		mutex_destroy(&tty->mutex);
		pipebuf_destroy(&tty->pipebuf);
		sma_free(&tty_sma, tty);
		return ret;
	}
	tty->cdev->userdata = tty;
	*ttyp = tty;
	return 0;
}

void tty_free(struct tty *tty)
{
	if (!tty)
		return;
	cdev_free(tty->cdev);
	pipebuf_destroy(&tty->pipebuf);
	mutex_destroy(&tty->mutex);
	sma_free(&tty_sma, tty);
}

static void tty_signal(struct tty *tty, int sig)
{
	if (tty->pgid == -1)
		return;
	struct pgrp *pgrp = getpgrp(tty->pgid);
	if (!pgrp)
		return;
	struct proc *proc;
	pgrp_lock(pgrp);
	TAILQ_FOREACH(proc, &pgrp->processes, pgrp_chain)
		proc_signal(proc, sig);
	pgrp_unlock(pgrp);
	pgrp_free(pgrp);
}

static ssize_t write_chars(struct tty *tty, uint8_t *buf, size_t size)
{
	uint8_t *last_w = buf;
	size_t buf_len = 0;
	ssize_t ret;
	for (size_t i = 0; i < size; ++i)
	{
		switch (buf[i])
		{
			case '\n':
				if (tty->termios.c_oflag & ONLCR)
				{
					if (buf_len)
					{
						ret = tty->op->write(tty, last_w, buf_len);
						if (ret < 0)
							return ret;
						buf_len = 0;
					}
					last_w = &buf[i + 1];
					static const char nlcr[2] = "\r\n";
					ret = tty->op->write(tty, nlcr, 2);
					if (ret < 0)
						return ret;
					continue;
				}
				break;
			case '\r':
				if (!(tty->termios.c_oflag & ONLRET))
				{
					ret = tty->op->write(tty, last_w, buf_len);
					if (ret < 0)
						return ret;
					buf_len = 0;
					last_w = &buf[i + 1];
					continue;
				}
				if (tty->termios.c_oflag & ONOCR)
				{
					/* XXX */
				}
				if (tty->termios.c_oflag & OCRNL)
				{
					if (buf_len)
					{
						ret = tty->op->write(tty, last_w, buf_len);
						if (ret < 0)
							return ret;
						buf_len = 0;
					}
					last_w = &buf[i + 1];
					static const char c = '\n';
					ret = tty->op->write(tty, &c, 1);
					if (ret < 0)
						return ret;
					continue;
				}
				break;
			default:
				break;
		}
		buf_len++;
	}
	if (buf_len)
	{
		ret = tty->op->write(tty, last_w, buf_len);
		if (ret < 0)
			return ret;
	}
	return size;
}

static int write_char(struct tty *tty, uint8_t c)
{
	return write_chars(tty, &c, 1);
}

static int write_specialchar(struct tty *tty, uint8_t c)
{
	if (c < ' ')
	{
		char chars[2] = {'^', c + '@'};
		return tty->op->write(tty, chars, 2);
	}
	return tty->op->write(tty, &c, 1);
}

static int input_char(struct tty *tty, uint8_t c)
{
	if (tty->termios.c_iflag & ISTRIP)
		c &= 0x7F;
	if ((tty->termios.c_iflag & IGNCR) && c == '\r')
		return 1;
	if ((tty->termios.c_iflag & INLCR) && c == '\r')
		c = '\n';
	if (tty->termios.c_lflag & ISIG)
	{
		if (c == tty->termios.c_cc[VINTR])
		{
			write_specialchar(tty, c);
			tty_signal(tty, SIGINT);
			return 1;
		}
		if (c == tty->termios.c_cc[VQUIT])
		{
			write_specialchar(tty, c);
			tty_signal(tty, SIGQUIT);
			return 1;
		}
		if (c == tty->termios.c_cc[VSUSP])
		{
			write_specialchar(tty, c);
			tty_signal(tty, SIGTSTP);
			return 1;
		}
	}
	if (tty->termios.c_iflag & IXON)
	{
		if (c == tty->termios.c_cc[VSTART])
		{
			write_specialchar(tty, c);
			tty->flags |= TTY_STOPPED;
			return 1;
		}
		if (c == tty->termios.c_cc[VSTOP])
		{
			write_specialchar(tty, c);
			tty->flags &= ~TTY_STOPPED;
			return 1;
		}
	}
	if (tty->termios.c_lflag & ICANON)
	{
		if (c == tty->termios.c_cc[VEOF])
		{
			tty->flags |= TTY_EOF;
			struct uio uio;
			struct iovec iov;
			uio_fromkbuf(&uio, &iov, tty->line, tty->line_size, 0);
			if (pipebuf_write_locked(&tty->pipebuf, &uio, 0, NULL) < 0)
				return 0;
			poller_broadcast(&tty->poll_entries, POLLIN);
			tty->line_size = 0;
			/* XXX update cursor */
			write_char(tty, c);
			return 1;
		}
		if (c == tty->termios.c_cc[VEOL] || c == '\n')
		{
			/* XXX handle non-empty rbuf */
			if (tty->line_size == sizeof(tty->line))
				tty->line[sizeof(tty->line) - 1] = c;
			else
				tty->line[tty->line_size++] = c;
			struct uio uio;
			struct iovec iov;
			uio_fromkbuf(&uio, &iov, tty->line, tty->line_size, 0);
			if (pipebuf_write_locked(&tty->pipebuf, &uio, 0, NULL) < 0)
				return 0;
			poller_broadcast(&tty->poll_entries, POLLIN);
			tty->line_size = 0;
			write_char(tty, c);
			/* XXX update cursor */
			return 1;
		}
		if (c == tty->termios.c_cc[VERASE])
		{
			if (!tty->line_size)
				return 1;
			tty->line_size--;
			write_char(tty, '\x7F');
			return 1;
		}
		if (c == tty->termios.c_cc[VKILL])
		{
			/* XXX */
			return 1;
		}
		if (tty->line_size == sizeof(tty->line))
			return 0;
		tty->line[tty->line_size++] = c;
		/* XXX update cursor */
		if (tty->termios.c_lflag & ECHO)
			write_char(tty, c);
		return 1;
	}
	if (tty->termios.c_lflag & ECHO)
		write_char(tty, c);
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, &c, 1, 0);
	if (pipebuf_write_locked(&tty->pipebuf, &uio, 0, NULL) < 0)
		return 0;
	poller_broadcast(&tty->poll_entries, POLLIN);
	return 1;
}

int tty_input(struct tty *tty, char *c, size_t n)
{
	tty_lock(tty);
	for (size_t i = 0; i < n; ++i)
	{
		if (!input_char(tty, c[i]))
			break;
	}
	tty_unlock(tty);
	return 0;
}

static void handle_ctrl_graph(struct tty *tty)
{
	for (int n = 0; n < tty->args_nb; ++n)
	{
		switch (tty->args[n])
		{
			case 0:
				tty->op->ctrl(tty, TTY_CTRL_GC, 0);
				break;
			case 1:
				tty->op->ctrl(tty, TTY_CTRL_GB, 0);
				break;
			case 2:
				tty->op->ctrl(tty, TTY_CTRL_GD, 0);
				break;
			case 3:
				tty->op->ctrl(tty, TTY_CTRL_GI, 0);
				break;
			case 4:
				tty->op->ctrl(tty, TTY_CTRL_GU, 0);
				break;
			case 5:
				tty->op->ctrl(tty, TTY_CTRL_GBL, 0);
				break;
			case 7:
				tty->op->ctrl(tty, TTY_CTRL_GR, 0);
				break;
			case 8:
				tty->op->ctrl(tty, TTY_CTRL_GH, 0);
				break;
			case 9:
				tty->op->ctrl(tty, TTY_CTRL_GS, 0);
				break;
			case 22:
				tty->op->ctrl(tty, TTY_CTRL_GRB, 0);
				break;
			case 23:
				tty->op->ctrl(tty, TTY_CTRL_GRD, 0);
				break;
			case 24:
				tty->op->ctrl(tty, TTY_CTRL_GRU, 0);
				break;
			case 25:
				tty->op->ctrl(tty, TTY_CTRL_GRBL, 0);
				break;
			case 27:
				tty->op->ctrl(tty, TTY_CTRL_GRR, 0);
				break;
			case 28:
				tty->op->ctrl(tty, TTY_CTRL_GRH, 0);
				break;
			case 29:
				tty->op->ctrl(tty, TTY_CTRL_GRS, 0);
				break;
			case 30:
			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 39:
				tty->op->ctrl(tty, TTY_CTRL_GFG, tty->args[n] - 30);
				break;
			case 38:
				n++;
				if (n >= tty->args_nb)
					break;
				switch (tty->args[n])
				{
					case 5:
						n++;
						if (n >= tty->args_nb)
							break;
						tty->op->ctrl(tty, TTY_CTRL_GFG256, tty->args[n]);
						break;
					case 2:
					{
						uint8_t r;
						uint8_t g;
						uint8_t b;
						n++;
						if (n >= tty->args_nb)
							break;
						r = tty->args[n];
						n++;
						if (n >= tty->args_nb)
							break;
						g = tty->args[n];
						n++;
						if (n >= tty->args_nb)
							break;
						b = tty->args[n];
						tty->op->ctrl(tty, TTY_CTRL_GFG24, (r << 16) | (g << 8) | b);
						break;
					}
				}
				break;
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
			case 49:
				tty->op->ctrl(tty, TTY_CTRL_GBG, tty->args[n] - 40);
				break;
			case 48:
				n++;
				if (n >= tty->args_nb)
					break;
				switch (tty->args[n])
				{
					case 5:
						n++;
						if (n >= tty->args_nb)
							break;
						tty->op->ctrl(tty, TTY_CTRL_GBG256, tty->args[n]);
						break;
					case 2:
					{
						uint8_t r;
						uint8_t g;
						uint8_t b;
						n++;
						if (n >= tty->args_nb)
							break;
						r = tty->args[n];
						n++;
						if (n >= tty->args_nb)
							break;
						g = tty->args[n];
						n++;
						if (n >= tty->args_nb)
							break;
						b = tty->args[n];
						tty->op->ctrl(tty, TTY_CTRL_GBG24, (r << 16) | (g << 8) | b);
						break;
					}
				}
				break;
			case 90:
			case 91:
			case 92:
			case 93:
			case 94:
			case 95:
			case 96:
			case 97:
				tty->op->ctrl(tty, TTY_CTRL_GFGB, tty->args[n] - 90);
				break;
			case 100:
			case 101:
			case 102:
			case 103:
			case 104:
			case 105:
			case 106:
			case 107:
				tty->op->ctrl(tty, TTY_CTRL_GBGB, tty->args[n] - 100);
				break;
			default:
				/* unknown */
				break;
		}
	}
}

static void inc(struct tty *tty, char c)
{
	switch (tty->ctrl_state)
	{
		case 0: /* no state */
			if (!(tty->termios.c_oflag & OPOST) || c != '\033')
			{
				/* XXX bufferize the non-op chars to reduce overhead */
				write_char(tty, c);
				break;
			}
			tty->ctrl_state = 1;
			break;
		case 1: /* after ESC */
			switch (c)
			{
				case '[':
					tty->ctrl_state = 2;
					break;
				case '7':
					tty->op->ctrl(tty, TTY_CTRL_DECSC, 0);
					tty->ctrl_state = 0;
					break;
				case '8':
					tty->op->ctrl(tty, TTY_CTRL_DECRC, 0);
					tty->ctrl_state = 0;
					break;
				default:
					/* unknown */
					break;
			}
			break;
		case 2: /* after ESC[ */
			if (isdigit(c))
			{
				if (tty->args_nb < sizeof(tty->args) / sizeof(*tty->args))
					tty->args[tty->args_nb] = tty->args[tty->args_nb] * 10 + (c - '0');
				break;
			}
			tty->args_nb++;
			switch (c)
			{
				case ';':
					if (tty->args_nb < sizeof(tty->args) / sizeof(*tty->args))
						tty->args[tty->args_nb] = 0;
					return;
				case 'A':
					tty->op->ctrl(tty, TTY_CTRL_CUU, tty->args[0]);
					break;
				case 'B':
					tty->op->ctrl(tty, TTY_CTRL_CUD, tty->args[0]);
					break;
				case 'C':
					tty->op->ctrl(tty, TTY_CTRL_CUF, tty->args[0]);
					break;
				case 'D':
					tty->op->ctrl(tty, TTY_CTRL_CUB, tty->args[0]);
					break;
				case 'E':
					tty->op->ctrl(tty, TTY_CTRL_CNL, tty->args[0]);
					break;
				case 'F':
					tty->op->ctrl(tty, TTY_CTRL_CPL, tty->args[0]);
					break;
				case 'G':
					tty->op->ctrl(tty, TTY_CTRL_CHA, tty->args[0]);
					break;
				case 'H':
				case 'f':
				{
					uint8_t line;
					uint8_t column;
					if (tty->args_nb == 1)
					{
						line = 0;
						column = 0;
					}
					else if (tty->args_nb == 2)
					{
						line = tty->args[0];
						column = tty->args[1];
					}
					else
					{
						break;
					}
					tty->op->ctrl(tty, TTY_CTRL_CUP, (line << 8) | column);
					break;
				}
				case 'J':
					tty->op->ctrl(tty, TTY_CTRL_ED, tty->args[0]);
					break;
				case 'K':
					tty->op->ctrl(tty, TTY_CTRL_EL, tty->args[0]);
					break;
				case 'S':
					tty->op->ctrl(tty, TTY_CTRL_SU, tty->args[0]);
					break;
				case 'T':
					tty->op->ctrl(tty, TTY_CTRL_SD, tty->args[0]);
					break;
				case 'm':
					handle_ctrl_graph(tty);
					break;
				case 'n':
					if (tty->args[0] == 6)
					{
						char buf[64];
						snprintf(buf, sizeof(buf), "\033[%" PRIu32 ";%" PRIu32 "R", tty->cursor_x, tty->cursor_y);
						write_chars(tty, (uint8_t*)buf, strlen(buf));
					}
					break;
				case 's':
					tty->op->ctrl(tty, TTY_CTRL_SCP, 0);
					break;
				case 'u':
					tty->op->ctrl(tty, TTY_CTRL_RCP, 0);
					break;
				case '=':
					tty->ctrl_state = 3;
					return;
				case '?':
					tty->ctrl_state = 4;
					return;
				default:
					/* unknown */
					break;
			}
			tty->args[0] = 0;
			tty->args_nb = 0;
			tty->ctrl_state = 0;
			break;
		case 3: /* after ESC[= */
			if (isdigit(c))
			{
				if (tty->args_nb < sizeof(tty->args) / sizeof(*tty->args))
					tty->args[tty->args_nb] = tty->args[tty->args_nb] * 10 + (c - '0');
				break;
			}
			tty->args_nb++;
			switch (c)
			{
				case 'h':
					switch (tty->args[0])
					{
						case 0:
							tty->op->ctrl(tty, TTY_CTRL_S0, 0);
							break;
						case 1:
							tty->op->ctrl(tty, TTY_CTRL_S1, 0);
							break;
						case 2:
							tty->op->ctrl(tty, TTY_CTRL_S2, 0);
							break;
						case 3:
							tty->op->ctrl(tty, TTY_CTRL_S3, 0);
							break;
						case 4:
							tty->op->ctrl(tty, TTY_CTRL_S4, 0);
							break;
						case 5:
							tty->op->ctrl(tty, TTY_CTRL_S5, 0);
							break;
						case 6:
							tty->op->ctrl(tty, TTY_CTRL_S6, 0);
							break;
						case 7:
							tty->op->ctrl(tty, TTY_CTRL_SLW, 0);
							break;
						case 13:
							tty->op->ctrl(tty, TTY_CTRL_S13, 0);
							break;
						case 14:
							tty->op->ctrl(tty, TTY_CTRL_S14, 0);
							break;
						case 15:
							tty->op->ctrl(tty, TTY_CTRL_S15, 0);
							break;
						case 16:
							tty->op->ctrl(tty, TTY_CTRL_S16, 0);
							break;
						case 17:
							tty->op->ctrl(tty, TTY_CTRL_S17, 0);
							break;
						case 18:
							tty->op->ctrl(tty, TTY_CTRL_S18, 0);
							break;
						case 19:
							tty->op->ctrl(tty, TTY_CTRL_S19, 0);
							break;
						default:
							/* unknown */
							break;
					}
					break;
				default:
					/* unknown */
					break;
			}
			tty->args[0] = 0;
			tty->args_nb = 0;
			tty->ctrl_state = 0;
			break;
		case 4: /* after ESC[? */
			if (isdigit(c))
			{
				if (tty->args_nb < sizeof(tty->args) / sizeof(*tty->args))
					tty->args[tty->args_nb] = tty->args[tty->args_nb] * 10 + (c - '0');
				break;
			}
			tty->args_nb++;
			switch (c)
			{
				case 'l':
					switch (tty->args[0])
					{
						case 25:
							tty->op->ctrl(tty, TTY_CTRL_PCD, 0);
							break;
						case 47:
							tty->op->ctrl(tty, TTY_CTRL_PSR, 0);
							break;
						default:
							/* unknown */
							break;
					}
					break;
				case 'h':
					switch (tty->args[0])
					{
						case 25:
							tty->op->ctrl(tty, TTY_CTRL_PCE, 0);
							break;
						case 47:
							tty->op->ctrl(tty, TTY_CTRL_PSS, 0);
							break;
						default:
							/* unknown */
							break;
					}
					break;
				default:
					/* unknown */
					break;
			}
			break;
	}
}

int tty_write(struct tty *tty, struct uio *uio)
{
	if (!tty || !tty->op)
		return -EINVAL; /* XXX */
	if (!uio->count)
		return 0;
	size_t wr = 0;
	while (uio->count)
	{
		char buf[PAGE_SIZE];
		ssize_t ret = uio_copyout(buf, uio, sizeof(buf));
		if (ret < 0)
			return ret;
		if (!ret)
			break;
		tty_lock(tty);
		if (tty->flags & TTY_NOCTRL)
		{
			write_chars(tty, (uint8_t*)buf, ret);
		}
		else
		{
			for (ssize_t i = 0; i < ret; ++i)
				inc(tty, buf[i]);
		}
		/* XXX move the flush out of the loop ? */
		if (tty->op->flush)
			tty->op->flush(tty);
		tty_unlock(tty);
		wr += ret;
	}
	return wr;
}
