#include "ffmpeg.h"

#include <iostream>
using namespace std;

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
   QPair<QString,QString> device = QPair<QString,QString>("video=Venus USB2.0 Camera","Default webcam");
   cameras.append(device);
#endif
   QList< QPair<QString,QString> >::iterator i;
   for(i=cameras.begin();i!=cameras.end();i++) {
      Input* input = new InputGeneric(fmt,(*i).first);
      inputs.append(input);
      streams.append(input->getStreams());
   }
}

VideoHardware::~VideoHardware() { }

QList<QString> VideoHardware::getDevices() {
   QList<QString> out;
   QList< QPair<QString,QString> >::iterator i;
   for(i = cameras.begin(); i != cameras.end(); i++)
      out.append((*i).second);
   return out;
}

//TODO: fire up only currently up devices
void VideoHardware::worker() {
   QList<Input*>::iterator i;
   for(i=inputs.begin();i!=inputs.end();i++) {
      (*i)->setState(Playing);
   }
   /*
   QList<Stream*>::iterator i;
   while(state != Paused) {
      for(i=streams.begin();i!=streams.end();i++) {
         if((*i)->getSubscribers().count() > 0) (*i)->getOwner()->setState(Playing);
         else (*i)->getOwner()->setState(Paused);
      }
   }
   */
}
