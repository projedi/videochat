#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) {
   this->call = call;
   scaler = 0;
}

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

void RtpOutputStream::process(AVFrame* frame) {
   //qDebug("AV_NUM_DATA_POINTERS=%d",AV_NUM_DATA_POINTERS);
   //qDebug( "Rtp Output stream received frame with linesizes: %d, %d, %d, %d, %d, %d, %d, %d"
   //      , frame->linesize[0], frame->linesize[1], frame->linesize[2], frame->linesize[3]
   //      , frame->linesize[4], frame->linesize[5], frame->linesize[6], frame->linesize[7]);
   //qDebug("It's format is %d, size %dx%d",frame->format,frame->width,frame->height);
   AVFrame* newFrame = avcodec_alloc_frame();
   scaler = sws_getCachedContext( scaler, frame->width, frame->height
                                , (PixelFormat)frame->format
                                , frame->width, frame->height, PIX_FMT_RGB24
                                , SWS_BICUBIC, 0, 0, 0);
   avpicture_alloc( (AVPicture*)newFrame, PIX_FMT_RGB24, frame->width, frame->height);
   sws_scale( scaler, frame->data, frame->linesize, 0, frame->height
            , newFrame->data, newFrame->linesize);
   
   //qDebug( "Scaling produced frame with datas: %d, %d, %d, %d"
   //      , newFrame->data[0], newFrame->data[1], newFrame->data[2], newFrame->data[3]);
   //qDebug("It's format is %d, size %dx%d",(int)PIX_FMT_YUV420P,frame->width,frame->height);
   int size = newFrame->linesize[0]*frame->height;
   QXmppVideoFrame qframe( size, QSize(frame->width,frame->height), newFrame->linesize[0]
                         , QXmppVideoFrame::Format_RGB24);
   uchar *data = (uchar*) qframe.bits();
   memcpy(data, newFrame->data[0], size);
   av_free(newFrame);
   call->videoChannel()->writeFrame(qframe);
}
