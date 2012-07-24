#include <QtGui/QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "ffmpeg.h"
#include <iostream>
using namespace std;

void mesOutput(QtMsgType type, const char *msg) {
   cerr << msg << endl;
   logger(msg);
}

int main(int argc, char *argv[])
{
   initLogging();
   qInstallMsgHandler(mesOutput);
   QApplication a(argc, argv);

   avdevice_register_all();
   av_register_all();
   avformat_network_init();

   MainWindow w;
   w.show();

   return a.exec();
}
