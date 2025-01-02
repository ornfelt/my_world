include $(MAKEDIR)/env.mk

NAME    = gcc
VERSION = 14.2.0
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = a7b39bc69cbf9e25826c5a60ab26477001f7c08d85cec04bc0e29cabed6f3cc9
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(NAME)-$(VERSION)/$(FILE)

ifeq ($(ARCH), i386)
CONFIGURE_ARGS = --with-arch=i686 \
                 --with-tune=i686
endif

ifeq ($(ARCH), amd64)
CONFIGURE_ARGS = --with-arch=x86-64
endif

ifeq ($(ARCH), arm)
CONFIGURE_ARGS = --with-cpu=cortex-a15 \
                 --with-float=hard \
                 --with-fpu=neon
endif

ifeq ($(ARCH), aarch64)
CONFIGURE_ARGS = --with-cpu=cortex-a72
endif

ifeq ($(ARCH), riscv32)
CONFIGURE_ARGS = --with-arch=rv32gc
endif

ifeq ($(ARCH), riscv64)
CONFIGURE_ARGS = --with-arch=rv64gc
endif

ifeq ($(ARCH), mipsel)
CONFIGURE_ARGS = --with-abi=32 \
                 --with-arch=mips32r2
endif

ifeq ($(ARCH), mips64el)
CONFIGURE_ARGS = --with-abi=64 \
                 --with-arch=mips64r2
endif

CONFIGURE_ARGS+= --target="$(HOST)" \
                 --prefix="$(PREFIX)" \
                 --with-sysroot="$(SYSROOT)" \
                 --enable-languages=c,c++,lto \
                 --enable-host-bind-now \
                 --enable-threads=posix \
                 --enable-default-pie \
                 --enable-default-ssp \
                 --enable-host-shared \
                 --enable-host-pie \
                 --enable-lto \
                 --disable-multilib \
                 --disable-shared \
                 --disable-libssp \
                 --disable-nls \

CONFIG_SUB = config.sub

all:
	@$(LN) -sf $(BUILDDIR)/usr/lib/crti.o \
	           $(PREFIX)/lib/gcc/$(HOST)/$(VERSION)/crti.o
	@$(LN) -sf $(BUILDDIR)/usr/lib/crtn.o \
	           $(PREFIX)/lib/gcc/$(HOST)/$(VERSION)/crtn.o

include $(MAKEDIR)/cc.mk
