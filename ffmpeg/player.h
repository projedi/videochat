#include "ffmpeg.h"

//TODO: It can't dynamically resized. Bad.
//TODO: What if I want different audio renderings on different platforms? For some reason.
class Player: public FFSink, public FFVideoDevice {
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
