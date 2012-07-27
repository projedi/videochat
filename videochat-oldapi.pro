QT += core gui network

win32 {
   LIBS += -L$$PWD/3rd-party/lib -lavcodec -lavformat -lavdevice \
                                 -lswscale -lavutil -lswresample \
                                 -lqxmpp0 -ldnsapi -lws2_32 -lstabilization
   INCLUDEPATH += $$PWD/3rd-party/include
   DEFINES += WIN32
}
unix {
   LIBS += -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample -lqxmpp_d -lstabilization
   DEFINES += LINUX
}

DEFINES += __STDC_CONSTANT_MACROS

TARGET = videochat-oldapi
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           ffmpeg/input.cpp \
           ffmpeg/output.cpp \
           ffmpeg/hardware.cpp \
           ffmpeg/player.cpp \
           streaming.cpp \
           conversation.cpp

HEADERS  += mainwindow.h \
            ffmpeg.h \
            ffmpeg/input.h \
            ffmpeg/output.h \
            ffmpeg/hardware.h \
            ffmpeg/player.h \
            streaming.h \
            conversation.h

FORMS    += mainwindow.ui
