lib_LTLIBRARIES = libemu_nds_libretro.la libemu_nds.la

libemu_nds_la_SOURCES = src/nds.c \
                        src/nds.h \
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

libemu_nds_libretro_la_SOURCES = $(libemu_nds_la_SOURCES) \
                                 src/libretro/libretro.c \
                                 src/libretro/libretro.h \
                                 src/libretro/libretro.T

libemu_nds_libretro_la_LDFLAGS = -Wl,--version-script=$(srcdir)/src/libretro/libretro.T

if ENABLE_MULTITHREAD

libemu_nds_la_LIBADD = -lpthread
libemu_nds_libretro_la_LIBADD = -lpthread

endif

pkgincludedir = $(includedir)/emu_nds
pkginclude_HEADERS = src/nds.h

EXTRA_DIST = LICENSE emu_nds_libretro.info
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
