#include "ffmpeg.h"

FFConnector::FFConnector(FFSource* source, FFSink* sink) {
   scaler = sws_getCachedContext( 0, source->getWidth(), source->getHeight()
                                , source->getPixelFormat(), sink->getWidth()
                                , sink->getHeight(), sink->getPixelFormat()
                                , SWS_BICUBIC,0,0,0);
   resampler = swr_alloc_set_opts( 0, sink->getChannelLayout(), sink->getSampleFormat()
                                 , sink->getSampleRate(), source->getChannelLayout()
                                 , source->getSampleFormat(), source->getSampleRate()
                                 ,0,0);
   //TODO: Should I add Qt::QueuedConnection? ANALYZE!
   connect(source, SIGNAL(onNewVideoFrame(AVFrame*)), SLOT(newVideoFrame(AVFrame*)));
   connect(source, SIGNAL(onNewAudioFrame(AVFrame*)), SLOT(newAudioFrame(AVFrame*)));
   w = sink->getWidth();
   h = sink->getHeight();
   pf = sink->getPixelFormat();
}

FFConnector::~FFConnector() {
   //TODO: Do I need to do that? Doesn't it disconnect all automagically?
   disconnect();
   swr_free(&resampler);
   sws_freeContext(scaler); scaler = 0;
}

void FFConnector::newVideoFrame(AVFrame* frame) {
   AVFrame* newFrame = avcodec_alloc_frame();
   avpicture_alloc((AVPicture*)newFrame,pf,w,h)
   sws_scale(scaler,frame->data,frame->linesize,0,frame->height
            ,newFrame->data,newFrame->linesize);
   //TODO: Is there a way to copy all metas to new frame automagically?
   newFrame->width = w;
   newFrame->height = h;
   //TODO: better time scaling
   newFrame->pts = frame->pts;
   sink->newVideoFrame(newFrame);
   //TODO: When to dispose the old frame?
}

void FFConnector::newAudioFrame(AVFrame* frame) {
   //TODO: Check if I really need a resampler
   sink->newAudioFrame(frame);
   //TODO: Wait a second, champ. What if frame is disposed?
}
