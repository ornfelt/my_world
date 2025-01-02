lib_LTLIBRARIES = libjka.la

libjka_la_SOURCES = libjka/stream.c \
                    libjka/bilinear.c \
                    libjka/wave.c \
                    libjka/resample.c
libjka_la_CFLAGS = $(LIBSAMPLERATE_CFLAGS)
libjka_la_LIBADD = $(LIBSAMPLERATE_LIBS)
libjka_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libjka/libjka.map
libjka_ladir = $(includedir)
nobase_libjka_la_HEADERS = libjka/libjka.h

bin_PROGRAMS = audiod

audiod_SOURCES = src/audiod/main.c
audiod_LDADD = libjka.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libjka.pc

EXTRA_DIST = LICENSE \
             libjka/libjka.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
