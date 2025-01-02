lib_LTLIBRARIES = libGL.la libGLU.la

if ENABLE_TESTS

bin_PROGRAMS = demo \
               immediate

endif

libGL_la_SOURCES = gl/rast/frag.c \
                   gl/rast/line.c \
                   gl/rast/point.c \
                   gl/rast/texture.c \
                   gl/rast/triangle.c \
                   gl/rast/vert.c \
                   gl/texture/bind_texture.c \
                   gl/texture/delete_textures.c \
                   gl/texture/gen_textures.c \
                   gl/texture/is_texture.c \
                   gl/texture/parameter.c \
                   gl/texture/tex_image_2d.c \
                   gl/blend.c \
                   gl/clear.c \
                   gl/client_arrays.c \
                   gl/ctx.c \
                   gl/depth.c \
                   gl/draw.c \
                   gl/fixed.c \
                   gl/fog.c \
                   gl/get.c \
                   gl/immediate.c \
                   gl/light.c \
                   gl/list.c \
                   gl/mat4.c \
                   gl/material.c \
                   gl/matrix.c \
                   gl/misc.c \
                   gl/opt.c \
                   gl/read_pixels.c \
                   include/internal.h \
                   include/mat4.h \
                   include/utils.h \
                   include/fixed.h \
                   include/fnv.h
libGL_la_LDFLAGS = -Wl,--version-script=$(srcdir)/gl/libGL.map
libGL_la_CFLAGS = -I$(srcdir)/include
libGL_la_LIBADD = -lm
if ENABLE_GCCJIT
libGL_la_SOURCES+= gl/jit/fragment_set.c \
                   gl/jit/depth_test.c \
                   gl/jit/texture_fetch.c
libGL_la_CFLAGS+= -DENABLE_GCCJIT
libGL_la_LIBADD+= -lgccjit
endif

libGLU_la_SOURCES = glu/perspective.c
libGLU_la_LDFLAGS = -Wl,--version-script=$(srcdir)/glu/libGLU.map
libGLU_la_CFLAGS = -I$(srcdir)/include
libGLU_la_LIBADD = libGL.la

demo_SOURCES = tests/demo.c \
               tests/common.c

demo_CFLAGS = -I$(srcdir)/include
demo_LDFLAGS = -lm -lX11 -lXext
demo_LDADD = libGL.la libGLU.la

immediate_SOURCES = tests/immediate.c \
                    tests/common.c

immediate_CFLAGS = -I$(srcdir)/include
immediate_LDFLAGS = -lm -lX11 -lXext
immediate_LDADD = libGL.la

pkgincludedir = $(includedir)/GL

pkginclude_HEADERS = include/gl.h \
                     include/glu.h

EXTRA_DIST = LICENSE \
             gl/libGL.map \
             glu/libGLU.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
