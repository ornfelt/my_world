$(JKL_DIST_DIR)/$(ARCHIVE):
	curl "$(URL)" --output "$@.tmp"
	@openssl dgst -sha256 -r "$@.tmp" | cut -d ' ' -f 1 > "$@.hash"
	@[ "`cat "$@.hash"`" = "$(HASH)" ] && \
	 mv "$@.tmp" "$@" || \
	 { \
	 	echo "invalid hash for $@\nexpected $(HASH)\ngot `cat "$@.hash"`" && \
	 	{ \
	 		rm "$@.tmp"; \
	 		rm "$@.hash"; \
	 		exit 1; \
	 	} \
	 }
