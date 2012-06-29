#include "ffmpeg.h"

// Get stream from file or some network stream.
class Client: public FFSource {
   Q_OBJECT
public:
   Client(QString file);
   ~Client();

   int width() { initFuture.waitForFinished(); return vCodec->width; }
   int height() { initFuture.waitForFinished(); return vCodec->height; }
   PixelFormat pixelFormat() { initFuture.waitForFinished(); return vCodec->pix_fmt; }
   int64_t channelLayout() { initFuture.waitForFinished(); return aCodec->channel_layout; }
   AVSampleFormat sampleFormat() { initFuture.waitForFinished(); return aCodec->sample_fmt; }
   int sampleRate() { initFuture.waitForFinished(); return aCodec->sample_rate; }
private:
   AVFormatContext* format;
   AVCodecContext* vCodec;
   AVCodecContext* aCodec;
   int vIndex;
   int aIndex;

   QFuture<void> initFuture;
   void init(QString file);
   void worker();
};
