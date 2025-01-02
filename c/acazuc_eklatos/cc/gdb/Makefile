include $(MAKEDIR)/env.mk

NAME    = gdb
VERSION = 15.1
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = 38254eacd4572134bca9c5a5aa4d4ca564cbbd30c369d881f733fb6b903354f2
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

CONFIGURE_ARGS+= --target="$(HOST)" \
                 --prefix="$(PREFIX)" \

CONFIG_SUB = config.sub

include $(MAKEDIR)/cc.mk
