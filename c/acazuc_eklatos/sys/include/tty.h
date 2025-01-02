#ifndef TTY_H
#define TTY_H

#include <termios.h>
#include <pipebuf.h>
#include <types.h>
#include <mutex.h>
#include <poll.h>

#define TTY_STOPPED (1 << 0)
#define TTY_EOF     (1 << 1)
#define TTY_NOCTRL  (1 << 2)

struct tty_op;
struct vtty;
struct cdev;
struct fb;

struct tty
{
	struct cdev *cdev;
	struct termios termios;
	struct winsize winsize;
	struct poller_head poll_entries;
	struct pipebuf pipebuf;
	const struct tty_op *op;
	uint8_t args[8]; /* escape codes args; XXX: resize ? */
	uint8_t args_nb;
	uint8_t line[4096]; /* for ICANON; XXX move to vga / txt tty implem (make a terminal struct for kernel-base tty ? ) */
	uint32_t flags;
	size_t line_size;
	size_t ctrl_state;
	struct mutex mutex;
	struct waitq rwaitq;
	struct waitq wwaitq;
	uint32_t cursor_x;
	uint32_t cursor_y;
	void *userdata;
	pid_t pgid;
};

enum tty_ctrl
{
	TTY_CTRL_CUU, /* cursor up */
	TTY_CTRL_CUD, /* cursor down */
	TTY_CTRL_CUF, /* cursor forward */
	TTY_CTRL_CUB, /* cursor back */
	TTY_CTRL_CNL, /* cursor next line */
	TTY_CTRL_CPL, /* cursor previous line */
	TTY_CTRL_CHA, /* cursor horizontal absolute */
	TTY_CTRL_CUP, /* cursor position */
	TTY_CTRL_ED,  /* erase in display */
	TTY_CTRL_EL,  /* erase in line */
	TTY_CTRL_SU,  /* scroll up */
	TTY_CTRL_SD,  /* scroll down */
	TTY_CTRL_DSR, /* device status report */
	TTY_CTRL_SCP, /* cursor save pos */
	TTY_CTRL_RCP, /* cursor restore pos */
	TTY_CTRL_ES,  /* erase screen */
	TTY_CTRL_ESL, /* erase saved line */

	TTY_CTRL_DECSC, /* DEC save cursor */
	TTY_CTRL_DECRC, /* DEC restore cursor */

	TTY_CTRL_GC, /* graph clear */
	TTY_CTRL_GB, /* graph bold */
	TTY_CTRL_GRB, /* graph reset bold */
	TTY_CTRL_GD, /* graph dim */
	TTY_CTRL_GRD, /* graph reset dim */
	TTY_CTRL_GI, /* graph italic */
	TTY_CTRL_GRI, /* graph reset italic */
	TTY_CTRL_GU, /* graph underline */
	TTY_CTRL_GRU, /* graph reset underline */
	TTY_CTRL_GBL, /* graph blink */
	TTY_CTRL_GRBL, /* graph reset blink */
	TTY_CTRL_GR, /* graph reverse */
	TTY_CTRL_GRR, /* graph reset reverse */
	TTY_CTRL_GH, /* graph hidden */
	TTY_CTRL_GRH, /* graph reset hidden */
	TTY_CTRL_GS, /* graph strikethrough */
	TTY_CTRL_GRS, /* graph reset strikethrough */
	TTY_CTRL_GFG, /* graph foreground color */
	TTY_CTRL_GBG, /* graph background color */
	TTY_CTRL_GFG24, /* graph foreground color 24 bits */
	TTY_CTRL_GBG24, /* graph background color 24 bits */
	TTY_CTRL_GFG256, /* graph foreground color 256 */
	TTY_CTRL_GBG256, /* graph background color 256 */
	TTY_CTRL_GFGB, /* graph foreground bright */
	TTY_CTRL_GBGB, /* graph background bright */
	TTY_CTRL_S0, /* 40x25 monochrome text */
	TTY_CTRL_S1, /* 40x25 color text */
	TTY_CTRL_S2, /* 80x25 monochrome text */
	TTY_CTRL_S3, /* 80x25 color text */
	TTY_CTRL_S4, /* 320x200x4 bitmap */
	TTY_CTRL_S5, /* 320x200x1 bitmap */
	TTY_CTRL_S6, /* 640x200x1 bitmap */
	TTY_CTRL_SLW, /* enable line wrap */
	TTY_CTRL_S13, /* 320x200x24 bitmap */
	TTY_CTRL_S14, /* 640x200x4 bitmap */
	TTY_CTRL_S15, /* 640x350x1 bitmap */
	TTY_CTRL_S16, /* 640x350x4 bitmap */
	TTY_CTRL_S17, /* 640x480x2 bitmap */
	TTY_CTRL_S18, /* 640x480x4 bitmap */
	TTY_CTRL_S19, /* 320x200x8 bitmap */
	TTY_CTRL_SR, /* screen reset */
	TTY_CTRL_PCD, /* cursor disable */
	TTY_CTRL_PCE, /* cursor enable */
	TTY_CTRL_PSR, /* screen restore */
	TTY_CTRL_PSS, /* screen save */
	TTY_CTRL_PABE, /* alternative buffer enable */
	TTY_CTRL_PABD, /* alternative buffer disable */
};

struct tty_op
{
	ssize_t (*write)(struct tty *tty, const void *data, size_t len);
	int (*ctrl)(struct tty *tty, enum tty_ctrl ctrl, uint32_t val);
	int (*ioctl)(struct tty *tty, unsigned long request, uintptr_t data);
	void (*flush)(struct tty *tty);
};

int tty_alloc(const char *name, dev_t rdev, const struct tty_op *op,
              struct tty **tty);
void tty_free(struct tty *tty);
int tty_input(struct tty *tty, char *c, size_t n);
int tty_write(struct tty *tty, struct uio *uio);

int pty_init(void);

int vtty_alloc(const char *name, int id, struct fb *fb, struct tty **ttyp);

static inline void tty_lock(struct tty *tty)
{
	mutex_lock(&tty->mutex);
}

static inline void tty_unlock(struct tty *tty)
{
	mutex_unlock(&tty->mutex);
}

extern struct tty *curtty;

#endif
