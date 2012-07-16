#include "ffmpeg.h"
#include <QtConcurrentRun>

InputStream::InputStream(AVStream* avstream) {
   switch(avstream->codec->codec_type) {
      case AVMEDIA_TYPE_VIDEO:
         type = Video;
         break;
      case AVMEDIA_TYPE_AUDIO:
         type = Audio;
         break;
      default:
         type = Other;
         break;
   }
   codecCtx = avstream->codec;
   pts = 0;
   AVCodec* codec = avcodec_find_decoder(codecCtx->codec_id);
   if(avcodec_open2(codecCtx,codec,0) < 0) {
      avcodec_close(codecCtx);
      codecCtx = 0;
      state = Closed;
   } else state = Opened;
}

InputStream::~InputStream() { if(codecCtx) avcodec_close(codecCtx); }

StreamInfo InputStream::info() {
   StreamInfo info;
   info.type = type;
   switch(type) {
      case Video:
         info.video.width = codecCtx->width;
         info.video.height = codecCtx->height;
         info.video.pixelFormat = codecCtx->pix_fmt;
         info.video.fps = codecCtx->time_base.den;
         info.bitrate = codecCtx->bit_rate;
         break;
      case Audio:
         info.audio.channelLayout = codecCtx->channel_layout; 
         info.audio.sampleFormat = codecCtx->sample_fmt;
         info.audio.sampleRate = codecCtx->sample_rate;
         info.bitrate = codecCtx->bit_rate;
         break;
   }
   return info;
}

InputStream::State InputStream::getState() { return state; }

void InputStream::subscribe(OutputStream* client) {
   QMutexLocker l(&subscribersLock);
   if(subscribers.indexOf(client) == -1) subscribers.append(client);
}

void InputStream::unsubscribe(OutputStream* client) {
   QMutexLocker l(&subscribersLock);
   subscribers.removeOne(client);
}

void InputStream::process(AVPacket* pkt) {
   AVFrame* frame = 0;
   switch(type) {
      case Video:
         int got_picture;
         frame = avcodec_alloc_frame();
         if(avcodec_decode_video2(codecCtx,frame,&got_picture,pkt) < 0){frame = 0;break;}
         if(!got_picture) { av_free(frame); frame = 0; break; }
         //frame->pts = av_frame_get_best_effort_timestamp(frame);
         frame->pts = pts++;
         break;
      //TODO: Implement
      case Audio:
         break;
      case Other:
         break;
   }
   if(!frame) return;
   subscribersLock.lock();
   for(int i = 0; i < subscribers.count(); i++) subscribers[i]->process(frame);
   subscribersLock.unlock();
   av_free(frame);
}

Input::~Input() {
   setState(Paused);
   workerFuture.waitForFinished();
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
}

QList<InputStream*> Input::getStreams() { return streams; }

Input::State Input::getState() { return state; }

void Input::setState(Input::State state) {
   QMutexLocker l(&stateLocker);
   if(state == this->state) return;
   this->state = state;
   if(state == Playing) workerFuture = QtConcurrent::run(this,&Input::worker);
}

int callback(void* arg) {
   Input::State state = ((InputGeneric*)arg)->getState();
   return state == Input::Paused || state == Input::Closed;
}

InputGeneric::InputGeneric(QString filename, QString formatname) {
   //initFuture = QtConcurrent::run(this, &InputGeneric::init, filename, formatname);
   state = Opening;
   formatCtx = avformat_alloc_context();
   formatCtx->interrupt_callback.callback = callback;
   formatCtx->interrupt_callback.opaque = this;
   AVInputFormat* avif = av_find_input_format(formatname.toAscii().data());
   if(avformat_open_input(&formatCtx,filename.toAscii().data(),avif,0) < 0) state = Closed;
   else {
      for(int i = 0; i < formatCtx->nb_streams; i++) {
         InputStream* stream = new InputStream(formatCtx->streams[i]);
         if(stream->getState() == InputStream::Closed) delete stream;
         else streams.append(stream);
      }
      state = Paused;
   }
}

InputGeneric::~InputGeneric() {
   setState(Paused);
   closingLocker.lock();
   if(formatCtx->iformat && (formatCtx->iformat->read_close))
      formatCtx->iformat->read_close(formatCtx);
   avio_close(formatCtx->pb);
   closingLocker.unlock();
   //It might seem like duplication from parent destructor but it's required because
   //avformat_free_context frees codecs itself(but doesnt't close them).
   workerFuture.waitForFinished();
   //TODO: Watch the memory when I remove only this loop.
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
   streams.clear();
   avformat_free_context(formatCtx);
}

void InputGeneric::worker() {
   //TODO: If lots of errors, then what?
   while(state != Paused) {
      AVPacket* pkt = new AVPacket();
      av_init_packet(pkt);
      closingLocker.lock();
      //TODO: determine when negative number is an EOF
      if(av_read_frame(formatCtx,pkt) < 0) { logger("Can't read frame"); continue; }
      closingLocker.unlock();
      streams[pkt->stream_index]->process(pkt);
      av_free_packet(pkt);
   }
}
