include $(MAKEDIR)/env.mk

DIR = $(NAME)

CLEAN_SRCDIR = 0

CONFIG_SUB = build-aux/config.sub

include $(MAKEDIR)/ext.mk

$(EXTRACT_MARK): $(PATCHES_FILES)
	@cd "$(DIR)" && git clean -dfX && git checkout -- .
	@autoreconf -i "$(DIR)"
	@$(MKDIR) -p "$(EXTDIR)"
	@$(TOUCH) "$@"
