AC_INIT([wow_explorer],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR
LT_INIT

PKG_CHECK_MODULES(LIBWOW, [libwow], [], [AC_MSG_ERROR([libwow is required])])
PKG_CHECK_MODULES(LIBJKS, [libjks], [], [AC_MSG_ERROR([libjks is required])])
PKG_CHECK_MODULES(LIBPNG, [libpng], [], [AC_MSG_ERROR([libpng is required])])
PKG_CHECK_MODULES(JANSSON, [jansson], [], [AC_MSG_ERROR([jansson is required])])
PKG_CHECK_MODULES(LIBADWAITA, [libadwaita-1], [], [AC_MSG_ERROR([libadwaita-1 is required])])
PKG_CHECK_MODULES(GTKSOURCEVIEW, [gtksourceview-5], [], [AC_MSG_ERROR([gtksourceview-5 is required])])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
