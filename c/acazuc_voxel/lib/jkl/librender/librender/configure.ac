AC_INIT([librender],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CXX
AM_PROG_AR
LT_INIT

PKG_CHECK_MODULES(FREETYPE, [freetype2], [], [AC_MSG_ERROR([freetype is required])])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
