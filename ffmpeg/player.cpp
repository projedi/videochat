#include "player.h"

Player::Player(QWidget *parent) { }

Player::~Player() { } 

void Player::newVideoFrame(QAVFrame frame) {
   image = QImage( frame.data()->data[0], frame.data()->width, frame.data()->height
                 , QImage::Format_RGB32);
   update();
}

void Player::newAudioFrame(QAVFrame frame) { }

void Player::paintEvent(QPaintEvent*) {
   QPainter painter(this);
   painter.drawImage(QPointF(0,0),image);
}
