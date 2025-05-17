AC_INIT([jkgl], [1.0])

AC_CONFIG_SRCDIR([GL])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_CONFIG_FILES([Makefile])

AC_ARG_ENABLE([tests],
	AS_HELP_STRING([--enable-tests], [turn on tests binaries]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-tests)])],
	[enable_tests=no]
)
AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])

AC_ARG_ENABLE([soft],
	AS_HELP_STRING([--enable-soft], [turn on soft backend]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-soft)])],
	[enable_soft=yes]
)
AM_CONDITIONAL([ENABLE_SOFT], [test "x$enable_soft" = "xyes"])

AC_ARG_ENABLE([gjit],
	AS_HELP_STRING([--enable-gjit], [turn on gjit backend]),
	[AS_CASE(${enableval},
		[yes], [AC_CHECK_HEADER([libgccjit.h],
			[], [enable_gjit=no]
		)],
		[no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-gjit)])],
	[enable_gjit=no]
)
AM_CONDITIONAL([ENABLE_GJIT], [test "x$enable_gjit" = "xyes"])

AC_ARG_ENABLE([virgl],
	AS_HELP_STRING([--enable-virgl], [turn on virgl backend]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-virgl)])],
	[enable_virgl=no]
)
AM_CONDITIONAL([ENABLE_VIRGL], [test "x$enable_virgl" = "xyes"])

AC_OUTPUT
