#ifndef FFWEBCAM_H
#define FFWEBCAM_H

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

class FFVideoDevice: public QObject {
   Q_OBJECT
public:
   virtual int getWidth() = 0; 
   virtual int getHeight() = 0; 
   virtual PixelFormat getPixelFormat() = 0; 
};

class FFSource: public FFVideoDevice {
   Q_OBJECT
signals:
   void onNewVideoFrame(AVFrame* frame);
   void onNewAudioFrame(AVFrame* frame);
};

class FFSink: public FFVideoDevice {
   Q_OBJECT
public slots:
   virtual void newVideoFrame(AVFrame* frame) = 0;
   virtual void newAudioFrame(AVFrame* frame) = 0;
};

struct Device {
   QString name;
   QString ffmpegName;
   //QString ffmpegFormat;
};

// ALSA + Video4Linux2 as one input device
class ALSAV4L2: public FFSource {
   Q_OBJECT
public:
   ALSAV4L2(QString cameraName, QString microphoneName);
   ALSAV4L2(Device camera, Device microphone): this(camera.ffmpegName
                                                   ,microphone.ffmpegName);
   ~ALSAV4L2();
   //TODO: I want these to be inheritable
   static QList<Device> getCameraDevices();
   static QList<Device> getMicrophoneDevices();
   int getWidth() { QMutexLocker(&initLocker); return videoCodec->width; }
   int getHeight() { QMutexLocker(&initLocker); return videoCodec->height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return videoCodec->pix_fmt; }
private:
   AVFormatContext* videoFormat;
   AVFormatContext* audioFormat;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;
   QMutex initLocker;
   void init(QString cameraName, QString microphoneName);
   void videoWorker();
   void audioWorker();
}

//TODO: It can't dynamically resized. Bad.
//TODO: What if I want different audio renderings on different platforms? For some reason.
class Player: public FFSink {
   Q_OBJECT
public:
   Player(int width, int height, QString audioOutput);
   ~Player();
   void newVideoFrame(AVFrame* frame);
   void newAudioFrame(AVFrame* frame);
   int getWidth() { QMutexLocker(&initLocker); return codec->width; }
   int getHeight() { QMutexLocker(&initLocker); return codec->height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return codec->pix_fmt; }
signals:
   void onNewFrame(QPixmap frame);
private:
   AVFormatContext* audioFormat;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;
   QMutex initLocker;
};

//TODO: Client & Server? Seriously? WHY U NO USE PROPER NAMING?
class Client: public FFSource {
   Q_OBJECT
public:
   Client(QString port);
   ~Client();
   int getWidth() { QMutexLocker(&initLocker); return videoCodec->width; }
   int getHeight() { QMutexLocker(&initLocker); return videoCodec->height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return videoCodec->pix_fmt; }
private:
   AVFormatContext* videoFormat;
   AVFormatContext* audioFormat;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;
   QMutex initLocker;
   void init(QString cameraName, QString microphoneName);
   void videoWorker();
   void audioWorker();
};

class Server: public FFSink {
   Q_OBJECT 
public:
   Server(QString port);
   ~Server();
   void newVideoFrame(AVFrame* frame);
   void newAudioFrame(AVFrame* frame);
   int getWidth() { QMutexLocker(&initLocker); return codec->width; }
   int getHeight() { QMutexLocker(&initLocker); return codec->height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return codec->pix_fmt; }
private:
   AVFormatContext* format;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;
   QMutex initLocker;
}

#endif // FFWEBCAM_H
