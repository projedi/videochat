#include "player.h"

Player::Player(int width, int height) {
   cout << "Player is constructing with: " << width << "x" << height << endl;
   initFuture = QtConcurrent::run(this, &Player::init, width, height);
}

Player::~Player() {
   //avcodec_close(audioCodec);
   //avformat_free_context(audioFormat);
   //TODO: Where will it SEGFAULT?
   //av_free(audioCodec);
   //av_free(audioFormat);
}

void Player::newVideoFrame(QAVFrame frame) {
   QImage img(frame.data()->data[0],frame.data()->width,frame.data()->height,QImage::Format_RGB32);
   emit onNewFrame(QPixmap::fromImage(img));
}

void Player::newAudioFrame(QAVFrame frame) {

}

void Player::init(int width, int height) {
   //TODO: Add audio output
   //QByteArray audioName = audioOutput.toAscii();

   this->w = width;
   this->h = height;
}

/*
void audioCallback(void* opaque, Uint8* stream, int len) {
   
}

void openAudio() {
   const char* env = SDL_getenv("SDL_AUDIO_CHANNELS");
   if(env) {
      spec.channels = SDL_atoi(env);  
   } else {
      spec.channels = 2;
   }
   spec.format = AUDIO_S16SYS;
   spec.samples = 1024;
   spec.userdata = 0;
   spec.silence = 0;
   spec.freq = 44100;
   spec.callback = audioCallback;
   if(SDL_OpenAudio(&spec,0) < 0)
      cout << "Can't open audio: " << SDL_GetError() << endl;
}
*/
