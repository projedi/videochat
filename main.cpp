#include <QtGui/QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "ffmpeg.h"

int main(int argc, char *argv[])
{
   initLogging();
   QApplication a(argc, argv);

   avdevice_register_all();
   av_register_all();
   avformat_network_init();

   MainWindow w;
   w.show();

   return a.exec();
}
