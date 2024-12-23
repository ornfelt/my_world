lib_LTLIBRARIES = libgzip.la

libgzip_la_SOURCES = src/InputStream.cpp \
                     src/OutputStream.cpp \
                     src/FileInputStream.cpp \
                     src/FileOutputStream.cpp \
                     src/MemoryInputStream.cpp \
                     src/MemoryOutputStream.cpp

libgzip_la_CXXFLAGS = -I$(srcdir)/src

pkgincludedir = $(includedir)/libgzip

pkginclude_HEADERS = src/InputStream.h \
                     src/OutputStream.h \
                     src/FileInputStream.h \
                     src/FileOutputStream.h \
                     src/MemoryInputStream.h \
                     src/MemoryOutputStream.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
