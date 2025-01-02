include $(MAKEDIR)/env.mk

all: $(BUILDDIR)/usr/bin/test

$(BUILDDIR)/usr/bin/test:
	@$(LN) -sf \[ $@

clean:
	@$(RM) -f $(BUILDDIR)/usr/bin/test

.PHONY: all clean
