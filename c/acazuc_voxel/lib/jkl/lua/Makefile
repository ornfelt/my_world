NAME    = lua
VERSION = 5.1.5
TAR     = $(NAME)-$(VERSION).tar.gz
DIR     = $(NAME)-$(VERSION)
URL     = https://www.lua.org/ftp/$(TAR)
HASH    = 2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333

ifeq ($(ENABLE_SHARED), yes)
CFLAGS+= -fPIC
SED_CMD = sed -i 's/\$(RANLIB) $@/$(CC) -shared -ldl -Wl,-soname,liblua$R.so -o liblua$R.so $? -lm $(MYLDFLAGS)/' "$(DIR)/src/Makefile"
else
SED_CMD = true
endif

ifneq ($(filter $(TARGET), linux_32 linux_64 host),)
TO_BIN = lua luac
endif

ifneq ($(filter $(TARGET), windows_32 windows_64),)
TO_BIN = lua.exe luac.exe
endif

all: $(INSTALL_MARK)

$(BUILD_MARK): $(EXTRACT_MARK)
	@$(MAKE) -C "$(DIR)" clean
	@$(SED_CMD)
	@$(MAKE) -C "$(DIR)/src" all \
	                         CC="$(CC)" \
	                         AR="$(AR) rcu" \
	                         RANLIB="$(RANLIB)" \
	                         MYCFLAGS="$(CFLAGS)" \
	                         MYLDFLAGS="$(LDFLAGS)"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(DIR)" install INSTALL_TOP="$(INSTALL_DIR)" TO_BIN="$(TO_BIN)"
	@rm -rf "$(DIR)"

.PHONY: $(EXTRACT_MARK) $(BUILD_MARK) $(INSTALL_MARK)

include ../mk/tar.mk
