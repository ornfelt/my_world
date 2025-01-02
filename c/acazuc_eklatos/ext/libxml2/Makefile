NAME    = libxml2
VERSION = 2.12.3
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = 8c8f1092340a89ff32bc44ad5c9693aff9bc8a7a3e161bb239666e5d15ac9aaa
DIR     = $(NAME)-$(VERSION)
URL     = https://download.gnome.org/sources/libxml2/2.12/$(FILE)

CONFIGURE_ARGS = --without-python \
                 --without-zlib \

CONFIG_SUB = config.sub

include $(MAKEDIR)/ext_tar.mk
