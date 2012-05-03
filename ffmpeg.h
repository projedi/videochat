#pragma once

extern "C" {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libavdevice/avdevice.h>
   #include <libswscale/swscale.h>
   #include <libavutil/opt.h>
}

#include <iostream>

using namespace std;

class FFInput : public QObject {
   Q_OBJECT
public:
   FFInput(char* name, char* uri, char* fmt = 0);
   ~FFInput();
signals:
   void newFrame(AVFrame* frame);
private:
   void streamReader();
};

class FFOutput : public QObject {
   Q_OBJECT
public:
   FFOutput(char* file, char* fmt = 0, char* vcodec = 0, char* acodec = 0);
   ~FFOutput();
public slots:
   void addFrame(AVFrame* frame);
};

class FFConvert : public QObject {
   Q_OBJECT
public:
   FFConvert(FFInput* inp, FFOutput* out);
   ~FFConvert();
signals:
   void newFrame(AVFrame* frame);
public slots:
   void addFrame(AVFrame* frame);
private:
   SwsContext* convert;
};

class FFmpeg : public QObject {
   Q_OBJECT
public:
   FFmpeg();
   ~FFmpeg();
   FFInput* addInput(char* uri, char* fmt = 0);
   FFOutput* addOutput(char* uri, char* fmt = 0, char* codec = 0);
   connectIO(FFinput* inp, FFoutput* out);
signals:
   void cameraFrame(uint8_t* data, int width, int height);
   void remoteFrame(uint8_t* data, int width, int height);
private:
   list<FFInput*> inputs;
   list<FFOutput*> outputs;
   list<FFConvert*> contexts;
};
