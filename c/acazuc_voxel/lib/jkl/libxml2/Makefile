NAME    = libxml2
VERSION = 2.12.3
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://download.gnome.org/sources/libxml2/2.12/$(TAR)
HASH    = 8c8f1092340a89ff32bc44ad5c9693aff9bc8a7a3e161bb239666e5d15ac9aaa

CONFIGURE_ARGS+= --without-debug \
                 --without-ftp \
                 --without-http \
                 --without-iconv \
                 --without-python \
                 --without-zlib \
                 --without-lzma

include ../mk/tar.mk
include ../mk/automake.mk
