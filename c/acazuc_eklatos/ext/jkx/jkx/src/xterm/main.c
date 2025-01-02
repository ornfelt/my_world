#include "ringbuf.h"

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <sys/ioctl.h>

#define __USE_XOPEN2KXSI
#define __USE_XOPEN_EXTENDED
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <poll.h>
#include <pty.h>

#define LINES_HIST 1024

extern char **environ;

struct xterm
{
	const char *progname;
	Display *display;
	Window window;
	Cursor cursor;
	Font font;
	GC bg_gc;
	GC gc;
	char *lines[LINES_HIST];
	size_t lines_pos;
	pid_t shell;
	int pty;
	int pty_rok;
	int pty_wok;
	struct ringbuf pty_rbuf;
	struct ringbuf pty_wbuf;
	char line[4096];
	size_t line_pos;
	size_t char_width;
	size_t char_height;
	size_t window_width;
	size_t window_height;
	size_t rows;
	size_t cols;
};

static void input_char(struct xterm *xterm, char c)
{
	ringbuf_wu8(&xterm->pty_wbuf, c);
}

static void input_string(struct xterm *xterm, const char *s)
{
	ringbuf_write(&xterm->pty_wbuf, s, strlen(s));
}

static void input_sequence(struct xterm *xterm, const char *code, unsigned mods)
{
	unsigned modv = 0;
	if (mods & ShiftMask)
		modv |= 1;
	if (mods & Mod1Mask)
		modv |= 2;
	if (mods & ControlMask)
		modv |= 4;
	if (mods & Mod4Mask)
		modv |= 8;
	char buf[64];
	if (modv)
		snprintf(buf, sizeof(buf), "\033[%u;%s", modv, code);
	else
		snprintf(buf, sizeof(buf), "\033[%s", code);
	input_string(xterm, buf);
}

static int process_x11(struct xterm *xterm)
{
	while (XPending(xterm->display))
	{
		XEvent event;
		if (XNextEvent(xterm->display, &event))
			return 1;
		switch (event.type)
		{
			case KeyPress:
			{
				KeySym sym = XLookupKeysym(&event.xkey, 0);
				if (sym < 127)
				{
					if (event.xkey.state & ControlMask)
					{
						if (sym < 0x40)
							break;
						if (islower(sym))
							input_char(xterm, sym - 0x60);
						else
							input_char(xterm, sym - 0x40);
					}
					else
					{
						input_char(xterm, sym);
					}
				}
				else if (sym == XK_Backspace)
				{
					ringbuf_wu8(&xterm->pty_wbuf, '\b');
				}
				else if (sym == XK_Tab)
				{
					ringbuf_wu8(&xterm->pty_wbuf, '\t');
				}
				else if (sym == XK_Return)
				{
					ringbuf_wu8(&xterm->pty_wbuf, '\n');
				}
				else if (sym == XK_Delete)
				{
					ringbuf_wu8(&xterm->pty_wbuf, 0x7F);
				}
				else if (sym == XK_Up)
				{
					input_sequence(xterm, "A", event.xkey.state);
				}
				else if (sym == XK_Down)
				{
					input_sequence(xterm, "B", event.xkey.state);
				}
				else if (sym == XK_Right)
				{
					input_sequence(xterm, "C", event.xkey.state);
				}
				else if (sym == XK_Left)
				{
					input_sequence(xterm, "D", event.xkey.state);
				}
				break;
			}
			case ConfigureNotify:
			{
				if (event.xconfigure.window == xterm->window)
				{
					xterm->window_width = event.xconfigure.width;
					xterm->window_height = event.xconfigure.height;
					xterm->rows = xterm->window_height / xterm->char_height;
					xterm->cols = xterm->window_width / xterm->char_width;
					struct winsize winsize;
					winsize.ws_row = xterm->rows;
					winsize.ws_col = xterm->cols;
					winsize.ws_xpixel = 0;
					winsize.ws_ypixel = 0;
					if (ioctl(xterm->pty, TIOCSWINSZ, &winsize) == -1)
						fprintf(stderr, "%s: failed to resize tty\n",
						        xterm->progname);
					/* XXX SIGWINCH */
				}
				break;
			}
		}
	}
	return 0;
}

static int shell_write(struct xterm *xterm)
{
	if (!xterm->pty_wok || !ringbuf_read_size(&xterm->pty_wbuf))
		return 0;
	ssize_t ret = write(xterm->pty,
	                    ringbuf_read_ptr(&xterm->pty_wbuf),
	                    ringbuf_contiguous_read_size(&xterm->pty_wbuf));
	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
			return 0;
		fprintf(stderr, "%s: write: %s\n", xterm->progname,
		        strerror(errno));
		return 1;
	}
	ringbuf_advance_read(&xterm->pty_wbuf, ret);
	return 0;
}

