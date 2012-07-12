#include "player.h"

#include <iostream>
using namespace std;

Player::Player(QWidget* parent): QWidget(parent) { pkt = 0; }

Player::~Player() {
   cout << "Closing player" << endl;
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
   //QMutexLocker l(&m);
   if(streams[pkt->stream_index]->info().type == Video) {
      if(this->pkt) { av_free_packet(this->pkt); this->pkt=0; }
      this->pkt = pkt;
      update();
   }
}

void Player::paintEvent(QPaintEvent*) {
   //QMutexLocker l(&m);
   if(!pkt) return;
   QPainter painter(this);
   QImage image = QImage(pkt->data, this->width(), this->height(), QImage::Format_RGB32);
   painter.drawImage(QPointF(0,0),image);
   //av_free_packet(pkt);
}
