include $(MAKEDIR)/env.mk

FILES = passwd \
        group \
        rc \
        hosts \
        services \
        protocols \
        resolv.conf \
        networks \

all: $(addprefix $(BUILDDIR)/etc/, $(FILES))

$(BUILDDIR)/etc/%: %
	@$(MKDIR) -p "$(dir $@)"
	@$(CP) $< $@

clean:

.PHONY: all clean
