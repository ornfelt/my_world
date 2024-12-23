lib_LTLIBRARIES = libaudio.la

libaudio_la_SOURCES = src/CachedPlayer.cpp \
                      src/Device.cpp \
                      src/DirectPlayer.cpp \
                      src/loadDatas.cpp \
                      src/Player.cpp \
                      src/Stream.cpp \
                      src/Filters/BandPassFilter.cpp \
                      src/Filters/BilinearTransformFilter.cpp \
                      src/Filters/EchoFilter.cpp \
                      src/Filters/Filter.cpp \
                      src/Filters/HighPassFilter.cpp \
                      src/Filters/LowPassFilter.cpp

libaudio_la_CXXFLAGS = -I$(srcdir)/src

nobase_include_HEADERS = libaudio/AudioDevice.h \
                         libaudio/CachedPlayer.h \
                         libaudio/Device.h \
                         libaudio/DirectPlayer.h \
                         libaudio/Exception.h \
                         libaudio/loadDatas.h \
                         libaudio/Player.h \
                         libaudio/Stream.h \
                         libaudio/Filters/BandPassFilter.h \
                         libaudio/Filters/BilinearTransformFilter.h \
                         libaudio/Filters/EchoFilter.h \
                         libaudio/Filters/Filter.h \
                         libaudio/Filters/HighPassFilter.h \
                         libaudio/Filters/LowPassFilter.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
