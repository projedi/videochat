#pragma once

#include "ffmpeg.h"

class InputStream {
public:
   enum State { Opened, Closed };
   InputStream(AVStream*);
   ~InputStream();
   StreamInfo info();
   State getState();
   void subscribe(OutputStream*);
   void unsubscribe(OutputStream*);
   void process(AVPacket*);
private:
   MediaType type;
   AVCodecContext* codecCtx;
   QMutex subscribersLock;
   QList<OutputStream*> subscribers;
   Input* owner;
   int64_t pts;
   State state;
};

class Input {
public:
   enum State { Playing, Paused, Closed, Opening };
   virtual ~Input();
   QList<InputStream*> getStreams();
   State getState();
   void setState(State state);
protected:
   QMutex stateLocker;
   State state;
   QList<InputStream*> streams;
   QFuture<void> workerFuture;
   virtual void worker() = 0;
};

class InputGeneric: public Input {
public:
   // Sometimes can autodetect format
   InputGeneric(QString filename, QString formatname = "");
   ~InputGeneric();
private:
   QMutex closingLocker;
   AVFormatContext* formatCtx;
   void worker();
};
