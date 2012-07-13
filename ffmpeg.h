#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libswresample/swresample.h>
   //#include <libavutil/opt.h>
}

#include <QMutexLocker>
#include <QFuture>
#include "logging.h"

//#include <iostream>
//using namespace std;

enum MediaType { Video, Audio, Other };

struct StreamInfo {
   MediaType type;
   int bitrate;
   union {
      struct {
         int width;
         int height;
         PixelFormat pixelFormat;
         int fps;
      } video;
      struct {
         int64_t channelLayout;
         AVSampleFormat sampleFormat;
         int sampleRate;
      } audio;
   };
};

class Output {
public:
   class Stream {
   public:
      //TODO: Actually it uses pixelFormat from time to time
      //Doesn't use pixelFormat and sampleFormat from StreamInfo
      //if encoder is 0, uses x264 for video, mp2 for audio
      //Throws 1 if can't associate codec context with encoder
      Stream(StreamInfo,AVCodec** encoder,Output* owner,int index) throw(int);
      ~Stream();
      void process(AVFrame*);
      StreamInfo info();
      //int getIndex();
      AVCodecContext* getCodec();
   private:
      //Returns 0 on fail
      AVPacket* encode(AVFrame*);
      void sendToOwner(AVPacket*);
      MediaType type;
      union {
         SwsContext* scaler;
         SwrContext* resampler;
      };
      AVCodecContext* codec;
      Output* owner;
      int index;
   };

   virtual ~Output();
   QList<Stream*> getStreams() const;
   virtual Stream* addStream(StreamInfo) = 0;
   virtual void sendPacket(AVPacket*) = 0;
protected:
   QList<Stream*> streams;
};

class Input {
public:
   enum State { Playing, Paused };
   class Stream {
   public:
      Stream(MediaType,AVCodecContext*);
      ~Stream();
      void process(AVPacket*);
      void subscribe(Output::Stream*);
      void unsubscribe(Output::Stream*);
      QList<Output::Stream*> getSubscribers() const;
      StreamInfo info();
      //Input* getOwner();
   private:
      void broadcast(AVFrame*);
      AVFrame* decode(AVPacket*);
      MediaType type;
      AVCodecContext* codec;
      QList<Output::Stream*> subscribers;
      Input* owner;
      int64_t pts;
   };

   virtual ~Input();
   QList<Stream*> getStreams() const;
   State getState();
   void setState(State state);
protected:
   QMutex m;
   State state;
   QList<Stream*> streams;
   QFuture<void> workerFuture;
   virtual void worker() = 0;
   QString fmt;
};

class InputGeneric: public Input {
public:
   InputGeneric(QString fmt, QString file);
   ~InputGeneric();
private:
   QMutex closingMutex;
   AVFormatContext* format;
   void worker();
};

//Default video is x264 baseline, default audio is MP2.
//Container is mpegts
class OutputGeneric: public Output {
public:
   OutputGeneric(QString fmt, QString file);
   ~OutputGeneric();
   Stream* addStream(StreamInfo);
   void sendPacket(AVPacket*);
private:
   AVFormatContext* format;
};

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
