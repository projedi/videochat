#include "player.h"
#include <QPainter>

#include <iostream>
using namespace std;

Player::Player(QWidget* parent): QWidget(parent) {
   connect(&timer,SIGNAL(timeout()),this,SLOT(repaint()));
   timer.start(40);
   videoPacket = 0;
   stab = 0;
   convertToBGR = 0;
   convertToRGB = 0;
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
      stream = new FFmpegOutputStream(info,encoder,this,streams.count());
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
      stabilize(pkt);
      videoPacket = pkt;
      paintMutex.unlock();
   }
}

void Player::reset() {
   videoPacket = 0;
}

void Player::setStabilizing(int state) {
   stabLocker.lock();
   if(state == (int)Qt::Checked) {
      qDebug("Setting up stabilizer");
      stab = new Stabilizer(width(),height());
   } else if(state == (int)Qt::Unchecked) {
      qDebug("Removing stabilizer");
      delete stab;
      stab = 0;
   } else {
      qWarning("Unsupported stabilizing state: %d", state);
   }
   stabLocker.unlock();
}

void Player::paintEvent(QPaintEvent*) {
   paintMutex.lock();
   QPainter painter(this);
   if(videoPacket) {
      QImage image = QImage( videoPacket->data, this->width(), this->height()
                           , QImage::Format_RGB32);
      painter.drawImage(QPointF(0,0),image);
   } else {
      painter.setPen(Qt::NoPen);
      painter.drawRect(this->rect());
   }
   paintMutex.unlock();
}

void Player::stabilize(AVPacket *pkt) {
   if(!stab) return;
   stabLocker.lock();
   AVFrame *frame = avcodec_alloc_frame();
   AVFrame *bgrframe = avcodec_alloc_frame();
   AVFrame *bgrstabframe = avcodec_alloc_frame();
   AVFrame *stabframe = avcodec_alloc_frame();
   frame->width = bgrframe->width = bgrstabframe->width = stabframe->width = width();
   frame->height = bgrframe->height = bgrstabframe->height = stabframe->height = height();
   frame->format = stabframe->format = (int)PIX_FMT_RGB32;
   bgrframe->format = bgrstabframe->format = (int)PIX_FMT_BGR24;
   frame->linesize[0] = stabframe->linesize[0] = width()*4;
   bgrframe->linesize[0] = bgrstabframe->linesize[0] = width()*3;
   avpicture_alloc((AVPicture*)bgrframe, PIX_FMT_BGR24, width(), height());
   avpicture_alloc((AVPicture*)stabframe, PIX_FMT_RGB32, width(), height());
   convertToBGR = sws_getCachedContext( convertToBGR, width(), height(), PIX_FMT_RGB32
                                      , width(), height(), PIX_FMT_BGR24, SWS_BICUBIC
                                      , 0, 0, 0);
   convertToRGB = sws_getCachedContext( convertToRGB, width(), height(), PIX_FMT_BGR24
                                      , width(), height(), PIX_FMT_RGB32, SWS_BICUBIC
                                      , 0, 0, 0);

   frame->data[0] = pkt->data;
   sws_scale( convertToBGR, frame->data, frame->linesize, 0, height()
            , bgrframe->data, bgrframe->linesize);
   stab->addFrame(bgrframe->data[0]);
   bgrstabframe->data[0] = (uchar*) stab->getStabilizedImage();
   sws_scale( convertToRGB, bgrstabframe->data, bgrstabframe->linesize, 0, height()
            , stabframe->data, stabframe->linesize);
   pkt->data = stabframe->data[0];

   avpicture_free((AVPicture*)bgrframe);
   av_free(bgrframe);
   avpicture_free((AVPicture*)frame);
   av_free(frame);
   stabLocker.unlock();
}
