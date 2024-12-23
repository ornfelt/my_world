NAME = gfx

ifneq ($(filter $(TARGET), host),)
CONFIGURE_ARGS+= --enable-device-gl3 \
                 --enable-device-gl4 \
                 --enable-device-gles3 \
                 --enable-device-vk \
                 --enable-window-x11 \
                 --enable-window-glfw \
                 --enable-window-sdl
endif

ifneq ($(filter $(TARGET), linux_32 linux_64),)
CONFIGURE_ARGS+= --enable-device-gl3 \
                 --enable-device-gl4 \
                 --enable-device-gles3 \
                 --enable-device-vk \
                 --enable-window-x11
endif

ifneq ($(filter $(TARGET), windows_32 windows_64),)
CONFIGURE_ARGS+= --enable-device-gl3 \
                 --enable-device-gl4 \
                 --enable-device-gles3 \
                 --enable-device-d3d11 \
                 --enable-window-win32
endif

include ../mk/git.mk
include ../mk/automake.mk
