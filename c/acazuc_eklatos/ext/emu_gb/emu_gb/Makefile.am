lib_LTLIBRARIES = libemu_gb_libretro.la libemu_gb.la

libemu_gb_la_SOURCES = src/gb.c \
                       src/gb.h \
                       src/apu.c \
                       src/apu.h \
                       src/cpu.c \
                       src/cpu.h \
                       src/gpu.c \
                       src/gpu.h \
                       src/mem.c \
                       src/mem.h \
                       src/mbc.c \
                       src/mbc.h \
                       src/cpu/instr.c \
                       src/cpu/instr.h

libemu_gb_libretro_la_SOURCES = $(libemu_gb_la_SOURCES) \
                                src/libretro/libretro.c \
                                src/libretro/libretro.h \
                                src/libretro/libretro.T

libemu_gb_libretro_la_LDFLAGS = -Wl,--version-script=$(srcdir)/src/libretro/libretro.T

pkgincludedir = $(includedir)/emu_gb
pkginclude_HEADERS = src/gb.h

EXTRA_DIST = LICENSE emu_gb_libretro.info
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
