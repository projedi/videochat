#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) {
   this->call = call;
}

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

void RtpOutputStream::process(AVFrame* frame) {
   call->videoChannel()->writeFrame(frame);
}
