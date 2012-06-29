#pragma once

#include "ffmpeg.h"

// ALSA + Video4Linux2 as one input device
class ALSAV4L2: public FFSource, public FFHardware {
   Q_OBJECT
public:
   ALSAV4L2(QString camera, QString microphone);
   ~ALSAV4L2();

   QList<QPair<QString,QString> > cameras();
   QList<QPair<QString,QString> > microphones();

   int width() { initFuture.waitForFinished(); return vCodec->width; }
   int height() { initFuture.waitForFinished(); return vCodec->height; }
   PixelFormat pixelFormat() { initFuture.waitForFinished(); return vCodec->pix_fmt; }
   int64_t channelLayout() { initFuture.waitForFinished(); return aCodec->channel_layout; }
   AVSampleFormat sampleFormat() { initFuture.waitForFinished(); return aCodec->sample_fmt; }
   int sampleRate() { initFuture.waitForFinished(); return aCodec->sample_rate; }
private:
   AVFormatContext* vFormat;
   AVFormatContext* aFormat;
   AVCodecContext* vCodec;
   AVCodecContext* aCodec;

   QFuture<void> initFuture;
   void init(QString camera, QString microphone);
   void videoWorker();
   void audioWorker();
};
