#include "logging.h"

void logger(QString str) {
   clock_t curTime = clock();
   double timestamp = ((double)(curTime - logging::startTime)) / CLOCKS_PER_SEC;
   logging::logger << timestamp << ": " << str.toAscii().data() << endl;
}

void init() {
   logging::logger.open("/tmp/videochat.log",fstream::out);
   logging::startTime = clock();
}
