lib_LTLIBRARIES = libjkg.la \
                  libGL.la \
                  libGLU.la \
                  librast.la

if ENABLE_TESTS
bin_PROGRAMS = demo \
               immediate \
               next
endif

libGL_la_SOURCES = GL/blend.c \
                   GL/buffer.c \
                   GL/cache.c \
                   GL/clear.c \
                   GL/ctx.c \
                   GL/depth_stencil.c \
                   GL/draw.c \
                   GL/fixed.c \
                   GL/fog.c \
                   GL/framebuffer.c \
                   GL/get.c \
                   GL/immediate.c \
                   GL/internal.h \
                   GL/libGL.map \
                   GL/light.c \
                   GL/list.c \
                   GL/mat4.c \
                   GL/mat4.h \
                   GL/material.c \
                   GL/matrix.c \
                   GL/misc.c \
                   GL/object.c \
                   GL/opt.c \
                   GL/program.c \
                   GL/rasterizer.c \
                   GL/read.c \
                   GL/renderbuffer.c \
                   GL/sampler.c \
                   GL/shader.c \
                   GL/texture.c \
                   GL/vertex_array.c \
                   include/fnv.h \
                   include/utils.h
libGL_la_LDFLAGS = -Wl,--version-script=$(srcdir)/GL/libGL.map
libGL_la_CFLAGS = -I$(srcdir)/include
libGL_la_LIBADD = -lm \
                  -ldl \
                  libjkg.la

libjkg_la_SOURCES = jkg/bc.c \
                    jkg/format.c \
                    jkg/id.c \
                    jkg/jkg.c \
                    jkg/jkg.h \
                    jkg/libjkg.map \
                    jkg/tgsi.c
libjkg_la_LDFLAGS = -Wl,--version-script=$(srcdir)/jkg/libjkg.map
libjkg_la_CFLAGS = -I$(srcdir)/include

librast_la_SOURCES = rast/draw.c \
                     rast/librast.map \
                     rast/line.c \
                     rast/point.c \
                     rast/rast.h \
                     rast/triangle.c \
                     rast/vert.c
librast_la_LDFLAGS = -Wl,--version-script=$(srcdir)/rast/librast.map
librast_la_CFLAGS = -I$(srcdir)/include
librast_la_LIBADD = -lm

if ENABLE_GJIT
lib_LTLIBRARIES+= libjkg_gjit.la
libjkg_gjit_la_SOURCES = gjit/blend.c \
                         gjit/buffer.c \
                         gjit/clear.c \
                         gjit/depth_stencil.c \
                         gjit/draw.c \
                         gjit/frag.c \
                         gjit/gjit.c \
                         gjit/gjit.h \
                         gjit/image.c \
                         gjit/image_view.c \
                         gjit/input_layout.c \
                         gjit/rasterizer.c \
                         gjit/sampler.c \
                         gjit/shader.c \
                         gjit/surface.c
libjkg_gjit_la_LDFLAGS = -Wl,--version-script=$(srcdir)/gjit/libjkg_gjit.map
libjkg_gjit_la_CFLAGS = -I$(srcdir)/include
libjkg_gjit_la_LIBADD = -lm \
                        -lgccjit \
                        libjkg.la \
                        librast.la
endif

if ENABLE_SOFT
lib_LTLIBRARIES+= libjkg_soft.la
libjkg_soft_la_SOURCES = soft/blend.c \
                         soft/buffer.c \
                         soft/clear.c \
                         soft/depth_stencil.c \
                         soft/draw.c \
                         soft/frag.c \
                         soft/image.c \
                         soft/image_view.c \
                         soft/input_layout.c \
                         soft/rasterizer.c \
                         soft/sampler.c \
                         soft/shader.c \
                         soft/soft.c \
                         soft/soft.h \
                         soft/surface.c \
                         soft/texture.c
libjkg_soft_la_LDFLAGS = -Wl,--version-script=$(srcdir)/soft/libjkg_soft.map
libjkg_soft_la_CFLAGS = -I$(srcdir)/include
libjkg_soft_la_LIBADD = -lm \
                        libjkg.la \
                        librast.la
endif

if ENABLE_VIRGL
lib_LTLIBRARIES+= libjkg_virgl.la
libjkg_virgl_la_SOURCES = virgl/blend.c \
                          virgl/buffer.c \
                          virgl/clear.c \
                          virgl/depth_stencil.c \
                          virgl/draw.c \
                          virgl/input_layout.c \
                          virgl/image.c \
                          virgl/image_view.c \
                          virgl/rasterizer.c \
                          virgl/sampler.c \
                          virgl/shader.c \
                          virgl/surface.c \
                          virgl/vgl.c \
                          virgl/virgl.c \
                          virgl/virgl.h
libjkg_virgl_la_LDFLAGS = -Wl,--version-script=$(srcdir)/virgl/libjkg_virgl.map
libjkg_virgl_la_CFLAGS = -I$(srcdir)/include
libjkg_virgl_la_LIBADD = libjkg.la
endif

libGLU_la_SOURCES = GLU/libGLU.map \
                    GLU/perspective.c
libGLU_la_LDFLAGS = -Wl,--version-script=$(srcdir)/GLU/libGLU.map
libGLU_la_CFLAGS = -I$(srcdir)/include
libGLU_la_LIBADD = -lm \
                   libGL.la

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

next_SOURCES = tests/next.c \
               tests/common.c
next_CFLAGS = -I$(srcdir)/include
next_LDFLAGS = -lm -lX11 -lXext
next_LDADD = libGL.la

pkgincludedir = $(includedir)/GL

pkginclude_HEADERS = include/gl.h \
                     include/glu.h

EXTRA_DIST = LICENSE \
             GL/libGL.map \
             GLU/libGLU.map \
             gjit/libjkg_gjit.map \
             jkg/libjkg.map \
             rast/librast.map \
             soft/libjkg_soft.map \
             virgl/libjkg_virgl.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
