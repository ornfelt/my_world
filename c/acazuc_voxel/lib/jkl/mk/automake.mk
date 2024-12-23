all: $(INSTALL_MARK)

$(CONFIGURE_MARK): $(EXTRACT_MARK)
	@rm -rf "$(BUILD_DIR)"
	@mkdir -p "$(BUILD_DIR)"
	@cd "$(BUILD_DIR)" && "../$(DIR)/configure" \
	                       --enable-static=$(ENABLE_STATIC) \
	                       --enable-shared=$(ENABLE_SHARED) \
	                       --host=$(HOST) \
	                       --prefix="$(INSTALL_DIR)" \
	                       $(CONFIGURE_ARGS) \
	                       CFLAGS="$(CFLAGS)" \
	                       CXXFLAGS="$(CXXFLAGS)" \
	                       CPPFLAGS="$(CPPFLAGS)" \
	                       LDFLAGS="$(LDFLAGS)"

$(BUILD_MARK): $(CONFIGURE_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" clean
	@$(MAKE) -C "$(BUILD_DIR)"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" install
	@rm -rf "$(BUILD_DIR)"

.PHONY: $(CONFIGURE_MARK) $(BUILD_MARK) $(INSTALL_MARK)
