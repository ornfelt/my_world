NAME    = openssl
VERSION = 3.2.0
TAR     = $(NAME)-$(VERSION).tar.gz
DIR     = $(NAME)-$(VERSION)
URL     = https://www.openssl.org/source/$(TAR)
HASH    = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855

ifeq ($(TARGET), linux_32)
CONFIG = linux-x86
endif

ifeq ($(TARGET), linux_64)
CONFIG = linux-x86_64
endif

ifeq ($(TARGET), windows_32)
CONFIG = mingw
endif

ifeq ($(TARGET), windows_64)
CONFIG = mingw64
endif

ifeq ($(TARGET), host)
CONFIG = linux-x86_64
endif

ifneq ($(ENABLE_SHARED), yes)
NO_SHARED = -no-shared
endif

all: $(INSTALL_MARK)

$(CONFIGURE_MARK): $(EXTRACT_MARK)
	@rm -rf "$(BUILD_DIR)"
	@mkdir -p "$(BUILD_DIR)"
	@cd "$(BUILD_DIR)" && "../$(DIR)/Configure" \
	                      $(CONFIG) \
	                      $(NO_SHARED) \
	                      $(CROSS_PREFIX) \
	                      -no-docs \
	                      -no-tests \
	                      --prefix="$(INSTALL_DIR)" \
	                      $(CFLAGS) \
	                      $(CPPFLAGS) \
	                      $(LDFLAGS)

$(BUILD_MARK): $(CONFIGURE_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" clean
	@$(MAKE) -C "$(BUILD_DIR)"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" install
	@rm -rf "$(BUILD_DIR)"

.PHONY: $(CONFIGURE_MARK) $(BUILD_MARK) $(INSTALL_MARK)

include ../mk/tar.mk
