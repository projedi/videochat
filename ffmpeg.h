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
#include <QSharedData>
//#include <QFutureWatcher>
#include <QMutex>
#include <QMutexLocker>

#include <iostream>
using namespace std;

//TODO: Have a philosophical talk with yourself on whether you
//TODO: can allow abstract classes - not just interfaces

class AVFrameData: public QSharedData {
public:
   AVFrameData() { }
   AVFrameData(const AVFrameData &other): QSharedData(other), data(other.data) { }
   ~AVFrameData() { }
private:
   AVFrame* data;
};

//Simple reference counting for AVFrame
class QAVFrame {
public:
   QAVFrame() { d = new AVFrameData(); }
   QAVFrame(AVFrame* frame){ d = new AVFrameData(); setData(frame); }
   AVFrame* data() const { return d->data; }
   void setData(AVFrame* frame) { d->data = frame; }
private:
   QSharedDataPointer<AVFrameData> d;
}

//TODO: It doesn't really require a "Device". Use more generic term.
class FFDevice {
public:
   virtual int width() const = 0; 
   virtual int height() const = 0; 
   virtual PixelFormat pixelFormat() const = 0; 
   virtual int64_t channelLayout() const = 0;
   virtual AVSampleFormat sampleFormat() const = 0;
   virtual int sampleRate() const = 0;
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
   //TODO: they shoudln't be here. Find nice copying of AVFrame/AVPicture.
   int w;
   int h;
   PixelFormat pf;
}

class FFHardware {
   virtual QList<QPair<QString,QString>> cameras() const = 0;
   virtual QList<QPair<QString,QString>> microphones() const = 0;
};

// Not to clutter includes for users
#include "ffmpeg/alsav4l2.h"
#include "ffmpeg/client.h"
#include "ffmpeg/player.h"
#include "ffmpeg/server.h"
