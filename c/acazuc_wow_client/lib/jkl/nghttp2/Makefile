NAME    = nghttp2
VERSION = 1.58.0
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://github.com/nghttp2/nghttp2/releases/download/v$(VERSION)/$(TAR)
HASH    = 4a68a3040da92fd9872c056d0f6b0cd60de8410de10b578f8ade9ecc14d297e0

CONFIGURE_ARGS+= --enable-lib-only

include ../mk/tar.mk
include ../mk/automake.mk
