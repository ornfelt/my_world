include $(MAKEDIR)/env.mk

NAME    = binutils
VERSION = 2.44
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = ce2017e059d63e67ddb9240e9d4ec49c2893605035cd60e92ad53177f4377237
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
