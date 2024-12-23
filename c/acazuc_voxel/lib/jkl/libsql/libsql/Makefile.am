lib_LTLIBRARIES = libsql.la

libsql_la_SOURCES = src/MariaDBConnection.cpp \
                    src/MariaDBStatement.cpp
libsql_la_CXXFLAGS = -I$(srcdir)/src

pkgincludedir = $(includedir)/libsql

pkginclude_HEADERS = src/MariaDBConnection.h \
                     src/MariaDBStatement.h \
                     src/Connection.h \
                     src/Statement.h \
                     src/Exception.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
