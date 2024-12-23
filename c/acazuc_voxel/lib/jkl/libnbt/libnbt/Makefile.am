lib_LTLIBRARIES = libnbt.la

libnbt_la_SOURCES = src/sanitize.c \
                    src/stream.c \
                    src/tag.c

libnbt_la_CFLAGS = $(ZLIB_CFLAGS)
libnbt_la_LIBADD = $(ZLIB_LIBS)

pkgincludedir = $(includedir)/nbt

pkginclude_HEADERS = src/nbt.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnbt.pc

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
