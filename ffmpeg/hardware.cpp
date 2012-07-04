#include "ffmpeg.h"

//TODO: implement
AudioHardware::AudioHardware() {
#if defined(LINUX)
   QString fmt = "alsa";
   //QPair<QString,QString> device = qMakePair("hw:0","Default microphone");
   //microphones.append(device);
#elif defined(WIN32)
   QString fmt = "dshow";
#endif
   QList< QPair<QString,QString> >::iterator i;
   for(i=microphones.begin();i!=microphones.end();i++) {
      Input* input = new InputGeneric(fmt,(*i).first);
      inputs.append(input);
      streams.append(input->getStreams());
   }
}

AudioHardware::~AudioHardware() { }

void AudioHardware::worker() {
   QList<Stream*>::iterator i;
   while(state != Paused) {
      for(i=streams.begin();i!=streams.end();i++) {
         if((*i)->getSubscribers().count() > 0) (*i)->getOwner()->setState(Playing);
         else (*i)->getOwner()->setState(Paused);
      }
   }
}

VideoHardware::VideoHardware() {
#if defined(LINUX)
   QString fmt = "v4l2";
   QPair<QString,QString> device = QPair<QString,QString>("/dev/video0","Default webcam");
   cameras.append(device);
#elif defined(WIN32)
   QString fmt = "dshow";
#endif
   QList< QPair<QString,QString> >::iterator i;
   for(i=cameras.begin();i!=cameras.end();i++) {
      Input* input = new InputGeneric(fmt,(*i).first);
      inputs.append(input);
      streams.append(input->getStreams());
   }
}

VideoHardware::~VideoHardware() { }

void VideoHardware::worker() {
   QList<Stream*>::iterator i;
   while(state != Paused) {
      for(i=streams.begin();i!=streams.end();i++) {
         if((*i)->getSubscribers().count() > 0) (*i)->getOwner()->setState(Playing);
         else (*i)->getOwner()->setState(Paused);
      }
   }
}
