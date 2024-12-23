NAME    = freetype
VERSION = 2.13.2
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://download.savannah.gnu.org/releases/freetype/$(TAR)
HASH    = 12991c4e55c506dd7f9b765933e62fd2be2e06d421505d7950a132e4f1bb484d

CONFIGURE_ARGS+= -DFT_DISABLE_BZIP2=TRUE \
                 -DFT_REQUIRE_ZLIB=TRUE \
                 -DFT_DISABLE_HARFBUZZ=TRUE \
                 -DFT_REQUIRE_PNG=TRUE \
                 -DFT_DISABLE_BROTLI=TRUE

include ../mk/tar.mk
include ../mk/cmake.mk
