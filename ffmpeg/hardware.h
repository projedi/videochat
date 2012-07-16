#pragma once

#include "ffmpeg.h"

class Hardware {
public:
   virtual ~Hardware();
   QList<QString> getNames() const;
   QList<QString> getFormats() const;
   QList<QString> getFiles() const;
protected:
   QList<QString> names;
   QList<QString> formats;
   QList<QString> files;
};

class VideoHardware: public Hardware {
public:
   VideoHardware();
};

class AudioHardware: public Hardware {
public:
   AudioHardware();
};
