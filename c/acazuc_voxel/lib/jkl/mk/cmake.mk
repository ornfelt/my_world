all: $(INSTALL_MARK)

ifneq ($(filter $(TARGET), linux_32 linux_64),)
SYSTEM_NAME = Linux
endif

ifneq ($(filter $(TARGET), windows_32 windows_64),)
SYSTEM_NAME = Windows
endif

$(PRE_CONFIGURE_MARK):

$(CONFIGURE_MARK): $(EXTRACT_MARK) $(PRE_CONFIGURE_MARK)
	@rm -rf "$(BUILD_DIR)"
	@mkdir -p "$(BUILD_DIR)"
	@cd "$(BUILD_DIR)" && CFLAGS="$(CFLAGS)" \
	                      CXXFLAGS="$(CXXFLAGS)" \
	                      CPPFLAGS="$(CPPFLAGS)" \
	                      LDFLAGS="$(LDFLAGS)" \
	                      cmake "../$(DIR)" \
	                      -DCMAKE_C_COMPILER="$(CC)" \
	                      -DCMAKE_CXX_COMPILER="$(CXX)" \
	                      -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)" \
	                      -DCMAKE_BUILD_TYPE=Release \
	                      -DCMAKE_SYSTEM_NAME=$(SYSTEM_NAME) \
	                      -DCMAKE_FIND_ROOT_PATH="$(INSTALL_DIR)" \
	                      $(CONFIGURE_ARGS)

$(BUILD_MARK): $(CONFIGURE_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" clean
	@$(MAKE) -C "$(BUILD_DIR)"

$(INSTALL_MARK): $(BUILD_MARK)
	@$(MAKE) -C "$(BUILD_DIR)" install
	@rm -rf "$(BUILD_DIR)"

.PHONY: $(PRE_CONFIGURE_MARK) $(CONFIGURE_MARK) $(BUILD_MARK) $(INSTALL_MARK)
