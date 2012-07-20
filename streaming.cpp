#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) { this->call = call; }

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

//TODO: somewhere there might be a need for a pixel format conversion
void RtpOutputStream::process(AVFrame* frame) {
   QXmppVideoFrame qframe( frame->linesize[0]*frame->height
                         , QSize(frame->width,frame->height), frame->linesize[0]
                         , QXmppVideoFrame::Format_YUV420P);
   uchar *data = qframe.bits();
   memcpy(data, frame->data[0], frame->linesize[0]*frame->height);
   call->videoChannel()->writeFrame(qframe);
}
