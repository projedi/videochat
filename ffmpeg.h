#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libswresample/swresample.h>
   //#include <libavutil/opt.h>
}

#include <QObject>

//#include <QImage>
//#include <QObject>
//#include <QString>
//#include <QtConcurrentRun>
//#include <QSharedData>
//#include <QFutureWatcher>
//#include <QMutex>
//#include <QMutexLocker>

#include <iostream>
using namespace std;

//Simple reference counting for AVFrame
class QAVFrame {
public:
   QAVFrame() { cout << "QAVFrame constructed with no arguments" << endl; }
   QAVFrame(AVFrame* frame){ d = frame; c = new int(1); }
   QAVFrame(const QAVFrame &o) { QMutexLocker l(&m); c = o.c; (*c)++; d = o.d; }
   //TODO: Check if d structure leaks here since I don't do delete.
   ~QAVFrame() { QMutexLocker l(&m); (*c)--; if(*c == 0) { delete c; av_free(d); } }
   AVFrame* data() const { return d; }
private:
   AVFrame* d;
   int *c;
   QMutex m;
};

class FFParams {
public:
   virtual int width() = 0;
   virtual int height() = 0;
   virtual PixelFormat pixelFormat() = 0;
   virtual int64_t channelLayout() = 0;
   virtual AVSampleFormat sampleFormat() = 0;
   virtual int sampleRate() = 0;
}

class FFSource: public QObject, public FFParams {
   Q_OBJECT
signals:
   void onNewVideoFrame(QAVFrame frame);
   void onNewAudioFrame(QAVFrame frame);
};

class FFSink: public QObject, public FFParams {
   Q_OBJECT
public:
   explicit FFSink(QObject *parent = 0): QObject(parent) { }
   ~FFSink() { clearSource(); }
   void setSource(FFSource* src);
   void clearSource();
private slots:
   virtual void newVideoFrame(QAVFrame frame) = 0;
   virtual void newAudioFrame(QAVFrame frame) = 0;
private:
   QAVFrame rescale(QAVFrame frame);
   QAVFrame resample(QAVFrame frame);
   FFSource* source;
   SwsContext* scaler;
   SwrContext* resampler;
};

class FFHardware {
   virtual QList<QPair<QString,QString> > cameras() = 0;
   virtual QList<QPair<QString,QString> > microphones() = 0;
};

// Not to clutter includes for users
#include "ffmpeg/alsav4l2.h"
#include "ffmpeg/client.h"
#include "ffmpeg/player.h"
#include "ffmpeg/server.h"
