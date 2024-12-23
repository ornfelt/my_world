NAME    = glfw
VERSION = 3.3.9
ZIP     = $(NAME)-$(VERSION).zip
DIR     = $(NAME)-$(VERSION)
URL     = https://github.com/glfw/glfw/releases/download/$(VERSION)/$(ZIP)
HASH    = 55261410f8c3a9cc47ce8303468a90f40a653cd8f25fb968b12440624fb26d08

ifneq ($(HOST),)
CONFIGURE_ARGS+= -DCMAKE_TOOLCHAIN_FILE="CMake/$(HOST).cmake"
endif

CONFIGURE_ARGS+= -DBUILD_SHARED_LIBS=$(ENABLE_SHARED) \
                 -DGLFW_BUILD_EXAMPLES=OFF \
                 -DGLFW_BUILD_TESTS=OFF \
                 -DGLFW_BUILD_DOCS=OFF \

$(PRE_CONFIGURE_MARK): $(EXTRACT_MARK)
	@cp "files/x86_64-linux-gnu.cmake" "$(DIR)/CMake"

include ../mk/zip.mk
include ../mk/cmake.mk
