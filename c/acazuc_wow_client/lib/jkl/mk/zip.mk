ARCHIVE = $(ZIP)

$(EXTRACT_MARK): $(JKL_DIST_DIR)/$(ZIP)
	@rm -fr "$(DIR)"
	@unzip -qu "$<"
	@touch "$@"

include ../mk/dl.mk
