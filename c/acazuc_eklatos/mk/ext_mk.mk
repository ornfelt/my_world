EXTRACT_MARK     = $(EXTDIR)/.extract
AUTORECONF_MARK  = $(EXTDIR)/.autoreconf
PATCH_MARK       = $(EXTDIR)/.patch
CONFIGSUB_MARK   = $(EXTDIR)/.configsub
CONFIGGUESS_MARK = $(EXTDIR)/.configguess
CONFIGURE_MARK   = $(EXTDIR)/.configure
BUILD_MARK       = $(EXTDIR)/.build
INSTALL_MARK     = $(EXTDIR)/.install
PATCHES_FILES    = $(sort $(wildcard patches/*.patch))
PATCHES_MARKS    = $(patsubst patches/%.patch,$(EXTDIR)/.patch_%,$(PATCHES_FILES))

CLEAN_DIRS = $(WORKDIR)

ifneq ($(CLEAN_SRCDIR), 0)
CLEAN_DIRS+= $(DIR)
endif

all: $(INSTALL_MARK)

$(EXTRACT_MARK): patches

ifneq ($(CONFIG_SUB),)
$(CONFIGURE_MARK): $(CONFIGSUB_MARK)
endif

ifneq ($(CONFIG_GUESS),)
$(CONFIGURE_MARK): $(CONFIGGUESS_MARK)
endif

ifeq ($(RUN_AUTORECONF), yes)
$(CONFIGSUB_MARK): $(AUTORECONF_MARK)
$(CONFIGGUESS_MARK): $(AUTORECONF_MARK)
$(CONFIGURE_MARK): $(AUTORECONF_MARK)
endif

.NOTPARALLEL: $(PATCHES_MARKS)
$(EXTDIR)/.patch_%: patches/%.patch $(EXTRACT_MARK)
	@$(ECHO) "patch $*.patch"
	@$(PATCH) --dry-run --forward --batch --quiet -d "$(DIR)" -p0 < "$<"
	@$(PATCH) --forward --batch --quiet -d "$(DIR)" -p0 < "$<"
	@$(TOUCH) "$@"

$(PATCH_MARK): $(PATCHES_MARKS) $(EXTRACT_MARK)
	@$(TOUCH) "$@"

$(AUTORECONF_MARK): $(PATCH_MARK)
	@$(RM) -rf "$(WORKDIR)"
	@cd "$(DIR)" && { [ "$(RUN_AUTORECONF)" != "yes" ] || { autoreconf -fi ; } ; }
	@$(TOUCH) "$@"

$(CONFIGSUB_MARK): $(ROOTDIR)/ext/config.sub
	@$(CP) "$<" "$(DIR)/$(CONFIG_SUB)"
	@$(TOUCH) "$@"

$(CONFIGGUESS_MARK): $(ROOTDIR)/ext/config.guess
	@$(CP) "$<" "$(DIR)/$(CONFIG_GUESS)"
	@$(TOUCH) "$@"

$(CONFIGURE_MARK): $(PATCH_MARK)
	@$(MKDIR) -p "$(WORKDIR)"
	@cd "$(WORKDIR)" && $(CONFIGURE_ENV) $(SH) "$(SRCDIR)/configure" $(CONFIGURE_ARGS)
	@$(TOUCH) "$@"

$(BUILD_MARK): $(CONFIGURE_MARK)
	@$(MAKE) -C "$(WORKDIR)"
	@$(TOUCH) "$@"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(WORKDIR)" $(INSTALL_TARGET)
	@$(EXTRA_CMD)
	@$(TOUCH) "$@"

clean:
	@$(RM) -f "$(EXTRACT_MARK)" "$(AUTORECONF_MARK)" $(PATCHES_MARKS) "$(PATCH_MARK)" "$(CONFIGSUB_MARK)" "$(CONFIGGUESS_MARK)" "$(CONFIGURE_MARK)" "$(BUILD_MARK)" "$(INSTALL_MARK)"
	@$(RM) -fr $(CLEAN_DIRS)

.PHONY: all clean
