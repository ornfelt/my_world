NAME    = jansson
VERSION = 2.14
TAR     = $(NAME)-$(VERSION).tar.gz
DIR     = $(NAME)-$(VERSION)
URL     = https://github.com/akheron/jansson/releases/download/v$(VERSION)/$(TAR)
HASH    = 5798d010e41cf8d76b66236cfb2f2543c8d082181d16bc3085ab49538d4b9929

CONFIGURE_ARGS+= -DBUILD_SHARED_LIBS=$(ENABLE_SHARED) \
                 -DJANSSON_WITHOUT_TESTS=ON \
                 -DJANSSON_EXAMPLES=OFF \
                 -DJANSSON_BUILD_DOCS=OFF

include ../mk/tar.mk
include ../mk/cmake.mk
