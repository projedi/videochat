#include "ffmpeg.h"

//TODO: Client & Server? Seriously? WHY U NO USE PROPER NAMING?
class Client: public FFSource, public FFVideoDevice {
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
