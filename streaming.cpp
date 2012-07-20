#include "streaming.h"

RtpOutputStream::RtpOutputStream(QXmppCall* call) { }

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

void RtpOutputStream::process(AVFrame* frame) {
   QXmppVideoFrame qframe;
   //TODO: convert frame to qframe
   call->videoChannel()->writeFrame(qframe);
}

void callVideoModeChanged(QIODevice::OpenMode mode) { }
