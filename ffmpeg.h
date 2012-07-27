#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libswresample/swresample.h>
   //#include <libavutil/opt.h>
}

#include <QMutexLocker>
#include <QFuture>

enum MediaType { Video, Audio, Other };

struct StreamInfo {
   MediaType type;
   int bitrate;
   union {
      struct {
         int width;
         int height;
         PixelFormat pixelFormat;
         int fps;
      } video;
      struct {
         int64_t channelLayout;
         AVSampleFormat sampleFormat;
         int sampleRate;
      } audio;
   };
};
class OutputStream;
class Output;
class InputStream;
class Input;

#include "ffmpeg/output.h"
#include "ffmpeg/input.h"
#include "ffmpeg/hardware.h"
