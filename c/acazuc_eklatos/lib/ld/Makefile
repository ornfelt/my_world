BIN = ld

SRC = elf.c \
      ld.c \
      tls.c \
      ../../libc/src/auxv/getauxval.c \
      ../../libc/src/ctype/isspace.c \
      ../../libc/src/ctype/isdigit.c \
      ../../libc/src/ctype/tolower.c \
      ../../libc/src/fcntl/open.c \
      ../../libc/src/mman/mmap.c \
      ../../libc/src/mman/mprotect.c \
      ../../libc/src/mman/munmap.c \
      ../../libc/src/signal/kill.c \
      ../../libc/src/signal/raise.c \
      ../../libc/src/signal/sigaction.c \
      ../../libc/src/signal/sigaddset.c \
      ../../libc/src/signal/sigemptyset.c \
      ../../libc/src/signal/signal.c \
      ../../libc/src/signal/sigprocmask.c \
      ../../libc/src/signal/sigreturn.c \
      ../../libc/src/stdio/_printf.c \
      ../../libc/src/stdio/_stdio.c \
      ../../libc/src/stdio/fclose.c \
      ../../libc/src/stdio/fcloseall.c \
      ../../libc/src/stdio/fflush.c \
      ../../libc/src/stdio/fflush_unlocked.c \
      ../../libc/src/stdio/flockfile.c \
      ../../libc/src/stdio/fopen.c \
      ../../libc/src/stdio/fprintf.c \
      ../../libc/src/stdio/fputc.c \
      ../../libc/src/stdio/funlockfile.c \
      ../../libc/src/stdio/fwrite.c \
      ../../libc/src/stdio/fwrite_unlocked.c \
      ../../libc/src/stdio/printf.c \
      ../../libc/src/stdio/putchar.c \
      ../../libc/src/stdio/puts.c \
      ../../libc/src/stdio/snprintf.c \
      ../../libc/src/stdio/vfprintf.c \
      ../../libc/src/stdio/vsnprintf.c \
      ../../libc/src/stdlib/_alloc.c \
      ../../libc/src/stdlib/_atexit.c \
      ../../libc/src/stdlib/_env.c \
      ../../libc/src/stdlib/_strto.c \
      ../../libc/src/stdlib/abort.c \
      ../../libc/src/stdlib/calloc.c \
      ../../libc/src/stdlib/exit.c \
      ../../libc/src/stdlib/free.c \
      ../../libc/src/stdlib/getenv.c \
      ../../libc/src/stdlib/getprogname.c \
      ../../libc/src/stdlib/malloc.c \
      ../../libc/src/stdlib/realloc.c \
      ../../libc/src/stdlib/strtoul.c \
      ../../libc/src/string/memcpy.c \
      ../../libc/src/string/memmove.c \
      ../../libc/src/string/memrchr.c \
      ../../libc/src/string/memset.c \
      ../../libc/src/string/strchr.c \
      ../../libc/src/string/strchrnul.c \
      ../../libc/src/string/strcmp.c \
      ../../libc/src/string/strcpy.c \
      ../../libc/src/string/strdup.c \
      ../../libc/src/string/strerror.c \
      ../../libc/src/string/strlen.c \
      ../../libc/src/string/strlcpy.c \
      ../../libc/src/string/strncmp.c \
      ../../libc/src/time/clock_gettime.c \
      ../../libc/src/unistd/_exit.c \
      ../../libc/src/unistd/close.c \
      ../../libc/src/unistd/exit_group.c \
      ../../libc/src/unistd/getpagesize.c \
      ../../libc/src/unistd/getpid.c \
      ../../libc/src/unistd/gettid.c \
      ../../libc/src/unistd/lseek.c \
      ../../libc/src/unistd/read.c \
      ../../libc/src/unistd/settls.c \
      ../../libc/src/unistd/write.c \
      ../../libc/src/wchar/wcrtomb.c \
      ../../libc/src/wchar/wcslen.c \
      ../../libc/src/wchar/wcsnrtombs.c \
      ../../libc/src/__libc_start_main.c \
      ../../libc/src/_chk.c \
      ../../libc/src/futex.c \

LDFLAGS+= $(BUILDDIR)/usr/lib/crt0.o \
          -nostartfiles

STANDALONE = 1

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

include $(MAKEDIR)/lib.mk
