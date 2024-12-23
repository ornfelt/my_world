NAME    = zlib
VERSION = 1.3
TAR     = $(NAME)-$(VERSION).tar.xz
DIR     = $(NAME)-$(VERSION)
URL     = https://www.zlib.net/$(TAR)
HASH    = 8a9ba2898e1d0d774eca6ba5b4627a11e5588ba85c8851336eb38de4683050a7

ifeq ($(ENABLE_STATIC), yes)
CONFIGURE_ARGS+= --static
endif

ifeq ($(ENABLE_SHARED), yes)
CONFIGURE_ARGS+= --shared
endif

all: $(INSTALL_MARK)

$(CONFIGURE_MARK): $(EXTRACT_MARK)
	@cd "$(DIR)" && CC="$(CC)" \
	                AR="$(AR)" \
	                RANLIB="$(RANLIB)" \
	                CFLAGS="$(CFLAGS)" \
	                CPPFLAGS="$(CPPFLAGS)" \
	                LDFLAGS="$(LDFLAGS)" \
	                ./configure \
	                --prefix="$(INSTALL_DIR)" \
	                $(CONFIGURE_ARGS)

$(BUILD_MARK): $(CONFIGURE_MARK)
	@$(MAKE) -C "$(DIR)" clean
	@$(MAKE) -C "$(DIR)"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(DIR)" install

.PHONY: $(CONFIGURE_MARK) $(BUILD_MARK) $(INSTALL_MARK)

include ../mk/tar.mk
