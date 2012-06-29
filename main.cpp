#include <QtGui/QApplication>
#include "mainwindow.h"
#include "ffmpeg.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();
    avformat_network_init();

    qRegisterMetaType<QAVFrame>("QAVFrame");

    MainWindow w;
    w.show();

    return a.exec();
}
