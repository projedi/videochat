#include "player.h"

Player::Player(QWidget* parent): QWidget(parent) { }

Player::~Player() { }

Output::Stream* Player::addStream(StreamInfo info) {
   if(info.type != Video) return 0;
   AVCodec* decoder = avcodec_find_decoder(CODEC_ID_RAWVIDEO);
   Stream* stream = new Stream(info,&decoder,this,0);
   streams.append(stream);
   return stream;
}

void Player::sendPacket(AVPacket* pkt) {
   if(streams[pkt->stream_index]->info().type == Video) {
      image = QImage(pkt->data, this->width(), this->height(), QImage::Format_RGB32);
      update();
   }
   av_free_packet(pkt);
}

void Player::paintEvent(QPaintEvent*) {
   QPainter painter(this);
   painter.drawImage(QPointF(0,0),image);
}
