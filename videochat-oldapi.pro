#-------------------------------------------------
#
# Project created by QtCreator 2012-04-22T16:37:59
#
#-------------------------------------------------

QT       += core gui

CONFIG += link_pkgconfig
PKGCONFIG += libavcodec libavformat libavdevice libswscale libavutil
#LIBS += -lavcodec -lavformat -lavdevice
#INCLUDEPATH += /usr/include/c++/4.7.0/
QMAKE_CXXFLAGS = -D__STDC_CONSTANT_MACROS

TARGET = videochat-oldapi
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    ffwebcam.cpp

HEADERS  += mainwindow.h \
    ffwebcam.h

FORMS    += mainwindow.ui
