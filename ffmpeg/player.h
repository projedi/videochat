#pragma once

#include "ffmpeg.h"
#include <QImage>
#include <QPixmap>

//TODO: It can't dynamically resized. Bad.
class Player: public FFSink {
   Q_OBJECT
public:
   Player(int width, int height);
   ~Player();

   void newVideoFrame(QAVFrame frame);
   void newAudioFrame(QAVFrame frame);

   int width() { initFuture.waitForFinished(); return w; }
   int height() { initFuture.waitForFinished(); return h; }
   PixelFormat pixelFormat() { initFuture.waitForFinished(); return PIX_FMT_RGB32; }
   int64_t channelLayout() { initFuture.waitForFinished(); return av_get_default_channel_layout(2); }//h(spec.channels); }
   AVSampleFormat sampleFormat() { initFuture.waitForFinished(); return AV_SAMPLE_FMT_S16; }
   int sampleRate() { initFuture.waitForFinished(); return 44100; }//spec.freq; }
   AVCodecContext* audioCodec() { initFuture.waitForFinished(); return 0; }
signals:
   void onNewFrame(QPixmap frame);
private:
   //AVFormatContext* audioFormat;
   //AVCodecContext* audioCodec;
   QFuture<void> initFuture;
   void init(int width, int height);
   int w;
   int h;
   //SDL_AudioSpec spec;
};
