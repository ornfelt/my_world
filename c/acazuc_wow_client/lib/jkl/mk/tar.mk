ARCHIVE = $(TAR)

$(EXTRACT_MARK): $(JKL_DIST_DIR)/$(TAR)
	@rm -fr "$(DIR)"
	@tar xf "$<"
	@touch "$@"

include ../mk/dl.mk
