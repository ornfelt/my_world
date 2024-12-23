AC_INIT([fuse_mpq],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR
LT_INIT

PKG_CHECK_MODULES(FUSE, [fuse], [], [AC_MSG_ERROR([fuse is required])])
PKG_CHECK_MODULES(LIBWOW, [libwow], [], [AC_MSG_ERROR([libwow is required])])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
