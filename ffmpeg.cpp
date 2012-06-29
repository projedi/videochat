#include "ffmpeg.h"

void FFConnector::ffConnect(FFSource* source, FFSink* sink) {
   scaler = sws_getCachedContext( 0
                                , source->width(), source->height(), source->pixelFormat()
                                , sink->width(), sink->height(), sink->pixelFormat()
                                , SWS_BICUBIC, 0, 0, 0);
   /*
   resampler = swr_alloc_set_opts( 0, sink->channelLayout(), sink->sampleFormat()
                                 , sink->sampleRate(), source->channelLayout()
                                 , source->sampleFormat(), source->sampleRate()
                                 , 0, 0);
                                 */
   connect(source, SIGNAL(onNewVideoFrame(QAVFrame)), SLOT(newVideoFrame(QAVFrame)));
   //connect(source, SIGNAL(onNewAudioFrame(QAVFrame)), SLOT(newAudioFrame(QAVFrame)));
   this->source = source;
   this->sink = sink;
   w = sink->width();
   h = sink->height();
   pf = sink->pixelFormat();
}

void FFConnector::ffDisconnect() {
   disconnect();
   swr_free(&resampler);
   sws_freeContext(scaler); scaler = 0;
}

void FFConnector::newVideoFrame(QAVFrame frame) {
   AVFrame* newFrame = avcodec_alloc_frame();
   avpicture_alloc((AVPicture*)newFrame,pf,w,h);
   sws_scale(scaler,frame.data()->data,frame.data()->linesize,0,frame.data()->height
            ,newFrame->data,newFrame->linesize);
   //TODO: Is there a way to copy all metas to new frame automagically?
   newFrame->width = w;
   newFrame->height = h;
   //TODO: better time scaling
   newFrame->pts = frame.data()->pts;
   sink->newVideoFrame(QAVFrame(newFrame,true));
}

void FFConnector::newAudioFrame(QAVFrame frame) {
   /*
   AVFrame* newFrame = avcodec_alloc_frame();
   newFrame->nb_samples = 100;
   int ch = 2;
   int buf_size = av_samples_get_buffer_size( 0, ch
                                            , newFrame->nb_samples, sink->sampleFormat()
                                            , 0);
   const uint8_t* buf = new uint8_t[buf_size];
   const uint8_t *in[] = { frame.data()->data[0] };
   avcodec_fill_audio_frame(newFrame, 2, sink->sampleFormat(), buf, buf_size, 0);
   swr_convert( resampler, newFrame->data, newFrame->nb_samples
              , in, frame.data()->nb_samples);
   sink->newAudioFrame(QAVFrame(newFrame));
   */
}
