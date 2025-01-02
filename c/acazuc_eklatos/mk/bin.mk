include $(MAKEDIR)/env.mk

BIN_DIR = $(BUILDDIR)/usr/bin
DBG_DIR = $(BUILDDIR)/usr/dbg

BIN_FILE = $(BIN_DIR)/$(BIN)
DBG_FILE = $(DBG_DIR)/$(BIN).debug

SRC_PATH = .
OBJ_PATH = $(OBJDIR)/bin/$(BIN)/$(SRC_PATH)

LIBS_LINK = $(addprefix -l:, $(LIB))

CFLAGS+= -I . \
         -fPIE \
         -fvisibility=hidden \

LDFLAGS+= -fwhole-program \
          -rdynamic \
          -pie \

ifeq ($(WITH_LTO), yes)
CFLAGS+= -flto
endif

ifeq ($(WITH_STRIP),yes)
all: $(DBG_FILE)
else
all: $(BIN_FILE)
endif

all: man

include $(MAKEDIR)/man.mk
include $(MAKEDIR)/src.mk

$(BIN_FILE): $(OBJ_FILES) $(addprefix $(BUILDDIR)/usr/lib/, $(LIB)) $(BUILDDIR)/usr/lib/libc.so
	@$(MKDIR) -p "$(dir $@)"
	@$(ECHO) "LD $(notdir $@)"
	@$(CC) -o "$@" $(LDFLAGS) $(OBJ_FILES) $(LIBS_LINK) $(OBJ_ADD) -lgcc

$(DBG_FILE): $(BIN_FILE)
	@$(MKDIR) -p "$(dir $@)"
	@$(OBJCOPY) --only-keep-debug "$<" "$@"
	@$(STRIP) --strip-debug --strip-unneeded "$<"
	@$(OBJCOPY) --add-gnu-debuglink="$@" "$<"
	@$(TOUCH) "$@" # make sure DBG_FILE is newer than BIN_FILE

clean:
	@$(RM) -fr $(OBJ_FILES)

.PHONY: all clean