static int shell_read(struct xterm *xterm)
{
	if (!xterm->pty_rok || !ringbuf_write_size(&xterm->pty_rbuf))
		return 0;
	ssize_t ret = read(xterm->pty,
	                   ringbuf_write_ptr(&xterm->pty_rbuf),
	                   ringbuf_contiguous_write_size(&xterm->pty_rbuf));
	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
			return 0;
		fprintf(stderr, "%s: read: %s\n", xterm->progname,
		        strerror(errno));
		return 1;
	}
	ringbuf_advance_write(&xterm->pty_rbuf, ret);
	return 0;
}

static int process_shell(struct xterm *xterm)
{
	if (shell_read(xterm))
		return 1;
	if (shell_write(xterm))
		return 1;
	size_t n = ringbuf_read(&xterm->pty_rbuf,
	                        &xterm->line[xterm->line_pos],
	                        sizeof(xterm->line) - xterm->line_pos);
	xterm->line_pos += n;
	char *nl;
	while ((nl = memchr(xterm->line, '\n', xterm->line_pos)))
	{
		size_t len = nl - xterm->line + 1;
		char *line = malloc(len);
		if (!line)
		{
			fprintf(stderr, "%s: malloc: %s\n", xterm->progname,
			        strerror(errno));
			return 1;
		}
		memcpy(line, xterm->line, len - 1);
		line[len - 1] = '\0';
		free(xterm->lines[xterm->lines_pos]);
		xterm->lines[xterm->lines_pos++] = line;
		memmove(xterm->line, nl + 1, xterm->line_pos - len);
		xterm->line_pos -= len;
	}
	return 0;
}

static int process_display(struct xterm *xterm)
{
	XClearWindow(xterm->display, xterm->window);
	XFillRectangle(xterm->display, xterm->window, xterm->bg_gc, 0, 0,
	               xterm->window_width, xterm->window_height);
	{
		size_t lines = (xterm->line_pos + xterm->cols - 1) / xterm->cols;
		char line_buf[4096];
		size_t line_len = 0;
		for (size_t i = 0; i < xterm->line_pos; ++i)
		{
			if (line_len >= xterm->cols)
			{
				line_buf[line_len] = '\0';
				XDrawString(xterm->display, xterm->window, xterm->gc, 0,
				            xterm->char_height * (xterm->rows + 1 - lines + i / xterm->cols),
				            line_buf, line_len);
				line_len = 0;
			}
			switch (xterm->line[i])
			{
				case '\n':
				case '\r':
					break;
				case '\t':
					for (size_t n = line_len % 8; n < 8; ++n)
						line_buf[line_len++] = ' ';
					break;
				default:
					line_buf[line_len++] = xterm->line[i];
					break;
			}
		}
		if (line_len)
		{
			line_buf[line_len] = '\0';
			XDrawString(xterm->display, xterm->window, xterm->gc, 0,
			            xterm->char_height * (xterm->rows + 1 - lines + xterm->line_pos / xterm->cols),
			            line_buf, line_len);
		}
	}
	ssize_t y = xterm->rows + 1 - (xterm->line_pos + xterm->cols - 1) / xterm->cols;
	size_t i = 0;
	while (y > 0)
	{
		char *text = xterm->lines[(xterm->lines_pos + LINES_HIST - i - 1) % LINES_HIST];
		if (!text)
			break;
		size_t len = strlen(text); /* XXX cache */
		size_t lines = (len + xterm->cols - 1) / xterm->cols;
		char line_buf[4096];
		size_t line_len = 0;
		for (size_t n = 0; n < len; ++n)
		{
			if (line_len >= xterm->cols)
			{
				ssize_t pos_y = xterm->char_height * (y - lines + n / xterm->cols);
				if (pos_y > 0)
				{
					line_buf[line_len] = '\0';
					XDrawString(xterm->display, xterm->window, xterm->gc, 0,
					            pos_y, line_buf, line_len);
				}
				line_len = 0;
				break;
			}
			switch (text[n])
			{
				case '\n':
				case '\r':
					break;
				case '\t':
					for (size_t n = line_len % 8; n < 8; ++n)
						line_buf[line_len++] = ' ';
					break;
				default:
					line_buf[line_len++] = text[n];
					break;
			}
		}
		if (line_len)
		{
			ssize_t pos_y = xterm->char_height * (y - lines + len / xterm->cols);
			if (pos_y > 0)
			{
				line_buf[line_len] = '\0';
				XDrawString(xterm->display, xterm->window, xterm->gc, 0,
				            pos_y, line_buf, line_len);
			}
		}
		y -= lines;
		i++;
	}
	return 0;
}

