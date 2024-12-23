NAME    = curl
VERSION = 8.5.0
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://curl.se/download/$(TAR)
HASH    = 42ab8db9e20d8290a3b633e7fbb3cec15db34df65fd1015ef8ac1e4723750eeb

ifneq ($(filter $(TARGET), linux_32 linux_64 host),)
CONFIGURE_ARGS+= --with-openssl
endif

ifneq ($(filter $(TARGET), windows_32 windows_64),)
CONFIGURE_ARGS+= --with-schannel
endif

# XXX we must find a better workaround
ifeq ($(ENABLE_STATIC), yes)
CPPFLAGS+= -DNGHTTP2_STATICLIB
endif

CONFIGURE_ARGS+= --enable-debug \
                 --enable-optimize \
                 --enable-warnings \
                 --disable-rt \
                 --enable-http \
                 --disable-telnet \
                 --disable-tftp \
                 --disable-pop3 \
                 --disable-imap \
                 --disable-smtp \
                 --enable-ftp \
                 --disable-rtsp \
                 --disable-gopher \
                 --disable-dict \
                 --disable-smb \
                 --disable-ldap \
                 --enable-ipv6 \
                 --disable-threaded-resolver \
                 --disable-pthreads \
                 --without-brotli \
                 --without-zstd \
                 --with-zlib \
                 --without-libpsl \
                 --without-librtmp \
                 --without-winidn \
                 --without-libidn2 \
                 --with-nghttp2 \
                 --with-nghttp3 \

include ../mk/tar.mk
include ../mk/automake.mk
