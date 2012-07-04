#pragma once

#include "ffmpeg.h"
#include <QPainter>
#include <QWidget>

class Player: public Output, public QWidget {
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
