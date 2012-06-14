#include "ffmpeg.h"

//TODO: It can't dynamically resized. Bad.
//TODO: What if I want different audio renderings on different platforms? For some reason.
//TODO: There's no native ffmpeg method to output audio on windows. Therefore,
//TODO: this solution is just to display local video
//TODO: All that in mind, this class is probably useless
class Player: public FFSink {
   Q_OBJECT
public:
   Player(int width, int height, QString audioOutput);
   ~Player();
   void newVideoFrame(AVFrame* frame);
   void newAudioFrame(AVFrame* frame);
   int getWidth() { QMutexLocker(&initLocker); return width; }
   int getHeight() { QMutexLocker(&initLocker); return height; }
   PixelFormat getPixelFormat() { QMutexLocker(&initLocker); return PIX_FMT_RGB32; }
signals:
   void onNewFrame(QPixmap frame);
private:
   //AVFormatContext* audioFormat;
   //AVCodecContext* audioCodec;
   QMutex initLocker;
   void init(int width, int height, QString audioOutput);
   int width;
   int height;
};
