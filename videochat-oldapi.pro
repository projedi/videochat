QT       += core gui network
win32: LIBS += -L$$PWD/3rd-party/lib -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample
win32: INCLUDEPATH += $$PWD/3rd-party/include
unix: LIBS += -lavcodec -lavformat -lavdevice -lswscale -lavutil -lswresample -lx264

DEFINES += __STDC_CONSTANT_MACROS
unix: DEFINES += LINUX
win32: DEFINES += WIN32

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
           ffmpeg/player.cpp

HEADERS  += logging.h \
            mainwindow.h \
            callrequest.h \
            callresponse.h \
            callscreen.h \
            ffmpeg.h \
            player.h

FORMS    += mainwindow.ui \
            callrequest.ui \
            callresponse.ui \
            callscreen.ui
