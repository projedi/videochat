#pragma once

#include "ffmpeg.h"
#include <qxmpp/QXmppCallManager.h>

class RtpOutputStream: public OutputStream, public QObject {
   Q_OBJECT
public:
   RtpOutputStream(QXmppCall*);
   ~RtpOutputStream();
   StreamInfo info();
   void process(AVFrame*);
public slots:
   void callVideoModeChanged(QIODevice::OpenMode);
};
