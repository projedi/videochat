#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) {
   this->call = call;
   scaler = 0;
}

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

//TODO: somewhere there might be a need for a pixel format conversion
void RtpOutputStream::process(AVFrame* frame) {
   QXmppVideoFrame qframe( frame->linesize[0]*frame->height
                         , QSize(frame->width,frame->height), frame->linesize[0]
                         , QXmppVideoFrame::Format_YUV420P);
   AVFrame* newFrame = avcodec_alloc_frame();
   scaler = sws_getCachedContext( scaler, frame->width, frame->height
                                , (PixelFormat)frame->format
                                , frame->width, frame->height, PIX_FMT_YUV420P
                                , SWS_BICUBIC, 0, 0, 0);
   avpicture_alloc( (AVPicture*)newFrame, PIX_FMT_YUV420P, frame->width, frame->height);
   sws_scale( scaler, frame->data, frame->linesize, 0, frame->height
            , newFrame->data, newFrame->linesize);
   uchar *data = qframe.bits();
   memcpy(data, newFrame->data[0], newFrame->linesize[0]*frame->height);
   av_free(newFrame);
   call->videoChannel()->writeFrame(qframe);
}
