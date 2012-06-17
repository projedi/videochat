#include "ffmpeg.h"

void FFConnector::ffConnect(FFSource* source, FFSink* sink) {
   scaler = sws_getCachedContext( 0
                                , source->width(), source->height(), source->pixelFormat()
                                , sink->width(), sink->height(), sink->pixelFormat()
                                , SWS_BICUBIC, 0, 0, 0);
   resampler = swr_alloc_set_opts( 0, sink->channelLayout(), sink->sampleFormat()
                                 , sink->sampleRate(), source->getChannelLayout()
                                 , source->getSampleFormat(), source->getSampleRate()
                                 , 0, 0);
   //TODO: Should I add Qt::QueuedConnection? ANALYZE!
   connect(source, SIGNAL(onNewVideoFrame(QAVFrame)), SLOT(newVideoFrame(QAVFrame)));
   connect(source, SIGNAL(onNewAudioFrame(QAVFrame)), SLOT(newAudioFrame(QAVFrame)));
   w = sink->getWidth();
   h = sink->getHeight();
   pf = sink->getPixelFormat();
}

void FFConnector::ffDisconnect() {
   disconnect();
   swr_free(&resampler);
   sws_freeContext(scaler); scaler = 0;
}

void FFConnector::newVideoFrame(QAVFrame frame) {
   AVFrame* newFrame = avcodec_alloc_frame();
   avpicture_alloc((AVPicture*)newFrame.data,pf,w,h)
   sws_scale(scaler,frame.data->data,frame.data->linesize,0,frame.data->height
            ,newFrame->data,newFrame->linesize);
   //TODO: Is there a way to copy all metas to new frame automagically?
   newFrame->width = w;
   newFrame->height = h;
   //TODO: better time scaling
   newFrame->pts = frame.data->pts;
   sink->newVideoFrame(QAVFrame(newFrame));
}

void FFConnector::newAudioFrame(QAVFrame frame) {
   //TODO: Check if I really need a resampler
   sink->newAudioFrame(frame);
}
