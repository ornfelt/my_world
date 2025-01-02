lib_LTLIBRARIES = libemu_nes_libretro.la libemu_nes.la

libemu_nes_la_SOURCES = src/nes.c \
                        src/apu.c \
                        src/cpu.c \
                        src/gpu.c \
                        src/mem.c \
                        src/mbc.c \
                        src/cpu/instr.c \
                        src/cpu/instr.h \
                        src/internal.h \
                        src/nes.h

libemu_nes_libretro_la_SOURCES = $(libemu_nes_la_SOURCES) \
                                 src/libretro/libretro.c \
                                 src/libretro/libretro.h \
                                 src/libretro/libretro.T

libemu_nes_libretro_la_LDFLAGS = -Wl,--version-script=$(srcdir)/src/libretro/libretro.T

pkgincludedir = $(includedir)/emu_nes
pkginclude_HEADERS = src/nes.h

EXTRA_DIST = LICENSE emu_nes_libretro.info
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
