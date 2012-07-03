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

#include <iostream>
using namespace std;

/*
//Simple reference counting for AVFrame
class QAVFrame {
public:
   QAVFrame() { cout << "QAVFrame constructed with no arguments" << endl; }
   QAVFrame(AVFrame* frame){ d = frame; c = new int(1); }
   QAVFrame(const QAVFrame &o) {
      QMutexLocker l(&m); c = o.c; (*c)++; d = o.d; t = o.t;
   }
   //TODO: Check if d structure leaks here since I don't do delete.
   ~QAVFrame() { QMutexLocker l(&m); (*c)--; if(*c == 0) { delete c; av_free(d); } }
   AVFrame* data() const { return d; }
   FrameType type() { return t; }
private:
   FrameType t;
   AVFrame* d;
   int *c;
   QMutex m;
};
*/

enum MediaType { Video, Audio };

//TODO: add EOF marker
//TODO: add NODATA marker
class QAVFrame {
public:
   QFrame();
   ~QFrame();
private:
   AVFrame* data;
   MediaType type;
   int destStream;
};

struct StreamInfo {
   MediaType type;
   AVCodecContext* codec;
   int index;
   union {
      struct {
         int width;
         int height;
         PixelFormat pixelFormat;
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
   virtual void newFrame(QAVFrame frame) = 0;
};

//TODO: There might be some races
class InputGeneric {
public:
   enum State { Running, Paused, Stopped };

   InputGeneric();
   InputGeneric(QString format, QString file);
   ~InputGeneric();
   void setSource(QString format, QString file);

   int streamCount();
   StreamInfo streamInfo(int stream);

   // Gives the current state. Won't tell if the state is changing due to setState.
   State state();
   // Will not react instantly.
   void setState(State state);

   //if destStream == -1 creates new
   //returns stream in destination or -1 if error
   int addOutput(Output* out, int srcStream, int destStream);
   //if srcStream == -1 removes all connections to this output
   void removeOutput(Output* out, int srcStream);
private:
   struct Stream {
      StreamInfo info;

      Output* output;
      int destIndex;
      union {
         SwsContext* scaler;
         SwrContext* resampler;
      };
   };
   AVFormatContext* format;
   QList<Stream> streams;

   //QFuture<void> initFuture;
   //void init(QString format, QString file);
   State curState;
   State wantedState;
   void worker();
};

//Default video is x264 baseline, default audio is AAC.
//Container is mpegts
class OutputGeneric: Output {
public:
   OutputGeneric();
   OutputGeneric(QString format, QString file);
   ~OutputGeneric();
   void setDestination(QString format, QString file);

   int streamCount();
   StreamInfo streamInfo(int stream);
   int addStream(StreamInfo info);
   //TODO: how should input react on that?
   int removeStream(StreamInfo info);

   void newFrame(QAVFrame frame);
private:
   AVFormatContext* format;
   QList<StreamInfo> streams;
   //QFuture<void> initFuture;
   //void init(QString format, QString file);
   void worker();
};

class Hardware {
   virtual QList< QPair<QString,QString> > cameras() = 0;
   virtual QList< QPair<QString,QString> > microphones() = 0;
};

// Not to clutter includes for users
//#include "ffmpeg/alsav4l2.h"
//#include "ffmpeg/client.h"
//#include "ffmpeg/player.h"
//#include "ffmpeg/server.h"
