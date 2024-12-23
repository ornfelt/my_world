lib_LTLIBRARIES = libwow.la

libwow_la_SOURCES = src/adt.c \
                    src/blp.c \
                    src/bls.c \
                    src/common.c \
                    src/dbc.c \
                    src/m2.c \
                    src/mpq.c \
                    src/toc.c \
                    src/trs.c \
                    src/wdb.c \
                    src/wdl.c \
                    src/wdt.c \
                    src/wmo.c \
                    src/wmo_group.c \
                    src/zmp.c

libwow_la_CFLAGS = $(ZLIB_CFLAGS)
libwow_la_LIBADD = $(ZLIB_LIBS)

pkgincludedir = $(includedir)/wow

pkginclude_HEADERS = src/adt.h \
                     src/blp.h \
                     src/bls.h \
                     src/common.h \
                     src/dbc.h \
                     src/m2.h \
                     src/mpq.h \
                     src/toc.h \
                     src/trs.h \
                     src/wdb.h \
                     src/wdl.h \
                     src/wdt.h \
                     src/wmo.h \
                     src/wmo_group.h \
                     src/zmp.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libwow.pc

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
