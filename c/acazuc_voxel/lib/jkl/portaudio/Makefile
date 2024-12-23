NAME    = pa
VERSION = stable_v190700_20210406
TAR     = $(NAME)_$(VERSION).tgz
DIR     = portaudio
URL     = http://files.portaudio.com/archives/$(TAR)
HASH    = f5a21d7dcd6ee84397446fa1fa1a0675bb2e8a4a6dceb4305a8404698d8d1513

ifneq ($(filter $(TARGET), linux_32 linux_64),)
CONFIGURE_ARGS+= -DPA_USE_ALSA=ON -DPA_USE_JACK=OFF -DPA_USE_OSS=OFF
endif

ifneq ($(filter $(TARGET), windows_32 windows_64),)
CONFIGURE_ARGS+= -DPA_USE_DS=ON
endif

ifeq ($(ENABLE_SHARED), yes)
CONFIGURE_ARGS+= -DPA_BUILD_SHARED=ON
else
CONFIGURE_ARGS+= -DPA_BUILD_SHARED=OFF
endif

ifeq ($(ENABLE_STATIC), yes)
CONFIGURE_ARGS+= -DPA_BUILD_STATIC=ON
else
CONFIGURE_ARGS+= -DPA_BUILD_STATIC=OFF
endif

CONFIGURE_ARGS+= -DPA_BUILD_EXAMPLES=OFF

include ../mk/tar.mk
include ../mk/cmake.mk
