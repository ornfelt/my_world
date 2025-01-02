include $(MAKEDIR)/env.mk

NAME    = binutils
VERSION = 2.43
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

CONFIGURE_ARGS+= --target="$(HOST)" \
                 --prefix="$(PREFIX)" \
                 --with-sysroot="$(SYSROOT)" \
                 --enable-default-hash-style=both \
                 --enable-compressed_debug_sections=all \
                 --disable-default-execstack \
                 --enable-bindnow \
                 --enable-shared \
                 --enable-relro \
                 --enable-lto \
                 --disable-nls \
                 --disable-werror \

CONFIG_SUB = config.sub

include $(MAKEDIR)/cc.mk
