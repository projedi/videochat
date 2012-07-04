#pragma once

#include "ffmpeg.h"

class Player: Output, QWidget {
Q_OBJECT
public:
   Player(QWidget* parent = 0);
   ~Player();
   Stream* addStream(StreamInfo);
   void sendPacket(AVPacket*);
protected:
   void paintEvent(QPaintEvent*);
private:
   QImage image;
};
