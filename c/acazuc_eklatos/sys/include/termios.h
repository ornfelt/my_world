#ifndef TERMIOS_H
#define TERMIOS_H

#include <types.h>

#define IGNBRK (1 << 0)
#define BRKINT (1 << 1)
#define IGNPAR (1 << 2)
#define PARMRK (1 << 3)
#define INPCK  (1 << 4)
#define ISTRIP (1 << 5)
#define INLCR  (1 << 6)
#define IGNCR  (1 << 7)
#define ICRNL  (1 << 8)
#define IXON   (1 << 9)
#define IXANY  (1 << 10)
#define IXOFF  (1 << 11)

#define OPOST  (1 << 0)
#define ONLCR  (1 << 1)
#define OCRNL  (1 << 2)
#define ONOCR  (1 << 3)
#define ONLRET (1 << 4)
#define OFILL  (1 << 5)
#define OFDEL  (1 << 6)
#define NLDLY  (1 << 7)
#define CRDLY  (1 << 8)
#define TABDLY (1 << 9)
#define BSDLY  (1 << 10)
#define VTDLY  (1 << 11)
#define FFDLY  (1 << 12)

#define CBAUD   (0x1F << 0)
#define B0      (0 << 0)
#define B50     (1 << 0)
#define B75     (2 << 0)
#define B110    (3 << 0)
#define B134    (4 << 0)
#define B150    (5 << 0)
#define B200    (6 << 0)
#define B300    (7 << 0)
#define B600    (8 << 0)
#define B1200   (9 << 0)
#define B1800   (10 << 0)
#define B2400   (11 << 0)
#define B4800   (12 << 0)
#define B9600   (13 << 0)
#define B19200  (14 << 0)
#define B38400  (15 << 0)
#define B57600  (16 << 0)
#define B115200 (17 << 0)
#define B230400 (18 << 0)
#define CSIZE  (3 << 6)
#define CS5    (0 << 6)
#define CS6    (1 << 6)
#define CS7    (2 << 6)
#define CS8    (3 << 6)
#define CSTOPB (1 << 8)
#define CREAD  (1 << 9)
#define PARENB (1 << 10)
#define PARODD (1 << 11)
#define HUPCL  (1 << 12)
#define CLOCAL (1 << 13)

#define ISIG   (1 << 0)
#define ICANON (1 << 1)
#define ECHO   (1 << 2)
#define ECHOE  (1 << 3)
#define ECHOK  (1 << 4)
#define ECHONL (1 << 5)
#define NOFLSH (1 << 6)
#define TOSTOP (1 << 7)
#define IEXTEN (1 << 8)

#define VEOF   0x0
#define VEOL   0x1
#define VERASE 0x2
#define VINTR  0x3
#define VKILL  0x4
#define VMIN   0x5
#define VQUIT  0x6
#define VSTART 0x7
#define VSTOP  0x8
#define VSUSP  0x9
#define VTIME  0xA

#define NCCS 0xB

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#define TCIFLUSH  0
#define TCOFLUSH  1
#define TCIOFLUSH 2

#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3

typedef uint32_t tcflag_t;
typedef uint8_t cc_t;
typedef uint32_t speed_t;

struct termios
{
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
	speed_t c_ispeed;
	speed_t c_ospeed;
};

struct winsize
{
	uint16_t ws_row;
	uint16_t ws_col;
	uint16_t ws_xpixel;
	uint16_t ws_ypixel;
};

#endif
