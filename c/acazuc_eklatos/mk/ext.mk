ifeq ($(INSTALL_TARGET),)
ifeq ($(WITH_STRIP),yes)
INSTALL_TARGET = install-strip
else
INSTALL_TARGET = install
endif
endif

INSTALL_TARGET:= DESTDIR="$(BUILDDIR)" $(INSTALL_TARGET)

CONFIGURE_ARGS+= --host="$(HOST)" \
                 --target="$(HOST)" \
                 --prefix=/usr \
                 --enable-static \
                 --enable-shared \
                 CFLAGS="$(CONFIGURE_CFLAGS)" \
                 CXXFLAGS="$(CONFIGURE_CXXFLAGS)" \
                 LDFLAGS="$(CONFIGURE_LDFLAGS)" \

CONFIGURE_CFLAGS   += -O2 -g $(FPU_FLAGS) $(HARDENING_CFLAGS)
CONFIGURE_CXXFLAGS += -O2 -g $(FPU_FLAGS) $(HARDENING_CXXFLAGS)
CONFIGURE_LDFLAGS  += $(HARDENING_LDFLAGS)

ifneq ($(DISABLE_CONFIGURE_CACHE), yes)
CONFIGURE_ARGS+= --cache-file="$(ROOTDIR)/obj/$(ARCH)/ext/config.cache"
endif

EXTDIR  = $(OBJDIR)/ext/$(NAME)
WORKDIR = $(EXTDIR)/build
SRCDIR  = $(ROOTDIR)/ext/$(NAME)/$(DIR)

include $(MAKEDIR)/ext_mk.mk
