#include "ffmpeg.h"

// ALSA + Video4Linux2 as one input device
class ALSAV4L2: public FFSource {
   Q_OBJECT
public:
   ALSAV4L2(QString cameraName, QString microphoneName);
   ALSAV4L2(Device camera, Device microphone): this(camera.ffmpegName
                                                   ,microphone.ffmpegName);
   ~ALSAV4L2();
   //TODO: I want these to be inheritable
   static QList<Device> getCameraDevices();
   static QList<Device> getMicrophoneDevices();

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
   AVFormatContext* videoFormat;
   AVFormatContext* audioFormat;
   AVCodecContext* videoCodec;
   AVCodecContext* audioCodec;

   QMutex initLocker;
   void init(QString cameraName, QString microphoneName);
   void videoWorker();
   void audioWorker();
};
