lib_LTLIBRARIES = libgfx.la

libgfx_la_SOURCES = src/device.c src/window.c \
                    src/device_vtable.h src/window_vtable.h \
                    src/headers/EGL/egl.h \
                    src/headers/EGL/eglplatform.h \
                    src/headers/GL/glcorearb.h \
                    src/headers/GL/glxext.h \
                    src/headers/GL/wglext.h \
                    src/headers/GLES3/gl32.h \
                    src/headers/GLES3/gl3platform.h \
                    src/headers/KHR/khrplatform.h

libgfx_la_CFLAGS = -I$(srcdir)/src/headers -I$(srcdir)/src $(LIBJKS_CFLAGS)
libgfx_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libgfx.map
libgfx_la_LIBADD = $(SYSTEM_LIBS) $(LIBJKS_LIBS)

if DEVICE_GL
libgfx_la_SOURCES += src/devices/gl.c
endif

if DEVICE_GL3
libgfx_la_SOURCES += src/devices/gl3.c
endif

if DEVICE_GL4
libgfx_la_SOURCES += src/devices/gl4.c
endif

if DEVICE_GLES3
libgfx_la_SOURCES += src/devices/gles3.c
endif

if DEVICE_VK
libgfx_la_SOURCES += src/devices/vk.c \
                     src/devices/vk_mem_alloc.cpp
endif

if DEVICE_D3D9
libgfx_la_SOURCES += src/devices/d3d9.c
endif

if DEVICE_D3D11
libgfx_la_SOURCES += src/devices/d3d11.c
endif

if WINDOW_GLX
libgfx_la_SOURCES += src/windows/glx.c
endif

if WINDOW_X11
libgfx_la_SOURCES += src/windows/x11.c
endif

if WINDOW_EGL
libgfx_la_SOURCES += src/windows/egl.c
endif

if WINDOW_WL
libgfx_la_SOURCES += src/windows/wl.c
endif

if WINDOW_WIN32
libgfx_la_SOURCES += src/windows/win32.c
endif

if WINDOW_D3D
libgfx_la_SOURCES += src/windows/d3d.c
endif

if WINDOW_WGL
libgfx_la_SOURCES += src/windows/wgl.c
endif

if WINDOW_GL_GLFW
libgfx_la_SOURCES += src/windows/gl_glfw.c
endif

if WINDOW_GLFW
libgfx_la_SOURCES += src/windows/glfw.c
endif

if WINDOW_GL_SDL
libgfx_la_SOURCES += src/windows/gl_sdl.c
endif

if WINDOW_SDL
libgfx_la_SOURCES += src/windows/sdl.c
endif

if WINDOW_VK_X11
libgfx_la_SOURCES += src/windows/vk_x11.c
endif

if WINDOW_VK_WL
libgfx_la_SOURCES += src/windows/vk_wl.c
endif

if WINDOW_VK_WIN32
libgfx_la_SOURCES += src/windows/vk_win32.c
endif

if WINDOW_VK_GLFW
libgfx_la_SOURCES += src/windows/vk_glfw.c
endif

if WINDOW_VK_SDL
libgfx_la_SOURCES += src/windows/vk_sdl.c
endif

if WINDOW_EMSCRIPTEN
libgfx_la_SOURCES += src/windows/emscripten.c
endif

pkgincludedir = $(includedir)/gfx
pkginclude_HEADERS = src/device.h \
                     src/events.h \
                     src/objects.h \
                     src/window.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgfx.pc

EXTRA_DIST = LICENSE libgfx.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
