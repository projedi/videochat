#pragma once

#include "ffmpeg.h"

class Server: public FFSink {
   Q_OBJECT 
public:
   Server(QString file);
   ~Server();
   void newVideoFrame(QAVFrame frame);
   void newAudioFrame(QAVFrame frame);

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
   AVStream* vStream;
   AVStream* aStream;
   QFuture<void> initFuture;
   QFuture<void> videoWorkerFuture;
   void videoWorker(QAVFrame frame);
   void init(QString file);
};