static int start_shell(struct xterm *xterm)
{
	pid_t ret = forkpty(&xterm->pty, NULL, NULL, NULL);
	if (ret == -1)
	{
		fprintf(stderr, "%s: forkpty: %s\n", xterm->progname,
		        strerror(errno));
		return 1;
	}
	if (ret)
	{
		int fl = fcntl(xterm->pty, F_GETFL, 0);
		if (fcntl(xterm->pty, F_SETFL, fl | O_NONBLOCK) == -1)
		{
			fprintf(stderr, "%s: fcntl: %s\n", xterm->progname,
			        strerror(errno));
			return 1;
		}
		return 0;
	}
	char *argv[] =
	{
		"/bin/sh",
		NULL,
	};
	execve("/bin/sh", argv, environ);
	fprintf(stderr, "%s: execve: %s\n", xterm->progname, strerror(errno));
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct xterm xterm;

	(void)argc;
	memset(&xterm, 0, sizeof(xterm));
	if (ringbuf_init(&xterm.pty_rbuf, 4096)
	 || ringbuf_init(&xterm.pty_wbuf, 4096))
	{
		fprintf(stderr, "%s: malloc: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	xterm.progname = argv[0];
	xterm.display = XOpenDisplay(NULL);
	if (!xterm.display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	xterm.font = XLoadFont(xterm.display, "terminus-16");
	if (xterm.font)
	{
		xterm.char_width = 8;
		xterm.char_height = 16;
	}
	else
	{
		xterm.font = XLoadFont(xterm.display, "8x13");
		if (xterm.font)
		{
			xterm.char_width = 8;
			xterm.char_height = 13;
		}
		else
		{
			xterm.font = XLoadFont(xterm.display, "8x16");
			if (xterm.font)
			{
				xterm.char_width = 8;
				xterm.char_height = 16;
			}
			else
			{
				fprintf(stderr, "%s: failed to open font\n",
				        argv[0]);
				return EXIT_FAILURE;
			}
		}
	}
	Window root = XRootWindow(xterm.display, 0);
	int screen = DefaultScreen(xterm.display);
	XVisualInfo vi;
	if (!XMatchVisualInfo(xterm.display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask
	               | KeyPressMask
	               | KeyReleaseMask
	               | ButtonPressMask
	               | ButtonReleaseMask
	               | PointerMotionMask
	               | FocusChangeMask
	               | StructureNotifyMask;
	unsigned mask = CWEventMask;
	xterm.rows = 25;
	xterm.cols = 80;
	xterm.window_width = xterm.cols * xterm.char_width;
	xterm.window_height = xterm.rows * xterm.char_height;
	xterm.window = XCreateWindow(xterm.display, root, 0, 0,
	                             xterm.window_width, xterm.window_height,
	                             0, vi.depth, InputOutput, vi.visual, mask,
	                             &swa);
	if (!xterm.window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XChangeProperty(xterm.display, xterm.window, XA_WM_NAME, XA_STRING, 8,
	                PropModeReplace, (uint8_t*)"xterm", 6);
	xterm.cursor = XCreateFontCursor(xterm.display, XC_xterm);
	if (!xterm.cursor)
	{
		fprintf(stderr, "%s: failed to create cursor\n", argv[0]);
		return EXIT_FAILURE;
	}
	XDefineCursor(xterm.display, xterm.window, xterm.cursor);
	XGCValues gc_values;
	gc_values.foreground = 0x00FFFFFF;
	gc_values.background = 0x00000000;
	gc_values.font = xterm.font;
	gc_values.graphics_exposures = 0;
	unsigned gc_mask = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
	xterm.gc = XCreateGC(xterm.display, xterm.window, gc_mask, &gc_values);
	if (!xterm.gc)
	{
		fprintf(stderr, "%s: failed to create gc\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	gc_values.foreground = 0x00000000;
	gc_mask = GCForeground;
	xterm.bg_gc = XCreateGC(xterm.display, xterm.window, gc_mask, &gc_values);
	if (!xterm.bg_gc)
	{
		fprintf(stderr, "%s: failed to create background gc\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(xterm.display, xterm.window);
	XSynchronize(xterm.display, False);
	if (start_shell(&xterm))
		return EXIT_FAILURE;
	while (1)
	{
		struct pollfd fds[3];
		int nfd = 2;
		fds[0].fd = XConnectionNumber(xterm.display);
		fds[0].events = POLLIN;
		fds[1].fd = xterm.pty;
		fds[1].events = POLLIN;
		if (!xterm.pty_wok)
			fds[1].events |= POLLOUT;
		int ret = poll(fds, nfd, -1);
		if (ret == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "%s: poll: %s\n", xterm.progname,
			        strerror(errno));
			return EXIT_FAILURE;
		}
		if (!ret)
			continue;
		if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
		 || (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)))
		{
			fprintf(stderr, "%s: pipe error\n", xterm.progname);
			return EXIT_FAILURE;
		}
		if (fds[1].revents & POLLIN)
			xterm.pty_rok = 1;
		if (fds[1].revents & POLLOUT)
			xterm.pty_wok = 1;
		if (process_x11(&xterm))
			return EXIT_FAILURE;
		if (process_shell(&xterm))
			return EXIT_FAILURE;
		if (process_display(&xterm))
			return EXIT_FAILURE;
		XFlush(xterm.display);
	}
	return EXIT_SUCCESS;
}
