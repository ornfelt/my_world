DIRS = curl \
       flac \
       freetype \
       gfx \
       glad \
       glfw \
       jansson \
       jks \
       jkssl \
       libaudio \
       libformat \
       libgzip \
       libhttp \
       libnbt \
       libnet \
       libogg \
       libpng \
       librender \
       libsamplerate \
       libsql \
       libunicode \
       libvorbis \
       libwow \
       libxml2 \
       lua \
       nghttp2 \
       nghttp3 \
       openssl \
       portaudio \
       zlib \

export ENABLE_STATIC = yes
export ENABLE_SHARED = no

export EXTRACT_MARK       = .extract
export PRE_CONFIGURE_MARK = .pre_configure
export CONFIGURE_MARK     = .configure
export BUILD_MARK         = .build
export INSTALL_MARK       = .install

export BUILD_DIR = build

ifeq ($(INSTALL_DIR),)
$(error "no INSTALL_DIR given");
endif

export INSTALL_DIR

TARGET = host

ifeq ($(filter $(TARGET), linux_32 linux_64 windows_32 windows_64 host),)
$(error "no valid TARGET given")
endif

ifeq ($(TARGET), linux_32)
HOST = x86_64-linux-gnu
override CFLAGS  := $(CFLAGS) -m32
override CXXFLAGS:= $(CXXFLAGS) -m32
override LDFLAGS := $(LDFLAGS) -m32
endif

ifeq ($(TARGET), linux_64)
HOST = x86_64-linux-gnu
override CFLAGS  := $(CFLAGS) -m64
override CXXFLAGS:= $(CXXFLAGS) -m64
override LDFLAGS := $(LDFLAGS) -m64
endif

ifeq ($(TARGET), windows_32)
HOST = i686-w64-mingw32
override CFLAGS  := $(CFLAGS) -m32
override CXXFLAGS:= $(CXXFLAGS) -m32
override LDFLAGS := $(LDFLAGS) -m32
endif

ifeq ($(TARGET), windows_64)
HOST = x86_64-w64-mingw32
override CFLAGS  := $(CFLAGS) -m64
override CXXFLAGS:= $(CXXFLAGS) -m64
override LDFLAGS := $(LDFLAGS) -m64
endif

override CPPFLAGS:= $(CPPFLAGS) -I$(INSTALL_DIR)/include
override LDFLAGS:= $(LDFLAGS) -L$(INSTALL_DIR)/lib
override LDFLAGS:= $(LDFLAGS) -L$(INSTALL_DIR)/lib64

export TARGET
export HOST
export CFLAGS
export CXXFLAGS
export CPPFLAGS
export LDFLAGS

ifneq ($(HOST),)
CC      = $(HOST)-gcc
CXX     = $(HOST)-g++
AR      = $(HOST)-gcc-ar
RANLIB  = $(HOST)-ranlib
WINDRES = $(HOST)-windres
else
CC      = gcc
CXX     = g++
AR      = gcc-ar
RANLIB  = ranlib
WINDRES = windres
endif

export CC
export CXX
export AR
export RANLIB
export WINDRES

export PKG_CONFIG_LIBDIR=$(INSTALL_DIR)/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$(INSTALL_DIR)
export PKG_CONFIG_PATH=$(INSTALL_DIR)/lib:$(INSTALL_DIR)/lib64

JKL_DIST_DIR ?= .

export JKL_DIST_DIR

export MAKEOVERRIDES=

all: $(DIRS)

-include $(addsuffix /depend.mk, $(DIRS))

$(DIRS):
	@$(MAKE) -C $@ all

$(addprefix clean_,$(DIRS)):
	@$(MAKE) -C $(@:clean_%=%) clean

clean: $(addprefix clean_,$(DIRS))

.PHONY: all clean $(DIRS) $(addprefix clean_,$(DIRS))
