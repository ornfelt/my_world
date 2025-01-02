include $(MAKEDIR)/env.mk

DIRS = bash \
       binutils \
       bison \
       diffutils \
       emu_gb \
       emu_gba \
       emu_nds \
       emu_nes \
       findutils \
       gcc \
       gdb \
       gettext \
       gmp \
       grep \
       gzip \
       jka \
       jkgl \
       jkssh \
       jkssl \
       jkx \
       libiconv \
       libsamplerate \
       libxml2 \
       m4 \
       make \
       mpc \
       mpfr \
       ncurses \
       patch \
       retrosef \
       sed \
       tar \
       texinfo \
       xz \

ENABLED_DIRS = dummy

ifeq ($(WITH_RETRO_EMU), yes)
ENABLED_DIRS+= emu_gb \
               emu_gba \
               emu_nes \
               emu_nds \
               retrosef
endif

ifeq ($(WITH_JKSSL), yes)
ENABLED_DIRS+= jkssl
endif

ifeq ($(WITH_JKX), yes)
ENABLED_DIRS+= jkx
endif

ifeq ($(WITH_JKGL), yes)
ENABLED_DIRS+= jkgl
endif

ifeq ($(WITH_JKSSH), yes)
ENABLED_DIRS+= jkssh
endif

ifeq ($(WITH_JKA), yes)
ENABLED_DIRS+= jka
endif

# external contribs

ifeq ($(WITH_BINUTILS), yes)
ENABLED_DIRS+= binutils
endif

ifeq ($(WITH_GCC), yes)
ENABLED_DIRS+= gcc
endif

ifeq ($(WITH_M4), yes)
ENABLED_DIRS+= m4
endif

ifeq ($(WITH_NCURSES), yes)
ENABLED_DIRS+= ncurses
endif

ifeq ($(WITH_BASH), yes)
ENABLED_DIRS+= bash
endif

ifeq ($(WITH_DIFFUTILS), yes)
ENABLED_DIRS+= diffutils
endif

ifeq ($(WITH_MAKE), yes)
ENABLED_DIRS+= make
endif

ifeq ($(WITH_FINDUTILS), yes)
ENABLED_DIRS+= findutils
endif

ifeq ($(WITH_GREP), yes)
ENABLED_DIRS+= grep
endif

ifeq ($(WITH_PATCH), yes)
ENABLED_DIRS+= patch
endif

ifeq ($(WITH_SED), yes)
ENABLED_DIRS+= sed
endif

ifeq ($(WITH_TAR), yes)
ENABLED_DIRS+= tar
endif

ifeq ($(WITH_XZ), yes)
ENABLED_DIRS+= xz
endif

ifeq ($(WITH_BISON), yes)
ENABLED_DIRS+= bison
endif

ifeq ($(WITH_TEXINFO), yes)
ENABLED_DIRS+= texinfo
endif

ifeq ($(WITH_LIBXML2), yes)
ENABLED_DIRS+= libxml2
endif

ifeq ($(WITH_GZIP), yes)
ENABLED_DIRS+= gzip
endif

ifeq ($(WITH_GDB), yes)
ENABLED_DIRS+= gdb
endif

include $(MAKEDIR)/dir.mk

clean:
	@$(RM) -f $(ROOTDIR)/obj/$(ARCH)/ext/config.cache
