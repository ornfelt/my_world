NAME    = glad
VERSION = 4.6
TAR     = $(NAME)-$(VERSION).zip
DIR     = $(NAME)-$(VERSION)

all: $(INSTALL_MARK)

$(EXTRACT_MARK):
	@unzip -u -d "$(DIR)" "$(JKL_DIST_DIR)/glad-$(VERSION).zip" > /dev/null

$(BUILD_MARK): $(EXTRACT_MARK)
	@cd "$(DIR)" && $(CC) $(CFLAGS) -I include -c src/glad.c -o src/glad.o
	@cd "$(DIR)" && $(AR) rc libglad.a src/glad.o

$(INSTALL_MARK): $(BUILD_MARK)
	@mkdir -p "$(INSTALL_DIR)/lib"
	@cp "$(DIR)/libglad.a" "$(INSTALL_DIR)/lib"
	@mkdir -p "$(INSTALL_DIR)/include"
	@cp -R "$(DIR)/include/"* "$(INSTALL_DIR)/include"

.PHONY: $(EXTRACT_MARK) $(BUILD_MARK) $(INSTALL_MARK)
