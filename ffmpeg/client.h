#include "ffmpeg.h"

//TODO: Client & Server? Seriously? WHY U NO USE PROPER NAMING?
class Client: public FFSource {
   Q_OBJECT
public:
   Client(QString file);
   ~Client();
   int getWidth() { QMutexLocker(&initLocker); return videoCodec->width; }
   int getHeight() { QMutexLocker(&initLocker); return videoCodec->height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return videoCodec->pix_fmt; }
   int64_t getChannelLayout() { QMutexLocker(&initLocker);
                                return audioCodec->channel_layout;
                              }
   AVSampleFormat getSampleFormat() { QMutexLocker(&initLocker);
                                      return audioCodec->sample_fmt;
                                    }
   int getSampleRate() { QMutexLocker(&initLocker); return audioCodec->sample_rate; }
private:
   AVFormatContext* format;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;
   int videoStreamIndex;
   int audioStreamIndex;

   QMutex initLocker;
   void init(QString file);
   void worker();
};
