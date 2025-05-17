AC_INIT([jkx],[1.0])

AC_CONFIG_SRCDIR([X11])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR

LT_INIT

AC_CONFIG_FILES([
	Makefile
	GLX/glx.pc
	xcb/xcb.pc
	xcb/xcb-dbe.pc
	xcb/xcb-glx.pc
	xcb/xcb-shm.pc
	xcb/xcb-render.pc
	X11/x11.pc
	X11/x11-xcb.pc
	X11/xext.pc
])

AC_ARG_ENABLE([tests],
	AS_HELP_STRING([--enable-tests], [turn on tests binaries]),
	[AS_CASE(${enableval}, [yes], [], [no], [],
		[AC_MSG_ERROR(bad value ${enableval} for --enable-tests)])],
	[enable_tests=no]
)

AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])

AC_OUTPUT
