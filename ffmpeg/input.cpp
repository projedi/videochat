#include "ffmpeg.h"
#include <QtConcurrentRun>

#include <iostream>
using namespace std;

Input::Stream::Stream(AVStream* avstream) throw(int) {
   if(avstream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      type = Video;
   } else if(avstream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
      type = Audio;
   } else {
      type = Other;
   }

   codec = avstream->codec;
   pts = 0;
   if(type != Other) {
      AVCodec* decoder = avcodec_find_decoder(codec->codec_id);
      if(avcodec_open2(codec,decoder,0)<0) throw 1;
   }
}

Input::Stream::~Stream() {
   logger("Closing input stream");
   //subscribers.clear();
   if(codec) {
      logger("Closing codec on input stream");
      avcodec_close(codec);
      //cout << "Freeing codec on input stream" << endl;
      //av_free(codec);
   }
}

AVFrame* Input::Stream::decode(AVPacket* pkt) {
   if(type == Video) {
      int got_picture;
      AVFrame* frame = avcodec_alloc_frame();
      if(avcodec_decode_video2(codec,frame,&got_picture,pkt) < 0) return 0;
      if(!got_picture) { av_free(frame); return 0; }
      //frame->pts = av_frame_get_best_effort_timestamp(frame);
      frame->pts = pts++;
      return frame;
   } else if(type == Audio) {
      //TODO: Implement
      return 0;
   } else return 0;
}

void Input::Stream::broadcast(AVFrame* frame) {
   if(!frame) return;
   for(int i = 0; i < subscribers.count(); i++) {
      Output::Stream* subs = subscribers[i];
      try {
         //AVPacket* pkt = subs->encode(frame);
         //subs->sendToOwner(pkt);
         subs->process(frame);
      } catch(...) { logger("Error broadcasting to a stream"); continue; }
   }
   av_free(frame);
}

void Input::Stream::process(AVPacket* pkt) {
   AVFrame* frame = decode(pkt);
   broadcast(frame);
}

void Input::Stream::subscribe(Output::Stream* client) { subscribers.append(client); }

void Input::Stream::unsubscribe(Output::Stream* client) { subscribers.removeOne(client); }

QList<Output::Stream*> Input::Stream::getSubscribers() const { return subscribers; }

StreamInfo Input::Stream::info() {
   StreamInfo info;
   info.type = type;
   if(type == Video) {
      info.video.width = codec->width;
      info.video.height = codec->height;
      info.video.pixelFormat = codec->pix_fmt;
      info.video.fps = codec->time_base.den;
      info.bitrate = codec->bit_rate;
   } else if(type == Audio) {
      info.audio.channelLayout = codec->channel_layout; 
      info.audio.sampleFormat = codec->sample_fmt;
      info.audio.sampleRate = codec->sample_rate;
      info.bitrate = codec->bit_rate;
   }
   return info;
}

Input::~Input() {
   logger("Closing input");
   setState(Paused);
   workerFuture.waitForFinished();
   logger("Worker down");
   for(int i = 0; i < streams.count(); i++) {
      if(streams[i]) delete streams[i];
   }
}

QList<Input::Stream*> Input::getStreams() const { return streams; }

Input::State Input::getState() { return state; }

void Input::setState(Input::State state) {
   QMutexLocker l(&m);
   logger( fmt + ":Setting state from " + QString::number((int)this->state)
         + " to " + QString::number((int)state));
   if(state == this->state) return;
   this->state = state;
   if(state == Playing) workerFuture = QtConcurrent::run(this,&Input::worker);
}

int callback(void* arg) {
    Input::State state = ((InputGeneric*)arg)->getState();
    return state == Input::Paused;
}

//TODO: provide a way to automagically determine the format
InputGeneric::InputGeneric(QString fmt, QString file) throw(int) {
   QByteArray formatN = fmt.toAscii();
   QByteArray fileN = file.toAscii();
   format = avformat_alloc_context();
   format->interrupt_callback.callback = callback;
   format->interrupt_callback.opaque = this;
   if(avformat_open_input( &format, fileN.data(), av_find_input_format(formatN.data()), 0)<0)
      throw 1;
   //in win32 it must be commented for some reason
   //avformat_find_stream_info(format,0);
   for(int i = 0; i < format->nb_streams; i++) {
      try {
         streams.append(new Stream(format->streams[i]));
      } catch(...) { logger(fmt + ":Error creating stream"); }
   }
   state = Paused;
   this->fmt = fmt;
}

InputGeneric::~InputGeneric() {
   logger(fmt + ":Closing generic input");
   setState(Paused);
   closingMutex.lock();
   logger(fmt + ":Locking closingMutex in destructor");
   if (format->iformat && (format->iformat->read_close)) {
      logger(fmt + ":Found iformat on generic input");
      format->iformat->read_close(format);
   }
   avio_close(format->pb);
   closingMutex.unlock();
   logger(fmt + ":Unlocking closingMutex in destructor");
   //logger(fmt + ":Waiting for worker to go down");
   workerFuture.waitForFinished();
   logger(fmt + ":Worker down with generic input");

   // TODO: for the same reason as in OutputGeneric
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
   streams.clear();
   logger(fmt + ":Freeing context on input generic");
   avformat_free_context(format);

}

void InputGeneric::worker() {
   //TODO: If lots of errors, then what?
   while(state != Paused) {
      AVPacket* pkt = new AVPacket();
      av_init_packet(pkt);
      //cout << "On in: pts=" << pkt->pts << ";dts=" << pkt->dts << endl;
      closingMutex.lock();
      //TODO: determine when negative number is an EOF
      if(av_read_frame(format,pkt) < 0) { logger(fmt + ":Can't read frame"); continue; }
      closingMutex.unlock();
      if(state != Playing) {
         logger(fmt + ":Read(?) frame but the pause was signaled");
         av_free_packet(pkt);
         break;
      }
      streams[pkt->stream_index]->process(pkt);
      av_free_packet(pkt);
   }
}
