#ifndef INTTYPES_H
#define INTTYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __SIZE_WIDTH__ == 32

#define PRIi8 "i"
#define PRId8 "d"
#define PRIu8 "u"
#define PRIo8 "o"
#define PRIx8 "x"
#define PRIX8 "X"

#define PRIi16 "i"
#define PRId16 "d"
#define PRIu16 "u"
#define PRIo16 "o"
#define PRIx16 "x"
#define PRIX16 "X"

#define PRIi32 "li"
#define PRId32 "ld"
#define PRIu32 "lu"
#define PRIo32 "lo"
#define PRIx32 "lx"
#define PRIX32 "lX"

#define PRIi64 "lli"
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIo64 "llo"
#define PRIx64 "llx"
#define PRIX64 "llX"

#define PRIiMAX "li"
#define PRIdMAX "ld"
#define PRIuMAX "lu"
#define PRIoMAX "lo"
#define PRIxMAX "lx"
#define PRIXMAX "lX"

#define PRIiPTR "li"
#define PRIdPTR "ld"
#define PRIuPTR "lu"
#define PRIoPTR "lo"
#define PRIxPTR "lx"
#define PRIXPTR "lX"

#define SCNi8 "i"
#define SCNd8 "d"
#define SCNu8 "u"
#define SCNo8 "o"
#define SCNx8 "x"
#define SCNX8 "X"

#define SCNi16 "i"
#define SCNd16 "d"
#define SCNu16 "u"
#define SCNo16 "o"
#define SCNx16 "x"
#define SCNX16 "X"

#define SCNi32 "li"
#define SCNd32 "ld"
#define SCNu32 "lu"
#define SCNo32 "lo"
#define SCNx32 "lx"
#define SCNX32 "lX"

#define SCNi64 "lli"
#define SCNd64 "lld"
#define SCNu64 "llu"
#define SCNo64 "llo"
#define SCNx64 "llx"
#define SCNX64 "llX"

#define SCNiMAX "li"
#define SCNdMAX "ld"
#define SCNuMAX "lu"
#define SCNoMAX "lo"
#define SCNxMAX "lx"
#define SCNXMAX "lX"

#define SCNiPTR "li"
#define SCNdPTR "ld"
#define SCNuPTR "lu"
#define SCNoPTR "lo"
#define SCNxPTR "lx"
#define SCNXPTR "lX"

#elif __SIZE_WIDTH__ == 64

#define PRIi8 "i"
#define PRId8 "d"
#define PRIu8 "u"
#define PRIo8 "o"
#define PRIx8 "x"
#define PRIX8 "X"

#define PRIi16 "i"
#define PRId16 "d"
#define PRIu16 "u"
#define PRIo16 "o"
#define PRIx16 "x"
#define PRIX16 "X"

#define PRIi32 "i"
#define PRId32 "d"
#define PRIu32 "u"
#define PRIo32 "o"
#define PRIx32 "x"
#define PRIX32 "X"

#define PRIi64 "li"
#define PRId64 "ld"
#define PRIu64 "lu"
#define PRIo64 "lo"
#define PRIx64 "lx"
#define PRIX64 "lX"

#define PRIiMAX "li"
#define PRIdMAX "ld"
#define PRIuMAX "lu"
#define PRIoMAX "lo"
#define PRIxMAX "lx"
#define PRIXMAX "lX"

#define PRIiPTR "li"
#define PRIdPTR "ld"
#define PRIuPTR "lu"
#define PRIoPTR "lo"
#define PRIxPTR "lx"
#define PRIXPTR "lX"

#define SCNi8 "hhi"
#define SCNd8 "hhd"
#define SCNu8 "hhu"
#define SCNo8 "hho"
#define SCNx8 "hhx"
#define SCNX8 "hhX"

#define SCNi16 "hi"
#define SCNd16 "hd"
#define SCNu16 "hu"
#define SCNo16 "ho"
#define SCNx16 "hx"
#define SCNX16 "hX"

#define SCNi32 "i"
#define SCNd32 "d"
#define SCNu32 "u"
#define SCNo32 "o"
#define SCNx32 "x"
#define SCNX32 "X"

#define SCNi64 "li"
#define SCNd64 "ld"
#define SCNu64 "lu"
#define SCNo64 "lo"
#define SCNx64 "lx"
#define SCNX64 "lX"

#define SCNiMAX "li"
#define SCNdMAX "ld"
#define SCNuMAX "lu"
#define SCNoMAX "lo"
#define SCNxMAX "lx"
#define SCNXMAX "lX"

#define SCNiPTR "li"
#define SCNdPTR "ld"
#define SCNuPTR "lu"
#define SCNoPTR "lo"
#define SCNxPTR "lx"
#define SCNXPTR "lX"

#else
# error "unknown arch"
#endif

typedef struct
{
	intmax_t quot;
	intmax_t rem;
} imaxdiv_t;

intmax_t strtoimax(const char *nptr, char **endptr, int base);
uintmax_t strtoumax(const char *nptr, char **endptr, int base);
imaxdiv_t imaxdiv(intmax_t num, intmax_t dem);

#ifdef __cplusplus
}
#endif

#endif
