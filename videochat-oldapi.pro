#-------------------------------------------------
#
# Project created by QtCreator 2012-04-22T16:37:59
#
#-------------------------------------------------

QT       += core gui

CONFIG += link_pkgconfig
PKGCONFIG += opencv libavcodec libavformat
#INCLUDEPATH += /usr/include/c++/4.7.0/

TARGET = videochat-oldapi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    webcam.cpp \
    converter.c

HEADERS  += mainwindow.h \
    webcam.h \
    converter.h

FORMS    += mainwindow.ui
