#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libswresample/swresample.h>
   #include <libavutil/opt.h>
}

//#include <QImage>
#include <QObject>
#include <QString>
#include <QtConcurrentRun>
#include <QSharedData>
//#include <QFutureWatcher>
#include <QMutex>
#include <QMutexLocker>

#include <iostream>
using namespace std;

//TODO: Have a philosophical talk with yourself on whether you
//TODO: can allow abstract classes - not just interfaces

//Simple reference counting for AVFrame
class QAVFrame {
public:
   QAVFrame() { d = avcodec_alloc_frame(); c = new int(1); }
   QAVFrame(AVFrame* frame){ d = frame; c = new int(1); }
   QAVFrame(const QAVFrame &other) { QMutexLocker l(&m); c = other.c; (*c)++; d = other.d; }
   ~QAVFrame() { QMutexLocker l(&m); (*c)--; if(*c == 0) { delete c; av_free(d); }}
   AVFrame* data() const { return d; }
private:
   AVFrame* d;
   int *c;
   QMutex m;
};

//TODO: It doesn't really require a "Device". Use more generic term.
class FFDevice {
public:
   virtual int width() = 0; 
   virtual int height() = 0; 
   virtual PixelFormat pixelFormat() = 0; 
   virtual int64_t channelLayout() = 0;
   virtual AVSampleFormat sampleFormat() = 0;
   virtual int sampleRate() = 0;
};

class FFSource: public QObject, public FFDevice {
   Q_OBJECT
signals:
   void onNewVideoFrame(QAVFrame frame);
   void onNewAudioFrame(QAVFrame frame);
};

class FFSink: public QObject, public FFDevice {
   Q_OBJECT
public slots:
   virtual void newVideoFrame(QAVFrame frame) = 0;
   virtual void newAudioFrame(QAVFrame frame) = 0;
};

class FFConnector: public QObject {
   Q_OBJECT
public:
   explicit FFConnector(QObject* parent = 0): QObject(parent) { }
   ~FFConnector() { ffDisconnect(); }
   void ffConnect(FFSource* source, FFSink* sink);
   void ffDisconnect();
private slots:
   void newVideoFrame(QAVFrame frame);
   void newAudioFrame(QAVFrame frame);
private:
   SwsContext* scaler;
   SwrContext* resampler;
   FFSource* source;
   FFSink* sink;
   //TODO: they shoudln't be here. Find nice copying of AVFrame/AVPicture.
   int w;
   int h;
   PixelFormat pf;
};

class FFHardware {
   virtual QList<QPair<QString,QString> > cameras() = 0;
   virtual QList<QPair<QString,QString> > microphones() = 0;
};

// Not to clutter includes for users
#include "ffmpeg/alsav4l2.h"
//#include "ffmpeg/client.h"
#include "ffmpeg/player.h"
//#include "ffmpeg/server.h"
