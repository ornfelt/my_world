#ifndef JKS_OPTIONAL_H
#define JKS_OPTIONAL_H

#include <stdbool.h>
#include <stdint.h>

#define OPTIONAL_DEF(name, type) \
	struct name \
	{ \
		char set; \
		type val; \
	}

#define OPTIONAL_CTR(opt, v) \
	do \
	{ \
		OPTIONAL_SET(opt); \
		(opt).val = (v); \
	} while (0)

#define OPTIONAL_SET(opt) \
	do \
	{ \
		(opt).set = 1; \
	} while (0)

#define OPTIONAL_UNSET(opt) \
	do \
	{ \
		(opt).set = 0; \
	} while (0)

#define OPTIONAL_GET(opt) ((opt).val)
#define OPTIONAL_ISSET(opt) ((opt).set != 0)

typedef struct optional
{
	char set;
	char val[];
} optional_t;

OPTIONAL_DEF(optional_uint8, uint8_t);
OPTIONAL_DEF(optional_int8, int8_t);
OPTIONAL_DEF(optional_uint16, uint16_t);
OPTIONAL_DEF(optional_int16, int16_t);
OPTIONAL_DEF(optional_uint32, uint32_t);
OPTIONAL_DEF(optional_int32, int32_t);
OPTIONAL_DEF(optional_uint64, uint64_t);
OPTIONAL_DEF(optional_int64, int64_t);
OPTIONAL_DEF(optional_float, float);
OPTIONAL_DEF(optional_double, double);
OPTIONAL_DEF(optional_bool, bool);

#endif
