include $(MAKEDIR)/env.mk

BIN_DIR = $(BUILDDIR)/usr/lib
INC_DIR = $(BUILDDIR)/usr/include
DBG_DIR = $(BUILDDIR)/usr/dbg

SO_FILE  = $(BIN_DIR)/$(BIN).so
A_FILE   = $(BIN_DIR)/$(BIN).a
DBG_FILE = $(DBG_DIR)/$(BIN).so.debug

SRC_PATH = src
OBJ_PATH = $(OBJDIR)/lib/$(BIN)/$(SRC_PATH)

LIBS_LINK = $(addprefix -l:, $(LIB))

CFLAGS+= -I include \

HEADERS = $(addprefix $(INC_DIR)/, $(HDR))

DEPLIB = $(addprefix $(BUILDDIR)/usr/lib/, $(LIB))

ifeq ($(VERSION_MAJOR),)
VERSION_MAJOR = 0
endif

ifeq ($(VERSION_MINOR),)
VERSION_MINOR = 0
endif

ifeq ($(VERSION_BUILD),)
VERSION_BUILD = 0
endif

ifeq ($(STANDALONE), 1)
LDFLAGS+= -nolibc
CFLAGS+= -fno-builtin
else
DEPLIB+= $(BUILDDIR)/usr/lib/libc.so
ifeq ($(WITH_LTO), yes)
CFLAGS+= -flto=auto
endif
endif

VERSION_SCRIPT = symbols.map

LDFLAGS+= -Wl,--version-script=$(VERSION_SCRIPT) \
          -Wl,--unresolved-symbols=report-all \
          -shared \
          -Wl,-soname -Wl,$(notdir $@).$(VERSION_MAJOR) \

ifeq ($(WITH_STRIP),yes)
all: $(DBG_FILE)
else
all: $(SO_FILE) $(A_FILE)
endif

all: $(HEADERS) $(A_FILE)

include $(MAKEDIR)/man.mk
include $(MAKEDIR)/src.mk

$(SO_FILE): $(OBJ_FILES) $(DEPLIB) $(VERSION_SCRIPT)
	@$(MKDIR) -p "$(dir $@)"
	@$(ECHO) "LD $(notdir $@)"
	@$(CC) -o "$@.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD)" \
	       $(LDFLAGS) $(OBJ_FILES) $(LIBS_LINK) -lgcc
	@$(LN) -sf "$(notdir $@.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD))" "$@.$(VERSION_MAJOR)"
	@$(LN) -sf "$(notdir $@.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD))" "$@"

$(A_FILE): $(OBJ_FILES)
	@$(MKDIR) -p "$(dir $@)"
	@$(ECHO) "AR $(notdir $@)"
	@$(AR) rc "$@" $(OBJ_FILES)

$(DBG_FILE): $(SO_FILE)
	@$(MKDIR) -p "$(dir $@)"
	@$(OBJCOPY) --only-keep-debug "$<" "$@"
	@$(STRIP) --strip-debug --strip-unneeded "$<"
	@$(OBJCOPY) --add-gnu-debuglink="$@" "$<"
	@$(TOUCH) "$@" # make sure DBG_FILE is newer than SO_FILE

$(INC_DIR)/%.h: include/%.h
	@$(MKDIR) -p "$(dir $@)"
	@[ ! -f "$<" ] || $(CP) "$<" "$@"

clean:
	@$(RM) -fr $(OBJ_FILES)

.PHONY: all clean
