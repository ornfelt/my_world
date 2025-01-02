NAME    = libsamplerate
VERSION = 0.2.2
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = 3258da280511d24b49d6b08615bbe824d0cacc9842b0e4caf11c52cf2b043893
DIR     = $(NAME)-$(VERSION)
URL     = https://github.com/libsndfile/libsamplerate/releases/download/$(VERSION)/$(FILE)

CONFIG_SUB = build-aux/config.sub

CONFIGURE_ARGS = --disable-sndfile \
                 --disable-alsa \
                 --disable-fftw \

include $(MAKEDIR)/ext_tar.mk
