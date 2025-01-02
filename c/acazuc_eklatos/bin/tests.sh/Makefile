include $(MAKEDIR)/env.mk

SCRIPTS = case.sh \
          if.sh \
          test.sh \
          for.sh \
          group.sh \
          fn.sh \
          args.sh \

DST_SCRIPTS = $(addprefix $(BUILDDIR)/usr/bin/tests_, $(SCRIPTS))

all: $(DST_SCRIPTS)

$(BUILDDIR)/usr/bin/tests_%:%
	@$(CP) $^ $@

clean:
	@$(RM) -f $(DST_SCRIPTS)

.PHONY: all clean
