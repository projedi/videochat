#pragma once

#include "ffmpeg.h"
#include <qxmpp/QXmppCallManager.h>

class RtpOutputStream: public QObject, public OutputStream {
   Q_OBJECT
public:
   RtpOutputStream(QXmppCall*);
   ~RtpOutputStream();
   StreamInfo info();
   void process(AVFrame*);
private:
   QXmppCall* call;
};
