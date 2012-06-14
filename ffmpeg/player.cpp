#include "player.h"

Player::Player(int width, int height, QString audioOutput) {
   QtConcurrent::run(this, &Player::init, width, height, audioOutput);
}

Player::~Player() {
   //avcodec_close(audioCodec);
   //avformat_free_context(audioFormat);
   //TODO: Where will it SEGFAULT?
   //av_free(audioCodec);
   //av_free(audioFormat);
}

void Player::newVideoFrame(AVFrame* frame) {
   QImage img(frame->data[0],frame->width,frame->height,QImage::Format_RGB32);
   emit onNewFrame(QPixmap::fromImage(img));
   //TODO: What to do with frame?
}

//TODO: Add audio output
void Player::newAudioFrame(AVFrame* frame) {

}

void Player::init(int width, int height, QString audioOutput) {
   QMutexLocker(&initLocker);
   //TODO: Add audio output
   //QByteArray audioName = audioOutput.toAscii();

   this->width = width;
   this->height = height;
}
