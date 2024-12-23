bin_PROGRAMS = explorer

explorer_SOURCES = src/explorer.c \
                   src/nodes.c \
                   src/tree.c \
                   src/displays/adt.c \
                   src/displays/blp.c \
                   src/displays/bls.c \
                   src/displays/dbc.c \
                   src/displays/dir.c \
                   src/displays/display.c \
                   src/displays/img.c \
                   src/displays/m2.c \
                   src/displays/ttf.c \
                   src/displays/txt.c \
                   src/displays/wdl.c \
                   src/displays/wdt.c \
                   src/displays/wmo.c \
                   src/displays/wmo_group.c \
                   src/utils/dx9_shader.c \
                   src/utils/nv_register_shader.c \
                   src/utils/nv_texture_shader.c

explorer_CFLAGS = $(LIBJKS_CFLAGS) \
                  $(LIBWOW_CFLAGS) \
                  $(LIBPNG_CFLAGS) \
                  $(JANSSON_CFLAGS) \
                  $(LIBADWAITA_CFLAGS) \
                  $(GTKSOURCEVIEW_CFLAGS)

explorer_LDADD = $(LIBJKS_LIBS) \
                 $(LIBWOW_LIBS) \
                 $(LIBPNG_LIBS) \
                 $(JANSSON_LIBS) \
                 $(LIBADWAITA_LIBS) \
                 $(GTKSOURCEVIEW_LIBS)

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
