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
   connect(source, SIGNAL(onNewVideoFrame(QAVFrame)), SLOT(newVideoFrame(QAVFrame)));
   connect(source, SIGNAL(onNewAudioFrame(QAVFrame)), SLOT(newAudioFrame(QAVFrame)));
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

void FFConnector::newVideoFrame(QAVFrame frame) {
   QAVFrame newFrame();
   avpicture_alloc((AVPicture*)newFrame.data,pf,w,h)
   sws_scale(scaler,frame.data->data,frame.data->linesize,0,frame.data->height
            ,newFrame.data->data,newFrame.data->linesize);
   //TODO: Is there a way to copy all metas to new frame automagically?
   newFrame.data->width = w;
   newFrame.data->height = h;
   //TODO: better time scaling
   newFrame.data->pts = frame.data->pts;
   sink->newVideoFrame(newFrame);
}

void FFConnector::newAudioFrame(QAVFrame frame) {
   //TODO: Check if I really need a resampler
   sink->newAudioFrame(frame);
   //TODO: Wait a second, champ. What if a frame is disposed?
}
