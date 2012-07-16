#include "player.h"
#include <QPainter>

#include <iostream>
using namespace std;

Player::Player(QWidget* parent): QWidget(parent) {
   connect(&timer,SIGNAL(timeout()),this,SLOT(repaint()));
   timer.start(40);
   videoPacket = 0;
}

Player::~Player() {
   paintMutex.lock();
   if(videoPacket) av_free_packet(videoPacket);
   paintMutex.unlock();
}

OutputStream* Player::addStream(StreamInfo info) {
   OutputStream *stream = 0;
   if(info.type == Video) {
      int i;
      for(i = 0; i < streams.count(); i++) if(streams[i]->info().type == Video) break;
      if(i < streams.count()) return 0;
      AVCodec* encoder = avcodec_find_encoder(CODEC_ID_RAWVIDEO);
      info.video.width = this->width();
      info.video.height = this->height();
      info.video.pixelFormat = PIX_FMT_RGB32;
      info.bitrate = 0;
      stream = new OutputStream(info,encoder,this,streams.count());
      streams.append(stream);
   }
   return stream;
}

//TODO: Change index of all streams greater than removed
void Player::removeStream(OutputStream* stream) {
   int strIndex = streams.indexOf(stream);
   delete stream;
   streams.removeAt(strIndex);
}

void Player::sendPacket(AVPacket* pkt) {
   if(pkt->stream_index >= streams.count()) return;
   if(streams[pkt->stream_index]->info().type == Video) {
      paintMutex.lock();
      if(videoPacket) av_free_packet(videoPacket);
      videoPacket = pkt;
      paintMutex.unlock();
   }
}

void Player::paintEvent(QPaintEvent*) {
   paintMutex.lock();
   if(videoPacket) {
      QPainter painter(this);
      QImage image = QImage( videoPacket->data, this->width(), this->height()
                           , QImage::Format_RGB32);
      painter.drawImage(QPointF(0,0),image);
   }
   paintMutex.unlock();
}
