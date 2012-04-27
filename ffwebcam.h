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
    int device_id;
    int fps;
    int bufferSize;
    int port;
};

WebcamParams defaultParams();

/*
class FFSinkInput: public QObject {
   Q_OBJECT
public:
   FFSinkInput();
   ~FFSinkInput();
private:
};
*/

class Webcam: public QObject {
    Q_OBJECT
public:
    Webcam(WebcamParams params);
    ~Webcam();
    void start();
    void stop();
signals:
    void frameArrived(QImage* frame);
private slots:
    void grabTimerTimeout();
private:
    QTimer* grabTimer;
    WebcamParams params;

    AVFormatContext* inputFormat;
    AVCodecContext* inputCodec;

    AVFormatContext* outputFormat;
    AVCodecContext* outputCodec;
    AVStream* video_st;

    SwsContext* convertQt;
    SwsContext* convertStream;

    int pts;
};

#endif // FFWEBCAM_H
