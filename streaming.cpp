#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) {
   this->call = call;
   scaler = 0;
}

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

void RtpOutputStream::process(AVFrame* frame) {
   AVFrame* newFrame = avcodec_alloc_frame();
   scaler = sws_getCachedContext( scaler, frame->width, frame->height
                                , (PixelFormat)frame->format
                                , frame->width, frame->height, PIX_FMT_RGB24
                                , SWS_BICUBIC, 0, 0, 0);
   avpicture_alloc( (AVPicture*)newFrame, PIX_FMT_RGB24, frame->width, frame->height);
   sws_scale( scaler, frame->data, frame->linesize, 0, frame->height
            , newFrame->data, newFrame->linesize);
   
   int size = newFrame->linesize[0]*frame->height;
   QXmppVideoFrame qframe( size, QSize(frame->width,frame->height), newFrame->linesize[0]
                         , QXmppVideoFrame::Format_RGB24);
   uchar *data = (uchar*) qframe.bits();
   memcpy(data, newFrame->data[0], size);
   avpicture_free((AVPicture*)newFrame);
   av_free(newFrame);
   call->videoChannel()->writeFrame(qframe);
}
