#include "ffmpeg.h"

FFmpeg::FFmpeg() {
   inputs = new list<FFInput*>();
   outputs = new list<FFOutput*>();
   converts = new list<FFConvert*>();
}

FFmpeg::~FFmpeg() {
   inputs.clear();
   outputs.clear();
   converts.clear();
   delete inputs;
   delete outputs;
   delete converts;
}

FFInput* FFmpeg::addInput(char* uri, char* fmt = 0) {
   FFInput* inp = new FFInput(uri,fmt);
   inputs.add(inp);
   return inp;
}

FFOutput* FFmpeg::addOutput(char* uri, char* fmt = 0, char* codec = 0) {
   FFOutput* out = new FFOutput(uri,fmt,coder);
   outputs.add(out);
   return out;
}

void FFmpeg::connectIO(FFInput* inp, FFOutput* out) {
   FFConvert* conv = new FFConvert(inp,out);
   converts.add(conv);
}

FFConvert::FFConvert(FFInput* inp, FFOutput* out) {
   convert = sws_getContext(inp->width,inp->height,inp->pix_fmt
                           ,out->width,out->height,out->pix_fmt
                           ,SWS_BICUBIC,0,0,0);
   connect(inp,SIGNAL(FFInput::newFrame(AVFrame*)),this,SLOT(addFrame(AVFrame*)));
   connect(this,SIGNAL(newFrame(AVFrame*)),out,SLOT(FFOutput::addFrame(AVFrame*)));
}

FFConvert::~FFConvert() {
   av_free(convert);
}

void addFrame(AVFrame* frame) {
   AVFrame* newframe = new AVFrame;
   int bufsize = avpicture_get_size(out->pix_fmt,out->width,out->height);
   uint8_t* buffer = new uint8_t[bufsize];
   avpicture_fill((AVPicture*)newframe,buffer,out->pix_fmt,out->width,out->height);
   sws_scale(convert,frame->data,frame->linesize,0,frame->height
            ,newframe->data,newframe->linesize);
   //TODO spooky
   av_free(frame);
   newframe->pts = frame->pts;
   emit newFrame(newframe);
}
