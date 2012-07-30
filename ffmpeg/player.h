#pragma once

#include "ffmpeg.h"
#include <QWidget>
#include <QTimer>
#include <stabilization.h>

class Player: public QWidget, public Output {
   Q_OBJECT
public:
   Player(QWidget* parent = 0);
   ~Player();
   OutputStream* addStream(StreamInfo);
   void removeStream(OutputStream*);
   void sendPacket(AVPacket*);
   void reset();
public slots:
   void setStabilizing(int);
protected:
   void paintEvent(QPaintEvent*);
private:
   void stabilize(AVPacket* pkt);
   AVPacket* videoPacket;
   QTimer timer;
   QMutex paintMutex;
   QMutex stabLocker;
   Stabilizer* stab;
   SwsContext* convertToBGR;
   SwsContext* convertToRGB;
};
