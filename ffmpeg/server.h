#include "ffmpeg.h"

class Server: public FFSink, public FFVideoDevice {
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
