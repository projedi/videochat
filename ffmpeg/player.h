#pragma once

#include "ffmpeg.h"

class Player: public QWidget, public FFSink {
   Q_OBJECT
public:
   Player(QWidget *parent = 0): QWidget(parent) { }
   ~Player() { }

   void newVideoFrame(QAVFrame frame);
   void newAudioFrame(QAVFrame frame);

   int width() { initFuture.waitForFinished(); return w; }
   int height() { initFuture.waitForFinished(); return h; }
   PixelFormat pixelFormat() { initFuture.waitForFinished(); return PIX_FMT_RGB32; }
   int64_t channelLayout() { initFuture.waitForFinished();
      return av_get_default_channel_layout(2); }//h(spec.channels); }
   AVSampleFormat sampleFormat() { initFuture.waitForFinished(); return AV_SAMPLE_FMT_S16; }
   int sampleRate() { initFuture.waitForFinished(); return 44100; }//spec.freq; }
   AVCodecContext* audioCodec() { initFuture.waitForFinished(); return 0; }
protected:
   void paintEvent(QPaintEvent *event);
private:
   QImage image;
};
