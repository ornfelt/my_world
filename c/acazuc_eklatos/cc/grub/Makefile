include $(MAKEDIR)/env.mk

NAME    = grub
VERSION = 2.12
FILE    = $(NAME)-$(VERSION).tar.xz
HASH    = f3c97391f7c4eaa677a78e090c7e97e6dc47b16f655f04683ebd37bef7fe0faa
DIR     = $(NAME)-$(VERSION)
URL     = $(GNU_MIRROR_URL)/$(NAME)/$(FILE)

ifneq ($(filter $(ARCH), i386 amd64),)
PLATFORM = pc
else
ifneq ($(filter $(ARCH), mipsel mips64el),)
PLATFORM = qemu_mips
else
PLATFORM = efi
endif
endif

CONFIGURE_ARGS+= --target="$(HOST)" \
                 --prefix="$(ROOTDIR)/cc/$(ARCH)" \
                 --program-prefix="$(HOST)-" \
                 --with-platform="$(PLATFORM)" \
                 --disable-nls \
                 --disable-werror \

RUN_AUTORECONF = yes

CONFIG_SUB = build-aux/config.sub

all:
	@$(CP) "/usr/share/grub/unicode.pf2" "$(ROOTDIR)/cc/$(ARCH)/share/grub/unicode.pf2"

include $(MAKEDIR)/cc.mk
