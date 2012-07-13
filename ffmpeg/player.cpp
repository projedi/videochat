#include "player.h"

#include <iostream>
using namespace std;

Player::Player(QWidget* parent): QWidget(parent) {
   connect(&t,SIGNAL(timeout()),this,SLOT(repaint()));
   t.start(40);
   pkt = 0;
}

Player::~Player() {
   logger("Closing player");
   if(pkt) av_free_packet(pkt);
}

Output::Stream* Player::addStream(StreamInfo info) {
   if(info.type != Video) return 0;
   AVCodec* encoder = avcodec_find_encoder(CODEC_ID_RAWVIDEO);
   info.video.width = this->width();
   info.video.height = this->height();
   info.video.pixelFormat = PIX_FMT_RGB32;
   Stream* stream = new Stream(info,&encoder,this,0);
   streams.append(stream);
   return stream;
}

void Player::sendPacket(AVPacket* pkt) {
   logger("Sending packet");
   if(streams[pkt->stream_index]->info().type == Video) {
      logger("Sending video packet");
      m.lock();
      logger("locking for sendPacket");
      if(this->pkt) { logger("Clearing current packet"); av_free_packet(this->pkt); this->pkt=0; }
      this->pkt = pkt;
      //logger("Making it paint");
      m.unlock();
      logger("sendPacket unlocking");
      //repaint();
      //update();
      //logger("Made it paint");
   }
}

void Player::paintEvent(QPaintEvent*) {
   m.lock();
   logger("paintEvent locking");
   if(!pkt) { logger("Not really painting"); m.unlock(); return; }
   logger("Painting for real");
   QPainter painter(this);
   logger( "For " + QString::number(this->width()) + "x" + QString::number(this->height())
         + " I get packet with length " + QString::number(pkt->size));
   QImage image = QImage(pkt->data, this->width(), this->height(), QImage::Format_RGB32);
   painter.drawImage(QPointF(0,0),image);
   m.unlock();
   logger("paintEvent unlocking");
}
