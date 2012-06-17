#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libavutil/opt.h>
}

//#include <QImage>
#include <QObject>
#include <QString>
#include <QtConcurrentRun>
//#include <QFutureWatcher>
#include <QMutex>
#include <QMutexLocker>

#include <iostream>
using namespace std;

//TODO: Have a philosophical talk with yourself on whether you
//TODO: can allow inheriting fields, not just methods.

//Simple reference counting for AVFrame
//TODO: If SEGFAULT comes, add debug output.
class QAVFrame {
public:
   //TODO: Do I really need that shortcut?
   QAVFrame(): QAVFrame(avcodec_alloc_frame()) { }
   QAVFrame(AVFrame* frame) { data = frame; refcount = new int(1); }
   QAVFrame(const QAVFrame& other) { QMutexLocker(&mutex); (*count)++; data=other.data; }
   ~QAVFrame() { QMutexLocker(&mutex); (*count)--; if(*count <= 0) av_free(data); }
   AVFrame* getData() { QMutexLocker(&mutex); return data; }
private:
   int* count;
   AVFrame* data;
   QMutex mutex;
}

//TODO: It doesn't really require a "Device". Use more generic term.
class FFDevice: public QObject {
   Q_OBJECT
public:
   virtual int getWidth() = 0; 
   virtual int getHeight() = 0; 
   virtual PixelFormat getPixelFormat() = 0; 
   virtual int64_t getChannelLayout() = 0;
   virtual AVSampleFormat getSampleFormat() = 0;
   virtual int getSampleRate() = 0;
};

class FFSource: public FFDevice {
   Q_OBJECT
signals:
   void onNewVideoFrame(QAVFrame frame);
   void onNewAudioFrame(QAVFrame frame);
};

class FFSink: public FFDevice {
   Q_OBJECT
public slots:
   virtual void newVideoFrame(QAVFrame frame) = 0;
   virtual void newAudioFrame(QAVFrame frame) = 0;
};

class FFConnector: public QObject {
   Q_OBJECT
public:
   FFConnector(FFSource* source, FFSink* sink);
   ~FFConnector();
private slots:
   void newVideoFrame(QAVFrame frame);
   void newAudioFrame(QAVFrame frame);
private:
   SwsContext* scaler;
   SwrContext* resampler;
   //TODO: they shoudln't be here. Find nice copying of AVFrame/AVPicture.
   int w;
   int h;
   PixelFormat pf;
}

//TODO: Seems redundant. Doesn't it?
struct Device {
   QString name;
   QString ffmpegName;
};

// Not to clutter includes for users
#include "ffmpeg/alsav4l2.h"
#include "ffmpeg/client.h"
#include "ffmpeg/player.h"
#include "ffmpeg/server.h"
