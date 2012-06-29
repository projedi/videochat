#include "ffmpeg.h"

void FFSink::setSource(FFSource* src) {
   clearSource();
   source = src;
   scaler = sws_getCachedContext( scaler, src->width(), src->height(), src->pixelFormat()
                                , width(), height(), pixelFormat(), SWS_BICUBIC, 0, 0, 0);
   resampler = swr_alloc_set_opts( resampler, channelLayout(), sampleFormat(), sampleRate()
                                 , src->channelLayout(), src->sampleFormat()
                                 , src->sampleRate(), 0, 0);
   connect(source, SIGNAL(onNewVideoFrame(QAVFrame)), SLOT(newVideoFrame(QAVFrame)));
   connect(source, SIGNAL(onNewAudioFrame(QAVFrame)), SLOT(newAudioFrame(QAVFrame)));
}

void FFSink::clearSource() {
   if(!source) return;
   disconnect();
   source = 0;
   swr_free(&resampler);
   sws_freeContext(scaler);
   scaler = 0;
}

//TODO: do nothing if there's no conversion going
QAVFrame rescale(QAVFrame frame) {
   AVFrame* oldFrame = frame.data();
   AVFrame* newFrame = avcodec_alloc_frame();
   //TODO: Seems like this allocate doesn't go well with av_free
   avpicture_alloc((AVPicture*)newFrame,pixelFormat(),width(),height());
   sws_scale(scaler,oldFrame->data,oldFrame->linesize,0,oldFrame->height
            ,newFrame->data,newFrame->linesize);
   newFrame->width = oldFrame->width;
   newFrame->height = oldFrame->height;
   newFrame->pts = oldFrame->pts;
   return QAVFrame(newFrame);
}

QAVFrame resample(QAVFrame frame) {
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
   return frame;
}
