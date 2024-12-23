DIR = $(NAME)

$(EXTRACT_MARK):
	@cd "$(DIR)" && git clean -dfX
	@cd "$(DIR)" && autoreconf -i

.PHONY: $(EXTRACT_MARK)
