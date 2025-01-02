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
CONFIGURE_ARGS = --with-cpu=cortex-a15
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

CONFIGURE_ARGS+= --with-build-sysroot="$(BUILDDIR)" \
                 --with-sysroot=/ \
                 --enable-languages=c,c++,jit,lto \
                 --enable-host-bind-now \
                 --enable-threads=posix \
                 --enable-default-pie \
                 --enable-default-ssp \
                 --enable-host-shared \
                 --enable-host-pie \
                 --enable-shared \
                 --enable-lto \
                 --disable-multilib \
                 --disable-libssp \
                 --disable-nls \

include $(MAKEDIR)/ext_tar.mk
