#include "logging.h"
#include <fstream>
#include <ctime>

using namespace std;

namespace logging {
   static fstream logger;
   static clock_t startTime;
}

void logger(QString str) {
   clock_t curTime = clock();
   double timestamp = ((double)(curTime - logging::startTime)) / CLOCKS_PER_SEC;
   logging::logger << timestamp << ": " << str.toAscii().data() << endl;
}

void initLogging() {
#if defined(LINUX)
   char *filename = "/tmp/videochat.log";
#elif defined(WIN32)
   char *filename = "videochat.log";
#endif
   logging::logger.open(filename,fstream::out);
   logging::startTime = clock();
}
