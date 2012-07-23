#pragma once

#include "ffmpeg.h"
#include <QWidget>
#include <QTimer>

class Player: public Output, public QWidget {
public:
   Player(QWidget* parent = 0);
   ~Player();
   OutputStream* addStream(StreamInfo);
   void removeStream(OutputStream*);
   void sendPacket(AVPacket*);
   void reset();
protected:
   void paintEvent(QPaintEvent*);
private:
   AVPacket* videoPacket;
   QTimer timer;
   QMutex paintMutex;
};
