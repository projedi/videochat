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
//TODO: can allow inheriting fields and not just methods.

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
   void onNewVideoFrame(AVFrame* frame);
   void onNewAudioFrame(AVFrame* frame);
};

class FFSink: public FFDevice {
   Q_OBJECT
public slots:
   virtual void newVideoFrame(AVFrame* frame) = 0;
   virtual void newAudioFrame(AVFrame* frame) = 0;
};

class FFConnector: public QObject {
   Q_OBJECT
public:
   VideoFrameScaler(FFSource source, FFSink sink);
   ~VideoFrameScaler();
private slots:
   void newVideoFrame(AVFrame* frame);
   void newAudioFrame(AVFrame* frame);
private:
   SwsContext* scaler;
   SwrContext* resampler;
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
