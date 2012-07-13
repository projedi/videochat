#pragma once

#include <fstream>
#include <ctime>
#include <QString>

using namespace std;

namespace logging {
   static fstream logger;
   static clock_t startTime;
}

void logger(QString str);
void init();

