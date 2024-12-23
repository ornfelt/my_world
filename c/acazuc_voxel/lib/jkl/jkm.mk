ifeq ($(TARGET), windows_32)

NAME:= $(NAME).exe
HOST = i686-w64-mingw32-
JKL_TARGET = windows_32
JKM_OS = windows

else
ifeq ($(TARGET), windows_64)

NAME:= $(NAME).exe
HOST = x86_64-w64-mingw32-
JKL_TARGET = windows_64
JKM_OS = windows

else
ifeq ($(TARGET), linux_32)

JKL_TARGET = linux_32
JKM_OS = linux

else
ifeq ($(TARGET), linux_64)

JKL_TARGET = linux_64
JKM_OS = linux

else
ifeq ($(TARGET), wasm)

HOST = wasm-unknown-emscripten-
JKL_TARGET = wasm
JKM_OS = wasm
CFLAGS += -s WASM=1

else

$(error Must define TARGET as either windows_32, windows_64, linux_32, linux_64 or wasm)

endif
endif
endif
endif
endif

CC = $(HOST)gcc
CXX = $(HOST)g++

LDFLAGS+= -fwhole-program

JKL_CFLAGS+= -pipe
JKL_CFLAGS+= -g

CFLAGS+= -Wall
CFLAGS+= -Wextra
CFLAGS+= -Wshadow
CFLAGS+= -Wunused
CFLAGS+= -pipe
CFLAGS+= -g
CFLAGS+= -D__STDC_FORMAT_MACROS
CFLAGS+= -Wno-cpp

CXXFLAGS+= -Wall
CXXFLAGS+= -Wextra
CXXFLAGS+= -Wunused
CXXFLAGS+= -pipe
CXXFLAGS+= -g
CXXFLAGS+= -D__STDC_FORMAT_MACRO
CXXFLAGS+= -fpermissive

#XXX: Workaround
CFLAGS+= -Wno-missing-field-initializers
CFLAGS+= -Wno-format-truncation
CXXFLAGS+= -Wno-missing-field-initializers
CXXFLAGS+= -Wno-format-truncation

ifeq ($(JKM_OS), linux)
LDFLAGS+= -rdynamic
endif

ifeq ($(JKM_OS), windows)
CFLAGS+= -Wl,-subsystem,windows
CXXFLAGS+= -Wl,-subsystem,windows
LDFLAGS+= -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
#LDFLAGS+= -static
endif

ifeq ($(WITH_OPTIMIZATIONS), YES)
OPTIM_FLAGS+= -Ofast \
              -march=native
DIR_SUFFIX:= $(DIR_SUFFIX)_OPTIMIZE
else
OPTIM_FLAGS+= -O0
endif

ifeq ($(WITH_ASAN), YES)
SANITIZER_FLAGS+= -fsanitize=address
DIR_SUFFIX:= $(DIR_SUFFIX)_ASAN
endif

ifeq ($(WITH_UBSAN), YES)
SANITIZER_FLAGS+= -fsanitize=undefined
DIR_SUFFIX:= $(DIR_SUFFIX)_UBSAN
endif

ifeq ($(WITH_TSAN), YES)
SANITIZER_FLAGS+= -fsanitize=thread
DIR_SUFFIX:= $(DIR_SUFFIX)_TSAN
endif

ifeq ($(WITH_LSAN), YES)
SANITIZER_FLAGS+= -fsanitize=leak
DIR_SUFFIX:= $(DIR_SUFFIX)_LSAN
endif

ifeq ($(WITH_LTO), YES)
LTO_FLAGS+= -flto=auto
DIR_SUFFIX:= $(DIR_SUFFIX)_LTO
endif

ifeq ($(WITH_ASSERT), YES)
DIR_SUFFIX:= $(DIR_SUFFIX)_ASSERT
else
CPPFLAGS+= -DNDEBUG
JKL_CPPFLAGS+= -DNDEBUG
endif

CFLAGS+= $(OPTIM_FLAGS) $(SANITIZER_FLAGS) $(LTO_FLAGS)
CXXFLAGS+= $(OPTIM_FLAGS) $(SANITIZER_FLAGS) $(LTO_FLAGS)
JKL_CFLAGS+= $(OPTIM_FLAGS) $(SANITIZER_FLAGS) $(LTO_FLAGS)
JKL_CXXFLAGS+= $(OPTIM_FLAGS) $(SANITIZER_FLAGS) $(LTO_FLAGS)
LDFLAGS+= $(SANITIZER_FLAGS) $(LTO_FLAGS)
JKL_LDFLAGS+= $(SANITIZER_FLAGS) $(LTO_FLAGS)

LIB_DIR = lib/build_$(TARGET)$(DIR_SUFFIX)

