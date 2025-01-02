bin_PROGRAMS = retrosef

retrosef_SOURCES = src/main.c

retrosef_LDADD = -lX11 -lXext -ldl -lz

if AUDIO_PULSEAUDIO
retrosef_LDADD+= -lpulse -lpulse-simple
endif

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
