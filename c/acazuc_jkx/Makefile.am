lib_LTLIBRARIES = libxcb.la \
                  libX11.la \
                  libX11-xcb.la \
                  libxcb-shm.la \
                  libxcb-dbe.la \
                  libxcb-render.la \
                  libXext.la \
                  libxcb-glx.la \
                  libGLX.la

libX11_la_SOURCES = X11/Xlib.c \
                    X11/Xlibint.c \
                    X11/Xlibint.h \
                    X11/Xutil.c
libX11_la_LIBADD = libxcb.la
libX11_la_LDFLAGS = -Wl,--version-script=$(srcdir)/X11/libX11.map
libX11_ladir = $(includedir)
nobase_libX11_la_HEADERS = X11/X.h \
                           X11/Xlib.h \
                           X11/Xutil.h \
                           X11/Xatom.h \
                           X11/cursorfont.h \
                           X11/keysymdef.h \
                           X11/keysym.h

libxcb_la_SOURCES = xcb/xcb.c \
                    xcb/buf.c \
                    xcb/buf.h \
                    xcb/xproto.c \
                    xcb/int.h \
                    xcb/xcbext.h
libxcb_la_LDFLAGS =  -Wl,--version-script=$(srcdir)/xcb/libxcb.map
libxcb_ladir = $(includedir)
nobase_libxcb_la_HEADERS = xcb/xcb.h \
                           xcb/xproto.h

libXext_la_SOURCES = X11/extensions/Xdbe.c \
                     X11/extensions/Xrender.c \
                     X11/extensions/XShm.c
libXext_la_LIBADD = libX11.la \
                    libxcb.la \
                    libxcb-dbe.la \
                    libxcb-render.la \
                    libxcb-shm.la
libXext_la_LDFLAGS = -Wl,--version-script=$(srcdir)/X11/libXext.map
libXext_ladir = $(includedir)
nobase_libXext_la_HEADERS = X11/extensions/Xdbe.h \
                            X11/extensions/Xrender.h \
                            X11/extensions/XShm.h

libGLX_la_SOURCES = GLX/glx.c
libGLX_la_LIBADD = libxcb-glx.la
libGLX_la_LDFLAGS = -Wl,--version-script=$(srcdir)/GLX/libGLX.map
libGLX_ladir = $(includedir)
nobase_libGLX_la_HEADERS = GLX/glx.h

libxcb_shm_la_SOURCES = xcb/shm.c
libxcb_shm_la_LIBADD = libxcb.la
libxcb_shm_la_LDFLAGS = -Wl,--version-script=$(srcdir)/xcb/libxcb-shm.map
libxcb_shm_ladir = $(includedir)
nobase_libxcb_shm_la_HEADERS = xcb/shm.h

libxcb_dbe_la_SOURCES = xcb/dbe.c
libxcb_dbe_la_LIBADD = libxcb.la
libxcb_dbe_la_LDFLAGS = -Wl,--version-script=$(srcdir)/xcb/libxcb-dbe.map
libxcb_dbe_ladir = $(includedir)
nobase_libxcb_dbe_la_HEADERS = xcb/dbe.h

libX11_xcb_la_SOURCES = X11/Xlib-xcb.c
libX11_xcb_la_LIBADD = libX11.la libxcb.la
libX11_xcb_la_LDFLAGS = -Wl,--version-script=$(srcdir)/X11/libX11-xcb.map
libX11_xcb_ladir = $(includedir)
nobase_libX11_xcb_la_HEADERS = X11/Xlib-xcb.h

libxcb_render_la_SOURCES = xcb/render.c
libxcb_render_la_LIBADD = libxcb.la
libxcb_render_la_LDFLAGS = -Wl,--version-script=$(srcdir)/xcb/libxcb-render.map
libxcb_render_ladir = $(includedir)
nobase_libxcb_render_la_HEADERS = xcb/render.h

libxcb_glx_la_SOURCES = xcb/glx.c
libxcb_glx_la_LIBADD = libxcb.la
libxcb_glx_la_LDFLAGS = -Wl,--version-script=$(srcdir)/xcb/libxcb-glx.map
libxcb_glx_ladir = $(includedir)
nobase_libxcb_glx_la_HEADERS = xcb/glx.h

bin_PROGRAMS = xdpyinfo \
               xlsfonts \
               xkbbell \
               xmousepos \
               xsrv \
               xev \
               xprop \
               xwininfo \
               xwm \
               xkbvleds \
               xlsatoms \
               xterm \
               xsysmon

if ENABLE_TESTS

bin_PROGRAMS+= draw \
               shmput \
               shmget \
               cursors \
               dbe \
               setmousepos \
               anim \
               move \
               keyboard \
               tree \
               grabbutton \
               circulate \
               border_pixmap \
               pointer \
               getimage \
               background_pixel \
               xrender \
               glx

endif

draw_SOURCES = tests/draw.c
draw_LDADD = libX11.la

xdpyinfo_SOURCES = src/xdpyinfo.c
xdpyinfo_LDADD = libX11.la

xlsfonts_SOURCES = src/xlsfonts.c
xlsfonts_LDADD = libX11.la

shmput_SOURCES = tests/shmput.c
shmput_LDADD = libX11.la libXext.la

shmget_SOURCES = tests/shmget.c
shmget_LDADD = libX11.la libXext.la

xev_SOURCES = src/xev.c
xev_LDADD = libX11.la

cursors_SOURCES = tests/cursors.c
cursors_LDADD = libX11.la