INCLUDES+= -I src
INCLUDES+= -I $(LIB_DIR)/include

LDFLAGS+= -L $(LIB_DIR)/lib
LDFLAGS+= -L $(LIB_DIR)/lib64

ifeq ($(WITH_LIBFORMAT), yes)
JKL_LIBS+= libformat
LIBRARY+= -l:libformat.a
endif

ifeq ($(WITH_LIBRENDER), yes)
JKL_LIBS+= librender
LIBRARY+= -l:librender.a
endif

ifeq ($(WITH_FREETYPE), yes)
ifeq ($(JKM_OS), wasm)
CFLAGS+= -s USE_FREETYPE=1
else
JKL_LIBS+= freetype
PKG_CONFIG_LIBRARIES+= freetype2
endif
endif

ifeq ($(WITH_LIBPNG), yes)
ifeq ($(JKM_OS), wasm)
CFLAGS+= -s USE_LIBPNG=1
else
JKL_LIBS+= libpng
PKG_CONFIG_LIBRARIES+= libpng16
endif
endif

ifeq ($(WITH_LIBNBT), yes)
JKL_LIBS+= libnbt
PKG_CONFIG_LIBRARIES+= libnbt
endif

ifeq ($(WITH_ZLIB), yes)
ifeq ($(JKM_OS), wasm)
CFLAGS+= -s USE_ZLIB=1
else
JKL_LIBS+= zlib
PKG_CONFIG_LIBRARIES+= zlib
endif
endif

ifeq ($(WITH_GFX), yes)
JKL_LIBS+= gfx
PKG_CONFIG_LIBRARIES+= libgfx
endif

ifeq ($(WITH_GLFW), yes)
ifeq ($(JKM_OS), wasm)
CFLAGS+= -s USE_GLFW=1
else
JKL_LIBS+= glfw
endif
LIBRARY+= -l:libglfw3.a
ifeq ($(JKM_OS), linux)
SYSTEM_LIBRARY+= -ldl
SYSTEM_LIBRARY+= -lGL
SYSTEM_LIBRARY+= -lX11
SYSTEM_LIBRARY+= -lXcursor
SYSTEM_LIBRARY+= -lXi
SYSTEM_LIBRARY+= -lvulkan
endif
ifeq ($(JKM_OS), windows)
SYSTEM_LIBRARY+= -lopengl32
SYSTEM_LIBRARY+= -lgdi32
endif
endif

ifeq ($(WITH_GLAD), yes)
JKL_LIBS+= glad
LIBRARY+= -l:libglad.a
endif

ifeq ($(WITH_LIBXML2), yes)
JKL_LIBS+= libxml2
PKG_CONFIG_LIBRARIES+= libxml-2.0
CPPFLAGS+= -DLIBXML_STATIC #avoid libxml XMLPUBFUN =  __declspec(dllimport)
endif

ifeq ($(WITH_LIBWOW), yes)
JKL_LIBS+= libwow
PKG_CONFIG_LIBRARIES+= libwow
endif

ifeq ($(WITH_JKS), yes)
JKL_LIBS+= jks
PKG_CONFIG_LIBRARIES+= libjks
endif

ifeq ($(WITH_LUA), yes)
JKL_LIBS+= lua
LIBRARY+= -l:liblua.a
endif

ifeq ($(WITH_LIBAUDIO), yes)
JKL_LIBS+= libaudio
LIBRARY+= -l:libaudio.a
endif

ifeq ($(WITH_PORTAUDIO), yes)
JKL_LIBS+= portaudio
PKG_CONFIG_LIBRARIES+= portaudio-2.0
ifeq ($(JKM_OS), windows)
SYSTEM_LIBRARY+= -lsetupapi
endif
endif

ifeq ($(WITH_LIBSAMPLERATE), yes)
JKL_LIBS+= libsamplerate
PKG_CONFIG_LIBRARIES+= samplerate
endif

ifeq ($(WITH_JKSSL), yes)
JKL_LIBS+= jkssl
PKG_CONFIG_LIBRARIES+= libjkssl
endif
ifeq ($(JKM_OS), windows)
SYSTEM_LIBRARY+= -lws2_32
SYSTEM_LIBRARY+= -lcrypt32
SYSTEM_LIBRARY+= -lwinmm
SYSTEM_LIBRARY+= -lole32
endif

ifeq ($(WITH_LIBVORBIS), yes)
JKL_LIBS+= libvorbis
PKG_CONFIG_LIBRARIES+= vorbisenc vorbisfile vorbis
endif

ifeq ($(WITH_LIBOGG), yes)
JKL_LIBS+= libogg
PKG_CONFIG_LIBRARIES+= ogg
endif

