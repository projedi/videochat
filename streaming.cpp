#include "streaming.h"
#include <qxmpp/QXmppRtpChannel.h>

RtpOutputStream::RtpOutputStream(QXmppCall* call) {
   this->call = call;
   scaler = 0;
}

RtpOutputStream::~RtpOutputStream() { }

StreamInfo RtpOutputStream::info() { }

void RtpOutputStream::process(AVFrame* frame) {
   call->videoChannel()->writeFrame(frame);
}
