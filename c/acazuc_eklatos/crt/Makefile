include $(MAKEDIR)/env.mk

SRC = crt0.c \
      $(ARCH)/crti.S \
      $(ARCH)/crtn.S \

SRC_PATH = src

OBJ_PATH = $(OBJDIR)/crt/$(SRC_PATH)

CRT_DIR = $(BUILDDIR)/usr/lib

CRT_OBJECTS = crt0.o \
              crti.o \
              crtn.o \

CRT_OBJECTS:= $(addprefix $(CRT_DIR)/, $(CRT_OBJECTS))

CFLAGS += -fno-stack-protector

all: $(CRT_OBJECTS)

include $(MAKEDIR)/src.mk

$(CRT_DIR)/crt0.o: $(OBJ_PATH)/crt0.c.o
	@$(MKDIR) -p $(dir $@)
	@$(CP) "$<" "$@"

$(CRT_DIR)/crti.o: $(OBJ_PATH)/$(ARCH)/crti.S.o
	@$(MKDIR) -p $(dir $@)
	@$(CP) "$<" "$@"

$(CRT_DIR)/crtn.o: $(OBJ_PATH)/$(ARCH)/crtn.S.o
	@$(MKDIR) -p $(dir $@)
	@$(CP) "$<" "$@"

clean:
	@$(RM) -f $(OBJ_FILES)
	@$(RM) -f $(CRT_OBJECTS)

.PHONY: all clean
