NAME    = ncurses
VERSION = 6.4
FILE    = $(NAME)-$(VERSION).tar.gz
HASH    = 6931283d9ac87c5073f30b6290c4c75f21632bb4fc3603ac8100812bed248159
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

INSTALL_TARGET = install

CONFIGURE_ARGS = --with-shared \
                 --with-normal \
                 --with-cxx-shared \

CONFIG_SUB = config.sub

include $(MAKEDIR)/ext_tar.mk
