AC_INIT([libgfx], [1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_DEFUN([AC_DEVICE_ENABLE], [
	AC_ARG_ENABLE([device-$1],
		AS_HELP_STRING([--enable-device-$1], [turn on $2 device backend]),
		[AS_CASE(${enableval}, [yes], [], [no], [],
			[AC_MSG_ERROR(bad value ${enableval} for --enable-device-$1)])],
		[enable_device_$1=no]
	)]
)

AC_DEFUN([AC_WINDOW_ENABLE], [
	AC_ARG_ENABLE([window-$1],
		AS_HELP_STRING([--enable-window-$1], [turn on $2 window backend]),
		[AS_CASE(${enableval}, [yes], [], [no], [],
			[AC_MSG_ERROR(bad value ${enableval} for --enable-window-$1)])],
		[enable_window_$1=no]
	)]
)

SYSTEM_LIBS=""

AC_DEVICE_ENABLE([gl3], [OpenGL 3])
AM_CONDITIONAL([DEVICE_GL3], [test "x$enable_device_gl3" = "xyes"])
AM_COND_IF([DEVICE_GL3], AC_DEFINE([GFX_ENABLE_DEVICE_GL3],, [define to enable OpenGL 3 device backend]))

AC_DEVICE_ENABLE([gl4], [OpenGL 4])
AM_CONDITIONAL([DEVICE_GL4], [test "x$enable_device_gl4" = "xyes"])
AM_COND_IF([DEVICE_GL4], AC_DEFINE([GFX_ENABLE_DEVICE_GL4],, [define to enable OpenGL 4 device backend]))

AC_DEVICE_ENABLE([gles3], [OpenGL ES 3])
AM_CONDITIONAL([DEVICE_GLES3], [test "x$enable_device_gles3" = "xyes"])
AM_COND_IF([DEVICE_GLES3], AC_DEFINE([GFX_ENABLE_DEVICE_GLES3],, [define to enable OpenGL ES 3 device backend]))

AC_DEVICE_ENABLE([d3d9], [Direct3D 9])
AM_CONDITIONAL([DEVICE_D3D9], [test "x$enable_device_d3d9" = "xyes"])
AM_COND_IF([DEVICE_D3D9], AC_DEFINE([GFX_ENABLE_DEVICE_D3D9],, [define to enable Direct3D 9 device backend]))
AM_COND_IF([DEVICE_D3D9], [SYSTEM_LIBS="${SYSTEM_LIBS} -ld3d9"],,)

AC_DEVICE_ENABLE([d3d11], [Direct3D 11])
AM_CONDITIONAL([DEVICE_D3D11], [test "x$enable_device_d3d11" = "xyes"])
AM_COND_IF([DEVICE_D3D11], AC_DEFINE([GFX_ENABLE_DEVICE_D3D11],, [define to enable Direct3D 11 device backend]))
AM_COND_IF([DEVICE_D3D11], [SYSTEM_LIBS="${SYSTEM_LIBS} -ld3d11"],,)

AC_DEVICE_ENABLE([vk], [Vulkan])
AM_CONDITIONAL([DEVICE_VK], [test "x$enable_device_vk" = "xyes"])
AM_COND_IF([DEVICE_VK], AC_DEFINE([GFX_ENABLE_DEVICE_VK],, [define to enable Vulkan device backend]))
AM_COND_IF([DEVICE_VK], [SYSTEM_LIBS="${SYSTEM_LIBS} -lvulkan"],,)

AC_WINDOW_ENABLE([x11], [X11])
AM_CONDITIONAL([WINDOW_X11], [test "x$enable_window_x11" = "xyes"])
AM_COND_IF([WINDOW_X11], AC_DEFINE([GFX_ENABLE_WINDOW_X11],, [define to enable X11 window backend]))
AM_COND_IF([WINDOW_X11], [SYSTEM_LIBS="${SYSTEM_LIBS} -lX11 -lXi -lXcursor"],,)

AC_WINDOW_ENABLE([win32], [Win32])
AM_CONDITIONAL([WINDOW_WIN32], [test "x$enable_window_win32" = "xyes"])
AM_COND_IF([WINDOW_WIN32], AC_DEFINE([GFX_ENABLE_WINDOW_WIN32],, [define to enable Win32 window backend]))
AM_COND_IF([WINDOW_WIN32], [SYSTEM_LIBS="${SYSTEM_LIBS} -lgdi32"],,)

AC_WINDOW_ENABLE([wl], [Wayland])
AM_CONDITIONAL([WINDOW_WL], [test "x$enable_window_wl" = "xyes"])
AM_COND_IF([WINDOW_WL], AC_DEFINE([GFX_ENABLE_WINDOW_WL],, [define to enable Wayland window backend]))
AM_COND_IF([WINDOW_WL], [SYSTEM_LIBS="${SYSTEM_LIBS} -lwayland-client"],,)

AC_WINDOW_ENABLE([glfw], [GLFW])
AM_CONDITIONAL([WINDOW_GLFW], [test "x$enable_window_glfw" = "xyes"])
AM_COND_IF([WINDOW_GLFW], AC_DEFINE([GFX_ENABLE_WINDOW_GLFW],, [define to enable GLFW window backend]))
AM_COND_IF([WINDOW_GLFW], [SYSTEM_LIBS="${SYSTEM_LIBS} -lglfw3"],,)

AC_WINDOW_ENABLE([sdl], [SDL])
AM_CONDITIONAL([WINDOW_SDL], [test "x$enable_window_sdl" = "xyes"])
AM_COND_IF([WINDOW_SDL], AC_DEFINE([GFX_ENABLE_WINDOW_SDL],, [define to enable SDL window backend]))
AM_COND_IF([WINDOW_SDL], [SYSTEM_LIBS="${SYSTEM_LIBS} -lSDL2"],,)

AC_WINDOW_ENABLE([emscripten], [emscripten])
AM_CONDITIONAL([WINDOW_EMSCRIPTEN], [test "x$enable_window_emscripten" = "xyes"])
AM_COND_IF([WINDOW_EMSCRIPTEN], AC_DEFINE([GFX_ENABLE_WINDOW_EMSCRIPTEN],, [define to enable emscripten window backend]))

AS_IF([test "x$enable_device_gl3$enable_device_gl4$enable_device_gles3" != "xnonono"], [enable_device_gl=yes], [enable_device_gl=no])
AS_IF([test "x$enable_device_d3d9$enable_device_d3d11" != "xnono"], [enable_device_d3d=yes], [enable_device_d3d=no])

# Aggregate for individual files
AM_CONDITIONAL(DEVICE_GL, test "x$enable_device_gl" != "xno")
AM_CONDITIONAL(WINDOW_GL, test "x$enable_window_glfw$enable_window_x11$enable_window_win32$enable_window_wl" != "xnononono" -a "x$enable_device_gl" != "xno")
AM_CONDITIONAL(WINDOW_GLX, test "x$enable_window_x11$enable_device_gl" = "xyesyes")
AM_CONDITIONAL(WINDOW_EGL, test "x$enable_window_wl$enable_device_gl" = "xyesyes")
AM_CONDITIONAL(WINDOW_WGL, test "x$enable_window_win32$enable_device_gl" = "xyesyes")
AM_CONDITIONAL(WINDOW_D3D, test "x$enable_window_win32$enable_device_d3d" = "xyesyes")
AM_CONDITIONAL(WINDOW_GL_GLFW, test "x$enable_window_glfw$enable_device_gl" = "xyesyes")
AM_CONDITIONAL(WINDOW_GL_SDL, test "x$enable_window_sdl$enable_device_gl" = "xyesyes")
AM_CONDITIONAL(WINDOW_VK_X11, test "x$enable_window_x11$enable_device_vk" = "xyesyes")
AM_CONDITIONAL(WINDOW_VK_WIN32, test "x$enable_window_win32$enable_device_vk" = "xyesyes")
AM_CONDITIONAL(WINDOW_VK_WL, test "x$enable_window_wl$enable_device_vk" = "xyesyes")
AM_CONDITIONAL(WINDOW_VK_GLFW, test "x$enable_window_glfw$enable_device_vk" = "xyesyes")
AM_CONDITIONAL(WINDOW_VK_SDL, test "x$enable_window_sdl$enable_device_vk" = "xyesyes")

AM_COND_IF([WINDOW_GLX], [SYSTEM_LIBS="${SYSTEM_LIBS} -lGL"],,)
AM_COND_IF([WINDOW_EGL], [SYSTEM_LIBS="${SYSTEM_LIBS} -lEGL"],,)
AM_COND_IF([WINDOW_WGL], [SYSTEM_LIBS="${SYSTEM_LIBS} -lopengl32"],,)
AM_COND_IF([WINDOW_D3D], [SYSTEM_LIBS="${SYSTEM_LIBS} -ldxgi -ld3dcompiler"],,)

PKG_CHECK_MODULES(LIBJKS, [libjks], [], [AC_MSG_ERROR([libjks is required])])

AC_CONFIG_HEADERS([src/config.h])

AC_SUBST(SYSTEM_LIBS)

AC_PROG_CXX
AC_PROG_CC
AM_PROG_AR
LT_INIT

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libgfx.pc])

AC_OUTPUT
