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

//TODO: Change index of all streams greater than removed
void Player::removeStream(Output::Stream* stream) {
   int strIndex = streams.indexOf(stream);
   delete stream;
   streams.removeAt(strIndex);
}

void Player::sendPacket(AVPacket* pkt) {
   if(pkt->stream_index >= streams.count()) return;
   if(streams[pkt->stream_index]->info().type == Video) {
      m.lock();
      if(this->pkt) { av_free_packet(this->pkt); this->pkt=0; }
      this->pkt = pkt;
      m.unlock();
   }
}

void Player::paintEvent(QPaintEvent*) {
   m.lock();
   if(!pkt) { m.unlock(); return; }
   QPainter painter(this);
   QImage image = QImage(pkt->data, this->width(), this->height(), QImage::Format_RGB32);
   painter.drawImage(QPointF(0,0),image);
   m.unlock();
}
