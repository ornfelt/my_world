include $(MAKEDIR)/env.mk

all: $(BUILDDIR)/usr/bin/$(NAME)

$(BUILDDIR)/usr/bin/$(NAME): $(NAME)
	@$(CP) $< $@

clean:
	@$(RM) -f $(BUILDDIR)/usr/bin/$(NAME)

.PHONY: all clean
