NAME    = nghttp3
VERSION = 1.1.0
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://github.com/ngtcp2/nghttp3/releases/download/v$(VERSION)/$(TAR)
HASH    = f7ffcf21fb889e7d6a8422a3620deb52a8516364805ec3bd7ef296628ca595cb

CONFIGURE_ARGS+= --enable-lib-only

include ../mk/tar.mk
include ../mk/automake.mk
