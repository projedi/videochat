#include <QtGui/QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "ffmpeg.h"
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

namespace logging {
   static fstream logger;
   static clock_t startTime;

   void initLogging() {
#if defined(LINUX)
      char *filename = "/tmp/videochat.log";
#elif defined(WIN32)
      char *filename = "videochat.log";
#endif
      logging::logger.open(filename,fstream::out);
      logging::startTime = clock();
   }
}

void mesOutput(QtMsgType type, const char *msg) {
   double timestamp = ((double)(clock() - logging::startTime)) / CLOCKS_PER_SEC;
   logging::logger << timestamp << ": " << msg << endl;
   cerr << timestamp << ": " << msg << endl;
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
