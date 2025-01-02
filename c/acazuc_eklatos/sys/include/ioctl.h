#ifndef IOCTL_H
#define IOCTL_H

#define TCGETS  0x1
#define TCSETS  0x2
#define TCSETSW 0x3
#define TCSETSF 0x4
#define TCSBRK  0x5
#define TCFLSH  0x6
#define TCXONC  0x7

#define TIOCGWINSZ  0x10
#define TIOCSWINSZ  0x11
#define TIOCSPTLCK  0x12
#define TIOCGPTLCK  0x13
#define TIOCGPTN    0x14
#define TIOCGPTPEER 0x15
#define TIOCGPGRP   0x16
#define TIOCSPGRP   0x17
#define TIOCGSID    0x18

#define SIOCGIFADDR    0x101
#define SIOCSIFADDR    0x102
#define SIOCDIFADDR    0x103
#define SIOCGIFNETMASK 0x104
#define SIOCSIFNETMASK 0x105
#define SIOCGIFHWADDR  0x106
#define SIOCSIFHWADDR  0x107
#define SIOCGIFCONF    0x108
#define SIOCGIFNAME    0x109
#define SIOCGIFFLAGS   0x10A
#define SIOCSIFFLAGS   0x10B

#define SIOCSGATEWAY 0x201 /* XXX should be replaced by route management */
#define SIOCGGATEWAY 0x202 /* XXX should be replaced by route management */

#endif
