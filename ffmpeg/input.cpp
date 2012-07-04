
Input::Stream::Stream(MediaType type, AVCodecContext* codec) {
   this->type = type;
   this->codec = codec;
}

Input::Stream::~Stream() { avcodec_close(codec); av_free(codec); }

AVFrame* Input::Stream::decode(AVPacket* pkt) {
   AVFrame* frame = avcodec_alloc_frame();
   if(type == Video) {
      if(avcodec_decode_video2(codec,frame,&got_picture,&pkt) < 0) return;
      av_free_packet(pkt);
      if(!got_picture) { av_free(frame); return; }
      frame->pts = best_effort_timestamp(frame);
   } else if(type == Audio) {
      //TODO: Implement
   }
   return frame;
}

void Input::Stream::broadcast(AVFrame* frame) {
   QList<Output::Stream*>::iterator i;
   for(i=subscribers.begin();i!=subscribers.end();i++)
      (*i).sendToOwner((*i).encode(frame));
}

void Input::Stream::subscribe(Output::Stream* client) { subscribers.append(client); }

void Input::Stream::unsubscribe(Output::Stream* client) { subscribers.removeOne(client); }

StreamInfo Input::Stream::info() {
   StreamInfo info;
   info.type = type;
   info.bitrate = codec->bit_rate;
   if(type == Video) {
      info.video.width = codec->width;
      info.video.height = codec->height;
      info.video.pixelFormat = codec->pix_fmt;
      info.video.fps = codec->time_base.den;
   } else if(type == Audio) {
      info.audio.channelLayout = codec->channel_layout; 
      info.audio.sampleFormat = codec->sample_fmt;
      info.audio.sampleRate = codec->sample_rate;
   }
   return info;
}

Input::~Input() {
   setState(Paused);
   workerFuture.waitForFinished();
}

QList<Stream*> Input::getStreams() const { return streams; }

State Input::getState() { return state; }

void Input::setState(State state) {
   if(state == this->state) return;
   this->state = state;
   if(state == Playing) workerFuture = QtConcurrent::Run(this,&Input::worker);
}

InputGeneric::InputGeneric(QString fmt, QString file) {
   QByteArray formatName = fmt.toAscii();
   QByteArray fileName = file.toAscii();
   format = avformat_alloc_context();
   avformat_open_input(&format,fileName.data(),av_find_input_format(formatName.data()),0);
   avformat_find_stream_info(format,0);
   for(int i = 0; i < format->nb_streams; i++) {
      AVStream* avstream = format->streams[i];
      Stream* stream;
      if(avstream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
         stream = new Stream(Video,avstream->codec);
         streams.append(stream); 
      } else if(avstream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
         stream = new Stream(Audio,avstream->codec);
         streams.append(stream); 
      }
   }
   state = Paused;
}

~InputGeneric::InputGeneric() { avformat_close_input(&format); }

void InputGeneric::worker() {
   while(state != Paused) {
      AVPacket* pkt;
      av_read_frame(format,pkt);
      Stream* stream = streams[pkt->stream_index];
      stream->broadcast(stream->decode(pkt));
   }
}
