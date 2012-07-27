#include <QtGui/QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "ffmpeg.h"
#include <iostream>
#include <fstream>
using namespace std;

namespace logging {
   static fstream logger;
   static clock_t startTime;

   void initLogging() {
#if defined(LINUX)
      const char *filename = "/tmp/videochat.log";
#elif defined(WIN32)
      const char *filename = "videochat.log";
#endif
      logging::logger.open(filename,fstream::out);
      logging::startTime = clock();
   }
}

void mesOutput(QtMsgType, const char *msg) {
   clock_t curTime = clock();
   double timestamp = ((double)(curTime - logging::startTime)) / CLOCKS_PER_SEC;
   cerr << timestamp << ": " << msg << endl;
   logging::logger << timestamp << ": " << msg << endl;
}

int main(int argc, char *argv[])
{
   logging::initLogging();
   qInstallMsgHandler(mesOutput);
   QApplication a(argc, argv);

   avdevice_register_all();
   av_register_all();
   avformat_network_init();

   MainWindow w;
   w.show();

   return a.exec();
}
