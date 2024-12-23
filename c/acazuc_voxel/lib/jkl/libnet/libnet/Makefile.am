lib_LTLIBRARIES = libnet.la

libnet_la_SOURCES = src/Buffer.cpp \
                    src/ServerSocket.cpp \
                    src/Socket.cpp \
                    src/Connection.cpp \
                    src/Packet.cpp \
                    src/EventsListener.cpp \
                    src/Protocol.cpp
libnet_la_CXXFLAGS = -I$(srcdir)/src

pkgincludedir = $(includedir)/libnet

pkginclude_HEADERS = src/Buffer.h \
                     src/ServerSocket.h \
                     src/Socket.h \
                     src/Connection.h \
                     src/Packet.h \
                     src/EventsListener.h \
                     src/Protocol.h \
                     src/SocketPlatform.h \
                     src/Platform.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
