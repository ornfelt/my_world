SRC_FILES = $(addprefix $(SRC_PATH)/, $(SRC))
OBJ_FILES = $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(SRC)))
DEP_FILES = $(addprefix $(OBJ_PATH)/, $(addsuffix .d, $(SRC)))

$(OBJ_PATH)/%.c.o: $(SRC_PATH)/%.c
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) "CC $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$(OBJ_PATH)/$*.c.o" \
	                             -MD -MP -MF "$(OBJ_PATH)/$*.c.d"

$(OBJ_PATH)/%.S.o: $(SRC_PATH)/%.S
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) "AS $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$(OBJ_PATH)/$*.S.o" \
	                             -MD -MP -MF "$(OBJ_PATH)/$*.S.d"

-include $(DEP_FILES)

.PRECIOUS: $(OBJ_PATH)/%.c.o $(OBJ_PATH)/%.S.o
