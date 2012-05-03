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
#include <QTimer>
#include <QObject>

#include <iostream>
using namespace std;

struct WebcamParams {
   char* video_size;
   char* device_name;
   int fps;
};

WebcamParams defaultParams();

class Webcam: public QObject {
    Q_OBJECT
public:
    static char* listDevices();
    Webcam(WebcamParams params);
    ~Webcam();
signals:
    void frameArrived(AVFrame* frame);
private:
    WebcamParams params;

    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVCodec* decoder

    void mainLoop();

    SwsContext* convertQt;
    SwsContext* convertStream;

    int pts;
};

struct ServerParams {
   char* video_size;
   int port;
   int fps;
   char* format;
   char* codec;
}

class Server: public QObject {
   Q_OBJECT
public:
   Server(ServerParams params);
   ~Server();
public slots:
   void onFrameArrived(AVFrame* frame);
private:
   ServerParams params;
   AVFormatContext* outputFormat;
   AVCodecContext* outputCodec;
   AVStream* video_st;
}

#endif // FFWEBCAM_H