xsrv_SOURCES = src/xsrv/8x8.c \
               src/xsrv/backend.c \
               src/xsrv/client.c \
               src/xsrv/colormap.c \
               src/xsrv/cursor.c \
               src/xsrv/drawable.c \
               src/xsrv/font.c \
               src/xsrv/framebuffer.c \
               src/xsrv/gcontext.c \
               src/xsrv/main.c \
               src/xsrv/object.c \
               src/xsrv/pixmap.c \
               src/xsrv/poly.c \
               src/xsrv/proto.c \
               src/xsrv/ringbuf.c \
               src/xsrv/ringbuf.h \
               src/xsrv/window.c \
               src/xsrv/xcursor.c \
               src/xsrv/xsrv.h \
               src/xsrv/shm/proto.c \
               src/xsrv/shm/shm.c \
               src/xsrv/shm/shm.h \
               src/xsrv/shm/shmseg.c \
               src/xsrv/render/glyphset.c \
               src/xsrv/render/picture.c \
               src/xsrv/render/proto.c \
               src/xsrv/render/render.c \
               src/xsrv/render/render.h \
               src/xsrv/glx/glx.c \
               src/xsrv/glx/glx.h \
               src/xsrv/glx/proto.c
xsrv_CFLAGS = -I$(srcdir)/src/xsrv
xsrv_LDADD = libX11.la libXext.la

dbe_SOURCES = tests/dbe.c
dbe_LDADD = libX11.la libXext.la

xkbbell_SOURCES = src/xkbbell.c
xkbbell_LDADD = libX11.la

xmousepos_SOURCES = src/xmousepos.c
xmousepos_LDADD = libX11.la

setmousepos_SOURCES = tests/setmousepos.c
setmousepos_LDADD = libX11.la

anim_SOURCES = tests/anim.c
anim_LDADD = libX11.la -lm

move_SOURCES = tests/move.c
move_LDADD = libX11.la -lm

keyboard_SOURCES = tests/keyboard.c
keyboard_LDADD = libX11.la

xwininfo_SOURCES = src/xwininfo.c
xwininfo_LDADD = libX11.la

tree_SOURCES = tests/tree.c
tree_LDADD = libX11.la

xprop_SOURCES = src/xprop.c
xprop_LDADD = libX11.la

xwm_SOURCES = src/xwm/main.c \
              src/xwm/xwm.h \
              src/xwm/assets.c
xwm_LDADD = libX11.la libXext.la -lpng

xwmsharedir = $(datarootdir)/xwm
dist_xwmshare_DATA = src/xwm/assets/background.png \
                     src/xwm/assets/close.png \
                     src/xwm/assets/close_clicked.png \
                     src/xwm/assets/close_hovered.png \
                     src/xwm/assets/header.png \
                     src/xwm/assets/header_resize_l.png \
                     src/xwm/assets/header_resize_r.png \
                     src/xwm/assets/iconify.png \
                     src/xwm/assets/iconify_clicked.png \
                     src/xwm/assets/iconify_hovered.png \
                     src/xwm/assets/maximize.png \
                     src/xwm/assets/maximize_clicked.png \
                     src/xwm/assets/maximize_hovered.png \
                     src/xwm/assets/resize_b.png \
                     src/xwm/assets/resize_bl.png \
                     src/xwm/assets/resize_br.png \
                     src/xwm/assets/resize_l.png \
                     src/xwm/assets/resize_r.png \
                     src/xwm/assets/resize_t.png \
                     src/xwm/assets/resize_tl.png \
                     src/xwm/assets/resize_tr.png \
                     src/xwm/assets/taskbar.png \
                     src/xwm/assets/taskbar_button.png \
                     src/xwm/assets/taskbar_button_disabled.png \
                     src/xwm/assets/taskbar_button_hovered.png \
                     src/xwm/assets/taskbar_button_l.png \
                     src/xwm/assets/taskbar_button_r.png \
                     src/xwm/assets/taskbar_l.png \
                     src/xwm/assets/taskbar_r.png

grabbutton_SOURCES = tests/grabbutton.c
grabbutton_LDADD = libX11.la

circulate_SOURCES = tests/circulate.c
circulate_LDADD = libX11.la

border_pixmap_SOURCES = tests/border_pixmap.c
border_pixmap_LDADD = libX11.la -lm

pointer_SOURCES = tests/pointer.c
pointer_LDADD = libX11.la

xkbvleds_SOURCES = src/xkbvleds.c
xkbvleds_LDADD = libX11.la

xlsatoms_SOURCES = src/xlsatoms.c
xlsatoms_LDADD = libX11.la

getimage_SOURCES = tests/getimage.c
getimage_LDADD = libX11.la

background_pixel_SOURCES = tests/background_pixel.c
background_pixel_LDADD = libX11.la

xterm_SOURCES = src/xterm/main.c \
                src/xterm/ringbuf.h \
                src/xterm/ringbuf.c
xterm_LDADD = libX11.la -lutil

xrender_SOURCES = tests/xrender.c
xrender_LDADD = libX11.la libXext.la

xsysmon_SOURCES = src/xsysmon.c
xsysmon_LDADD = libX11.la

glx_SOURCES = tests/glx.c
glx_LDADD = libX11.la libGLX.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = GLX/glx.pc \
                 xcb/xcb.pc \
                 xcb/xcb-dbe.pc \
                 xcb/xcb-glx.pc \
                 xcb/xcb-shm.pc \
                 xcb/xcb-render.pc \
                 X11/x11.pc \
                 X11/x11-xcb.pc \
                 X11/xext.pc

EXTRA_DIST = LICENSE \
             GLX/libGLX.map \
             xcb/libxcb.map \
             xcb/libxcb-dbe.map \
             xcb/libxcb-glx.map \
             xcb/libxcb-render.map \
             xcb/libxcb-shm.map \
             X11/libX11.map \
             X11/libX11-xcb.map \
             X11/libXext.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
