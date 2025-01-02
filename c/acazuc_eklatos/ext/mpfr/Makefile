NAME    = mpfr
VERSION = 4.2.1
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = 277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

CONFIGURE_ARGS+= --disable-decimal-float

CONFIG_SUB = config.sub

include $(MAKEDIR)/ext_tar.mk
