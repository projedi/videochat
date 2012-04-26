#ifndef FFWEBCAM_H
#define FFWEBCAM_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
    #include <libswscale/swscale.h>
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

class Webcam: public QObject {
    Q_OBJECT
public:
    Webcam(WebcamParams params);
    void start();
    void stop();
signals:
    void frameArrived(QImage frame);
private slots:
    void grabTimerTimeout();
private:
    QTimer* grabTimer;
    WebcamParams params;
    AVFormatContext* inputFormatContext;
    AVStream* inputStream;
    AVCodecContext* avctx;
    SwsContext* img_convert_ctx;
    AVPicture* pict;
};

#endif // FFWEBCAM_H
