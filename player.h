#pragma once

#include "ffmpeg.h"
#include <QWidget>
#include <QTimer>

class Player: public Output, public QWidget {
public:
   Player(QWidget* parent = 0);
   ~Player();
   Stream* addStream(StreamInfo);
   void removeStream(Stream*);
   void sendPacket(AVPacket*);
protected:
   void paintEvent(QPaintEvent*);
private:
   AVPacket* videoPacket;
   QTimer timer;
   QMutex paintMutex;
};
