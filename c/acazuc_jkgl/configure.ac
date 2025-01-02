AC_INIT([jkgl], [1.0])

AC_CONFIG_SRCDIR([gl])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR
AM_PROG_LIBTOOL

LT_INIT

AC_CONFIG_FILES([Makefile])

AC_ARG_ENABLE([tests],
	AS_HELP_STRING([--enable-tests], [turn on tests binaries]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-tests)])],
	[enable_tests=no]
)

AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])

AC_ARG_ENABLE([gccjit],
	AS_HELP_STRING([--enable-gccjit], [turn on gccjit]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-gccjit)])],
	[enable_gccjit=no]
)

AM_CONDITIONAL([ENABLE_GCCJIT], [test "x$enable_gccjit" = "xyes"])

AC_OUTPUT
