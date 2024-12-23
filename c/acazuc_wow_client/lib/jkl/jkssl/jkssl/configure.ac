AC_INIT([jkssl],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR
LT_INIT

case "$host" in
	*mingw*)
		windows_target=yes
		;;
	*)
		windows_target=no
		;;
esac

AM_CONDITIONAL([LWS2_32], [test "x$windows_target" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libjkssl.pc])

AC_OUTPUT
