INSTALL_TARGET = install

EXTDIR  = $(ARCH)
WORKDIR = $(EXTDIR)/build
SRCDIR  = ../../$(DIR)

CONFIGURE_ENV = env PATH="$(PREFIX)/bin:$(PATH)"

include $(MAKEDIR)/ext_mk.mk
include $(MAKEDIR)/tar.mk
