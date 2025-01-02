lib_LTLIBRARIES = libemu_gba_libretro.la libemu_gba.la

libemu_gba_la_SOURCES = src/gba.c \
                        src/gba.h \
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
                        src/cpu/arm.c \
                        src/cpu/thumb.c \
                        src/cpu/instr.h

libemu_gba_libretro_la_SOURCES = $(libemu_gba_la_SOURCES) \
                                 src/libretro/libretro.c \
                                 src/libretro/libretro.h \
                                 src/libretro/libretro.T

libemu_gba_libretro_la_LDFLAGS = -Wl,--version-script=$(srcdir)/src/libretro/libretro.T

pkgincludedir = $(includedir)/emu_gba
pkginclude_HEADERS = src/gba.h

EXTRA_DIST = LICENSE emu_gba_libretro.info
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
