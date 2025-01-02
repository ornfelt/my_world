include $(MAKEDIR)/env.mk

ifeq ($(ENABLED_DIRS),)
ENABLED_DIRS = $(DIRS)
endif

all: $(ENABLED_DIRS)

-include $(addsuffix /depend.mk, $(DIRS))

$(DIRS):
	@$(MAKE) -C $@

dummy:

$(addprefix clean_,$(DIRS)):
	@$(MAKE) -C $(@:clean_%=%) clean

clean: $(addprefix clean_,$(DIRS))

.PHONY: all clean dummy $(DIRS) $(addprefix clean_,$(DIRS))
