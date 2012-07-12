#include "ffmpeg.h"
#include <QtConcurrentRun>

#include <iostream>
using namespace std;

Input::Stream::Stream(MediaType type, AVCodecContext* codec, Input* owner) {
   this->type = type;
   this->codec = codec;
   this->owner = owner;
   pts = 0;
   if(type != Other) {
      AVCodec* decoder = avcodec_find_decoder(codec->codec_id);
      avcodec_open2(codec,decoder,0);
   }
}

Input::Stream::~Stream() {
   cout << "Closing input stream" << endl;
   //subscribers.clear();
   if(codec) {
      cout << "Closing codec on input stream" << endl;
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
      } catch(...) { cout << "Error broadcasting to a stream" << endl; continue; }
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

Input* Input::Stream::getOwner() { return owner; }

Input::~Input() {
   cout << "Closing input" << endl;
   setState(Paused);
   workerFuture.waitForFinished();
   cout << "Worker down" << endl;
   for(int i = 0; i < streams.count(); i++) {
      if(streams[i]) delete streams[i];
   }
}

QList<Input::Stream*> Input::getStreams() const { return streams; }

Input::State Input::getState() { return state; }

void Input::setState(Input::State state) {
   QMutexLocker l(&m);
   if(state == this->state) return;
   this->state = state;
   if(state == Playing) workerFuture = QtConcurrent::run(this,&Input::worker);
}

int callback(void* arg) {
    Input::State state = ((InputGeneric*)arg)->getState();
    //cout << "Callbacked with state=" << (int)state << endl;
    return state == Input::Paused;
}

//TODO: provide a way to automagically determine the format
InputGeneric::InputGeneric(QString fmt, QString file) {
   QByteArray formatName = fmt.toAscii();
   QByteArray fileName = file.toAscii();
   format = avformat_alloc_context();
   format->interrupt_callback.callback = callback;
   format->interrupt_callback.opaque = this;
   avformat_open_input(&format,fileName.data(),av_find_input_format(formatName.data()),0);
   //in win32 it must be commented for some reason
   //avformat_find_stream_info(format,0);
   for(int i = 0; i < format->nb_streams; i++) {
      AVStream* avstream = format->streams[i];
      Stream* stream;
      if(avstream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
         stream = new Stream(Video,avstream->codec,this);
         streams.append(stream);
      } else if(avstream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
         stream = new Stream(Audio,avstream->codec,this);
         streams.append(stream);
      } else {
         stream = new Stream(Other,0,this);
         streams.append(stream);
      }
   }
   state = Paused;
}

InputGeneric::~InputGeneric() {
   //avformat_close_input(&format);
    cout << "Closing generic input" << endl;
   setState(Paused);
   if (format->iformat && (format->iformat->read_close)) {
       cout << "Found iformat on generic input" << endl;
      format->iformat->read_close(format);
   }
   avio_close(format->pb);
   cout << "Waiting for worker to go down" << endl;
   workerFuture.waitForFinished();
   cout << "Worker down with generic input" << endl;

   // TODO: for the same reason as in OutputGeneric
   // TODO: also check that delete nullifies pointer
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
   streams.clear();
   cout << "Freeing context on input generic" << endl;
   avformat_free_context(format);
   //cout << "Waiting for the worker cycle to end" << endl;;
   //QMutexLocker l(&m);
   //state = Paused;

}

void InputGeneric::worker() {
   //TODO: If lots of errors, then what?
   while(state != Paused) {
       //cout << "Worker working" << endl;
      //m.lock();
      //if(state != Playing) return;
      AVPacket* pkt = new AVPacket();
      av_init_packet(pkt);
      //TODO: determine when negative number is an EOF

      //if(state != Playing) return;
      //cout << "On in: pts=" << pkt->pts << ";dts=" << pkt->dts << endl;
      try {
         if(av_read_frame(format,pkt) < 0) { cout << "Can't read frame" << endl; continue; }
         if(state != Playing) return;
         //Stream* stream = streams[pkt->stream_index];
         //if(state != Playing) return;
         //AVFrame* frame = stream->decode(pkt);
         //if(state != Playing) return;
         //stream->broadcast(frame);
         streams[pkt->stream_index]->process(pkt);
      } catch(...) { cout << "Error using stream in worker" << endl; }
      av_free_packet(pkt);
      //m.unlock();
      //TODO: Add waiting
   }
}
