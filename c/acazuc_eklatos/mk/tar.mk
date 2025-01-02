$(FILE):
	@curl "$(URL)" --output "$@.tmp"
	@openssl dgst -sha256 -r "$@.tmp" | cut -d ' ' -f 1 > "$@.hash"
	@[ "`cat "$@.hash"`" = "$(HASH)" ] \
	 && $(MV) "$@.tmp" "$@" \
	 || { echo "invalid hash for $@\nexpected $(HASH)\ngot `cat "$@.hash"`" && exit 1; }

$(EXTRACT_MARK): $(FILE) $(PATCHES_FILES)
	@$(RM) -fr "$(DIR)"
	@$(TAR) xf "$<"
	@$(MKDIR) -p "$(EXTDIR)"
	@$(TOUCH) "$@"
