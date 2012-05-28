#ifndef FFWEBCAM_H
#define FFWEBCAM_H

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libavutil/opt.h>
}

#include <QImage>
#include <QObject>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QMutexLocker>

#include <iostream>
using namespace std;

class Server: public QObject {
   Q_OBJECT
public:
   Server(QString name, QString fmt, CodecID codecID, int w, int h, int bitrate, int fps
         ,int gop_size);
   ~Server();
   int getWidth() { return codec->width; }
   int getHeight() { return codec->height; }
   PixelFormat getPixelFormat() { return codec->pix_fmt; }
public slots:
   void onFrame(AVFrame* frame);
private:
   AVCodecContext* codec;
   AVFormatContext* format;
   AVStream* video_st;
   int bufsize;
   AVPacket pkt;
   //bool isBusy;
   QMutex mutex;
};

class Client: public QObject {
   Q_OBJECT
public:
   Client(QString name, QString fmt);
   ~Client();
   int getWidth() { initFuture.waitForFinished(); return codec->width; }
   int getHeight() { initFuture.waitForFinished(); return codec->height; }
   PixelFormat getPixelFormat() { initFuture.waitForFinished(); return codec->pix_fmt; }
signals:
   void newFrame(AVFrame* frame);
private:
   void init();
   void worker();

   QFutureWatcher<void> initFuture;
   QFutureWatcher<void> workerFuture;
   AVFormatContext* format;
   AVCodecContext* codec;
   bool isStopped;
   int pts;
};

#endif // FFWEBCAM_H
