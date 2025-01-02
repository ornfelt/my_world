include $(MAKEDIR)/env.mk

BIN_DIR = $(BUILDDIR)/usr/mod
DBG_DIR = $(BUILDDIR)/usr/dbg

KMOD_FILE = $(BIN_DIR)/$(BIN).kmod
DBG_FILE  = $(DBG_DIR)/$(BIN).kmod.debug

SRC_PATH = .
OBJ_PATH = $(OBJDIR)/mod/$(BIN)

CFLAGS+= -iquote . \
         -iquote $(ROOTDIR)/sys \
         -isystem $(ROOTDIR)/sys/include \
         -isystem $(OBJDIR)/sys/include \
         -fPIC \
         -fno-plt \

ifeq ($(WITH_LTO), yes)
CFLAGS+= -flto
endif

LDFLAGS+= -fPIC \
          -Wl,-z,noexecstack \
          -Wl,-z,relro,-z,now \
          -Wl,-ekmod \
          -rdynamic \
          -L $(BIN_DIR) \

VERSION_SCRIPT = symbols.map

CFLAGS+= $(addprefix -iquote $(ROOTDIR)/mod/, $(DEP))

LDFLAGS+= $(addprefix -l:, $(addsuffix .kmod, $(DEP))) \
          -Wl,--version-script=$(VERSION_SCRIPT) \

ifeq ($(ARCH), riscv32)
CFLAGS+= -mno-plt
endif

ifeq ($(ARCH), riscv64)
CFLAGS+= -mno-plt
endif

ifeq ($(WITH_STRIP),yes)
all: $(DBG_FILE)
else
all: $(KMOD_FILE)
endif

all: man

include $(MAKEDIR)/man.mk
include $(MAKEDIR)/src.mk

$(KMOD_FILE): $(OBJ_FILES) $(VERSION_SCRIPT)
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) "LD $(notdir $@)"
	@$(CC) -shared -o "$@" $(LDFLAGS) $(OBJ_FILES) -lgcc

$(DBG_FILE): $(KMOD_FILE)
	@$(MKDIR) -p "$(dir $@)"
	@$(OBJCOPY) --only-keep-debug "$<" "$@"
	@$(STRIP) --strip-debug --strip-unneeded "$<"
	@$(OBJCOPY) --add-gnu-debuglink="$@" "$<"
	@$(TOUCH) "$@" # make sure DBG_FILE is newer than BIN_FILE

clean:
	@$(RM) -fr $(OBJ_FILES)

.PHONY: all clean
