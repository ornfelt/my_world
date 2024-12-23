lib_LTLIBRARIES = libjks.la

libjks_la_SOURCES = src/aabb.c \
                    src/array.c \
                    src/frustum.c \
                    src/hmap.c \
                    src/list.c

libjks_la_LIBADD = -lm

pkgincludedir = $(includedir)/jks
pkginclude_HEADERS = src/aabb.h \
                     src/array.h \
                     src/frustum.h \
                     src/hmap.h \
                     src/list.h \
                     src/mat1.h \
                     src/mat2.h \
                     src/mat3.h \
                     src/mat4.h \
                     src/optional.h \
                     src/quaternion.h \
                     src/utf16.h \
                     src/utf8.h \
                     src/vec1.h \
                     src/vec2.h \
                     src/vec3.h \
                     src/vec4.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libjks.pc

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
