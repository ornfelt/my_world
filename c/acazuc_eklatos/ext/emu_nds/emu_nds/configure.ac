AC_INIT([emu_nds],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_CONFIG_FILES([Makefile])

AC_ARG_ENABLE([multithread],
	AS_HELP_STRING([--enable-multithread], [enable multithreading]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-multithread)])],
	[enable_multithread=no]
)

AM_CONDITIONAL([ENABLE_MULTITHREAD], [test "x$enable_multithread" = "xyes"])
AM_COND_IF([ENABLE_MULTITHREAD], AC_DEFINE([ENABLE_MULTITHREAD], [1], [define to enable multithreading]))

AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT
