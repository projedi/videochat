#include <QtGui/QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "ffmpeg.h"

int main(int argc, char *argv[])
{
   init();
   logger("Started");
   QApplication a(argc, argv);

   avdevice_register_all();
   av_register_all();
   avformat_network_init();

   MainWindow w;
   w.show();

   logger("Onto event loop");
   return a.exec();
}
