######################################################################
# Automatically generated by qmake (2.01a) Sat Jul 23 18:51:06 2016
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

OBJECTS_DIR = build

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -Wl,--no-as-needed

LIBS    += -lpthread

CONFIG      += link_pkgconfig
PKGCONFIG   += opencv


# Input
HEADERS += ThreadPool.h
SOURCES += warp.cpp
