#include "ffmpeg.h"

#include <iostream>
using namespace std;

Hardware::~Hardware() { }

QList<QString> Hardware::getNames() const { return names; }
QList<QString> Hardware::getFormats() const { return formats; }
QList<QString> Hardware::getFiles() const { return files; }

//TODO: Implement
VideoHardware::VideoHardware() {
#if defined(LINUX)
   names.append("Default webcam");
   formats.append("v4l2");
   files.append("/dev/video1");
#elif defined(WIN32)
   names.append("Default webcam");
   formats.append("dshow");
   files.append("video=Venus USB2.0 Camera");
#endif
}

//TODO: Implement
AudioHardware::AudioHardware() {
#if defined(LINUX)
   names.append("Default microphone");
   formats.append("alsa");
   files.append("hw:0");
#elif defined(WIN32)
   names.append("Default microphone");
   formats.append("dshow");
   files.append("audio=Venus USB2.0 Camera");
#endif
}
