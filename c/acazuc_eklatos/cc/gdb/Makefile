include $(MAKEDIR)/env.mk

NAME    = gdb
VERSION = 16.2
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = 4002cb7f23f45c37c790536a13a720942ce4be0402d929c9085e92f10d480119
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

CONFIGURE_ARGS+= --target="$(HOST)" \
                 --prefix="$(PREFIX)" \

CONFIG_SUB = config.sub

include $(MAKEDIR)/cc.mk
