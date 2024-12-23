lib_LTLIBRARIES = libformat.la

libformat_la_SOURCES = src/PNG.cpp \
                       src/OldACI.cpp \
                       src/ACI.cpp \
                       src/BC1.cpp \
                       src/BC2.cpp \
                       src/BC3.cpp \
                       src/BC4.cpp \
                       src/BC5.cpp

libformat_la_CXXFLAGS = -I$(srcdir)/src

pkgincludedir = $(includedir)/libformat

pkginclude_HEADERS = src/PNG.h \
                     src/OldACI.h \
                     src/ACI.h \
                     src/BC1.h \
                     src/BC2.h \
                     src/BC3.h \
                     src/BC4.h \
                     src/BC5.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
