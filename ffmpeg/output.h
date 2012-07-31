#pragma once

#include "ffmpeg.h"

class OutputStream {
public:
   virtual ~OutputStream() { }
   virtual void process(AVFrame* frame) = 0;
   virtual StreamInfo info() = 0;
};

class FFmpegOutputStream: public OutputStream {
public:
   enum State { Opened, Closed };
   //if encoder is 0, uses x264 for video, mp2 for audio
   FFmpegOutputStream(StreamInfo,AVCodec* encoder,Output* owner,int index);
   ~FFmpegOutputStream();
   StreamInfo info();
   State getState();
   void process(AVFrame*);
   AVCodecContext* getCodec();
private:
   MediaType type;
   union {
      SwsContext* scaler;
      SwrContext* resampler;
   };
   AVCodecContext* codec;
   Output* owner;
   int index;
   State state;
};

class Output {
public:
   virtual ~Output();
   QList<OutputStream*> getStreams() const;
   virtual OutputStream* addStream(StreamInfo) = 0;
   virtual void removeStream(OutputStream*) = 0;
   virtual void sendPacket(AVPacket*) = 0;
protected:
   QList<OutputStream*> streams;
};

//Default video is x264 baseline, default audio is MP2.
//Container is mpegts
class OutputGeneric: public Output {
public:
   OutputGeneric(QString fmt, QString file);
   ~OutputGeneric();
   OutputStream* addStream(StreamInfo);
   void removeStream(OutputStream*);
   void sendPacket(AVPacket*);
private:
   AVFormatContext* format;
};
