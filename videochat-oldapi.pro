QT += core gui network

win32 {
   LIBS += -L$$PWD/3rd-party/lib -lavcodec -lavformat -lavdevice \
                                 -lswscale -lavutil -lswresample \
                                 -lqxmpp0 -ldnsapi -lws2_32
   INCLUDEPATH += $$PWD/3rd-party/include
   DEFINES += WIN32
}
unix {
   LIBS += -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample -lqxmpp
   DEFINES += LINUX
}

DEFINES += __STDC_CONSTANT_MACROS

TARGET = videochat-oldapi
TEMPLATE = app

SOURCES += main.cpp \
           logging.cpp \
           mainwindow.cpp \
           callrequest.cpp \
           callresponse.cpp \
           callscreen.cpp \
           ffmpeg/input.cpp \
           ffmpeg/output.cpp \
           ffmpeg/hardware.cpp \
           ffmpeg/player.cpp \
           streaming.cpp

HEADERS  += logging.h \
            mainwindow.h \
            callrequest.h \
            callresponse.h \
            callscreen.h \
            ffmpeg.h \
            ffmpeg/input.h \
            ffmpeg/output.h \
            ffmpeg/hardware.h \
            player.h \
            streaming.h

FORMS    += mainwindow.ui \
            callrequest.ui \
            callresponse.ui \
            callscreen.ui