ifeq ($(WITH_CURL), yes)
JKL_LIBS+= curl
PKG_CONFIG_LIBRARIES+= libcurl
endif

ifeq ($(WITH_OPENSSL), yes)
JKL_LIBS+= openssl
PKG_CONFIG_LIBRARIES+= libcrypto
endif

ifeq ($(WITH_LIBGZIP), yes)
JKL_LIBS+= libgzip
LIBRARY+= -l:libgzip.a
endif

ifeq ($(WITH_LIBNET), yes)
JKL_LIBS+= libnet
LIBRARY+= -l:libnet.a
endif

ifeq ($(WITH_LIBUNICODE), yes)
JKL_LIBS+= libunicode
endif

ifeq ($(WITH_JANSSON), yes)
JKL_LIBS+= jansson
PKG_CONFIG_LIBRARIES+= jansson
endif

PKG_CONFIG_CFLAGS = $(shell env PKG_CONFIG_LIBDIR="$(LIB_DIR)/lib/pkgconfig" PKG_CONFIG_PATH="$(LIB_DIR)/lib64/pkgconfig" pkg-config --cflags $(PKG_CONFIG_LIBRARIES))
PKG_CONFIG_LIB_DIR = $(shell env PKG_CONFIG_LIBDIR="$(LIB_DIR)/lib/pkgconfig" PKG_CONFIG_PATH="$(LIB_DIR)/lib64/pkgconfig" pkg-config  --libs-only-L $(PKG_CONFIG_LIBRARIES))
PKG_CONFIG_LIBS = $(shell env PKG_CONFIG_LIBDIR="$(LIB_DIR)/lib/pkgconfig" PKG_CONFIG_PATH="$(LIB_DIR)/lib64/pkgconfig" pkg-config --libs-only-l $(PKG_CONFIG_LIBRARIES))

CFLAGS+= $(PKG_CONFIG_CFLAGS)
CXXFLAGS+= $(PKG_CONFIG_CFLAGS)
LDFLAGS+= $(PKG_CONFIG_LIB_DIR)
LIBRARY+= $(PKG_CONFIG_LIBS)

LIBRARY+= $(SYSTEM_LIBRARY)
LIBRARY+= -lpthread
LIBRARY+= -lm
LIBRARY+= -static-libgcc -static-libstdc++

SRCS_PATH = src

SRCS+= $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))

OBJS_PATH = obj

OBJS_NAME = $(SRCS_NAME:.c=.o)
OBJS_NAME:= $(OBJS_NAME:.cpp=.opp)

OBJS+= $(addprefix $(OBJS_PATH)/, $(OBJS_NAME))

DEPS_PATH = obj

DEPS_NAME = $(SRCS_NAME:.c=.d)
DEPS_NAME:= $(DEPS_NAME:.cpp=.dpp)

DEPS+= $(addprefix $(DEPS_PATH)/, $(DEPS_NAME))

all: $(NAME)

$(NAME): $(OBJS)
	@echo "LD $(NAME)"
	@$(CXX) $(LDFLAGS) -o $(NAME) $(OBJS) $(LIBRARY)

$(OBJS_PATH)/%.o: $(SRCS_PATH)/%.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -std=gnu17 $(CPPFLAGS) -o $(OBJS_PATH)/$*.o \
	                 -c $< $(INCLUDES) \
	                 -MD -MP -MF $(DEPS_PATH)/$*.d

$(OBJS_PATH)/%.opp: $(SRCS_PATH)/%.cpp
	@mkdir -p $(dir $@)
	@echo "CXX $<"
	@$(CXX) $(CXXFLAGS) -std=gnu++17 $(CPPFLAGS) -o $(OBJS_PATH)/$*.opp \
	                    -c $< $(INCLUDES) \
	                    -MD -MP -MF $(DEPS_PATH)/$*.dpp

-include $(DEPS)

clean:
	@rm -f $(OBJS)
	@rm -f $(NAME)

size:
	@wc `find -L $(SRCS_PATH) -type f \( -name \*.cpp -o -name \*.c -o -name \*.h \)` | tail -n 1

objsize:
	@wc -c `find $(OBJS_PATH) -type f` | tail -n 1

lib:
	@$(MAKE) -C lib/jkl $(JKL_LIBS) CFLAGS="$(JKL_CFLAGS)" CXXFLAGS="$(JKL_CXXFLAGS)" CPPFLAGS="$(JKL_CPPFLAGS)" LDFLAGS="$(JKL_LDFLAGS)" TARGET="$(JKL_TARGET)" ENABLE_STATIC=yes ENABLE_SHARED=no INSTALL_DIR="$(PWD)/$(LIB_DIR)"

.PHONY: lib size objsize clean
