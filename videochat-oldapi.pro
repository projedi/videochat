#-------------------------------------------------
#
# Project created by QtCreator 2012-04-22T16:37:59
#
#-------------------------------------------------

QT       += core gui
win32: LIBS += -L$$PWD/3rd-party/lib -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample
win32: INCLUDEPATH += $$PWD/3rd-party/include
unix: LIBS += -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample -lx264
QMAKE_CXXFLAGS = -D__STDC_CONSTANT_MACROS
unix: DEFINES += LINUX
win32: DEFINES += WIN32

#unix {
#   CONFIG += link_pkgconfig
#   PKGCONFIG += libavcodec libavformat libavdevice libswscale libavutil
#}
#win32 {
#   QMAKE_CXX = i686-w64-mingw32-g++
#   QMAKE_INCDIR = /usr/i686-w64-mingw32/include
#   QMAKE_LIBDIR = /usr/i686-w64-mingw32/lib
#}
#INCLUDEPATH += /usr/include/c++/4.7.0/

TARGET = videochat-oldapi
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           ffmpeg/input.cpp \
           ffmpeg/output.cpp \
           ffmpeg/hardware.cpp \
           ffmpeg/player.cpp

HEADERS  += mainwindow.h \
            ffmpeg.h \
            player.h

FORMS    += mainwindow.ui

#win32: LIBS += -L$$PWD/external/ffmpeg/lib/ -lavcodec -lavformat -lavdevice -lswscale -lavutil

#win32: INCLUDEPATH += $$PWD/external/ffmpeg/include
#win32: DEPENDPATH += $$PWD/external/ffmpeg/include

#win32: PRE_TARGETDEPS += $$PWD/external/ffmpeg/lib/avcodec.lib
#win32: PRE_TARGETDEPS += $$PWD/external/ffmpeg/lib/avformat.lib
#win32: PRE_TARGETDEPS += $$PWD/external/ffmpeg/lib/avdevice.lib
#win32: PRE_TARGETDEPS += $$PWD/external/ffmpeg/lib/swscale.lib
#win32: PRE_TARGETDEPS += $$PWD/external/ffmpeg/lib/avutil.lib
