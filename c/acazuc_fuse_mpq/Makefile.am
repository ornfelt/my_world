bin_PROGRAMS = mount.mpq

mount_mpq_SOURCES = src/main.c
mount_mpq_CFLAGS = $(FUSE_CFLAGS) $(LIBWOW_CFLAGS)
mount_mpq_LDADD = $(FUSE_LIBS) $(LIBWOW_LIBS)

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
