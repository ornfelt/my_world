BIN = tests

SRC = main.c \
      string.c \
      ctype.c \
      pthread.c \
      strto.c \
      glob.c \
      wcstring.c \
      misc.c \

LIB = libm.so \
      libdl.so \
      libz.so \
      libfetch.so \
      libpthread.so \
      libelf.so \
      ld.so \

FLAGS= -Wno-stringop-truncation \
       -fstack-protector-all \
       -Wno-array-bounds \
       -Wno-stringop-overflow \
       -fno-builtin \
       -fno-delete-null-pointer-checks \

CFLAGS+= $(FLAGS)
LDFLAGS+= $(FLAGS)

include $(MAKEDIR)/bin.mk
