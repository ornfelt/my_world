#ifndef _STRING_H
#define _STRING_H

#define fillsize(v) ((v) * ((size_t)-1 / 255))
#define haszero(x) (((x) - fillsize(1)) & ~(x) & fillsize(0x80))

#endif
