include $(MAKEDIR)/env.mk

all: usr.tar etc.tar

%.tar: %
	@$(ECHO) "TAR $@"
	@cd "$(ARCH)/$<" && $(TAR) cf "../../$@" -H ustar *

clean:
	@$(RM) -rf $(ARCH)/usr
	@$(RM) -rf $(ARCH)/etc
	@$(RM) -f usr.tar
	@$(RM) -f etc.tar

.PHONY: all clean usr etc
