#include <QtGui/QApplication>
#include "mainwindow.h"
//#include "ffwebcam.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    avcodec_register_all();
    avdevice_register_all();
    av_register_all();
    avformat_network_init();

    MainWindow w;
    w.show();

    return a.exec();
}
