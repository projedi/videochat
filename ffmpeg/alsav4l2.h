#include "ffmpeg.h"

// ALSA + Video4Linux2 as one input device
class ALSAV4L2: public FFSource, public FFHardware {
   Q_OBJECT
public:
   ALSAV4L2(QString camera, QString microphone);
   ~ALSAV4L2();

   QList<QPair<QString,QString>> cameras();
   QList<QPair<QString,QString>> microphones();

   int width() { QMutexLocker(&initLocker); return vCodec->width; }
   int height() { QMutexLocker(&initLocker); return vCodec->height; }
   PixelFormat pixelFormat() { QMutexLocker(&initLocker); return vCodec->pix_fmt; }
   int64_t channelLayout() { QMutexLocker(&initLocker); return aCodec->channel_layout; }
   AVSampleFormat sampleFormat() { QMutexLocker(&initLocker); return aCodec->sample_fmt; }
   int sampleRate() { QMutexLocker(&initLocker); return aCodec->sample_rate; }
private:
   AVFormatContext* vFormat;
   AVFormatContext* aFormat;
   AVCodecContext* vCodec;
   AVCodecContext* aCodec;

   QMutex initLocker;
   void init(QString camera, QString microphone);
   void videoWorker();
   void audioWorker();
};