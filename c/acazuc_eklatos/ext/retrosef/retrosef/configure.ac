AC_INIT([retrosef],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile])

AC_ARG_WITH([audio],
	AS_HELP_STRING([--with-audio=pulseaudio|eklat], [use pulseaudio or eklat audio backend]),
	[AS_CASE($with_audio, [pulseaudio], [], [eklat], [],
		[AC_MSG_ERROR(bad value ${enableval} for --with-audio)])],
	[with_audio=]
)

AM_CONDITIONAL([AUDIO_PULSEAUDIO], [test "x$with_audio" = "xpulseaudio"])
AM_COND_IF([AUDIO_PULSEAUDIO], AC_DEFINE([AUDIO_PULSEAUDIO], [1], [define to enable pulseaudio backend]))
AM_CONDITIONAL([AUDIO_EKLAT], [test "x$with_audio" = "xeklat"])
AM_COND_IF([AUDIO_EKLAT], AC_DEFINE([AUDIO_EKLAT], [1], [define to enable eklat backend]))

AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_OUTPUT
