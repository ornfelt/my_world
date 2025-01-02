include $(MAKEDIR)/env.mk

BIN = evdev

SRC = main.c

ifeq ($(KEYBOARD), azerty)
CPPFLAGS+= -DAZERTY
endif

include $(MAKEDIR)/mod.mk
