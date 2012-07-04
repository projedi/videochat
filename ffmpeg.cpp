#include "ffmpeg.h"

Output::Stream::Stream(StreamInfo info,AVCodec** encoder,Output* owner,int index) {
   this->owner = owner;
   this->index = index;
   type = info.type;
   CodecID codec_id = (type==Video) ? CODEC_ID_H264
                    : (type==Audio) ? CODEC_ID_MP2
                    : CODEC_ID_NONE;
   *encoder = avcodec_find_encoder(codec_id);
   codec = avcodec_alloc_context3(*encoder);
   codec->codec_id = codec_id;
   codec->bit_rate = info.bitrate;
   if(type == Video) {
      codec->pix_fmt = *encoder->pix_fmts[0];
      codec->width = info.video.width;
      codec->height = info.video.height;
      codec->time_base.num = 1;
      codec->time_base.den = info.video.fps;
      codec->gop_size = 13;
   } else {
      codec->sample_fmt = *encoder->sample_fmts[0];
      codec->sample_rate = info.audio.sampleRate;
      codec->channel_layout = info.audio.channelLayout;
   }
   avcodec_open2(codec,*encoder);
}

Output::Stream::~Stream() {
   avcodec_close(codec); av_free(codec);
   if(type == Video && scaler) sws_freeContext(scaler);
   else if(type == Audio && resampler) swr_freeContext(&resampler);
}

AVPacket* Output::Stream::encode(AVFrame* frame) {
   AVFrame* newFrame = avcodec_alloc_frame();
   AVPacket* pkt = new AVPacket;
   av_init_packet(pkt);
   int got_packet;
   if(type == Video) {
      scaler = sws_getCachedContext( scaler, frame->format, frame->width, frame->height
                                   , codec->pix_fmt, codec->width, codec->height
                                   , SWS_BICUBIC, 0, 0, 0);
      avpicture_alloc( (AVPicture*)newFrame, destStream->info().pixelFormat
                     , destStream->info().width, destStream->info().height);
      sws_scale(scaler,frame,newFrame);
      avcodec_encode_video2(codec,pkt,newFrame,&got_packet);
   } else if(type == Audio) {
      //TODO: Implement
   }
   pkt->stream_index = index;
   if(codec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
   av_free(frame);
   avpicture_free((AVPicture*)newFrame);
   av_free(newFrame);
   return pkt;
}

void Output::Stream::sendToOwner(AVPacket* pkt) { owner->sendPacket(pkt); }

StreamInfo Output::Stream::info() {
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

Output::~Output() { }

QList<Stream*> Output::getStreams() const { return streams; }

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

OutputGeneric::OutputGeneric(QString fmt, QString file) {
   QByteArray formatName = fmt.toAscii();
   QByteArray fileName = file.toAscii();
   avformat_alloc_output_context2(&format,0,formatName.data(),fileName.data());
   avio_open(&format->pb,fileName.data(),AVIO_FLAG_WRITE);
   avformat_write_header(format,0);
}

OutputGeneric::~OutputGeneric() {
   avformat_write_trailer(format);
   avio_close(format->pb);
   avformat_free_context(format);
}

Stream* OutputGeneric::addStream(StreamInfo info) {
   AVCodec* encoder;
   Stream* stream = new Stream(info,&encoder,this,format->nb_streams);
   AVStream* avstream = avformat_new_stream(format,encoder);
   streams.append(stream);
   return stream;
}

void OutputGeneric::sendPacket(AVPacket* pkt) {
   /*
   if(pkt->pts != AV_NOPTS_VALUE) {
      pkt->pts = av_rescale_q(pkt->pts, vCodec->time_base, vStream->time_base);
   }
   if(pkt->dts != AV_NOPTS_VALUE) {
      pkt->dts = av_rescale_q(pkt->dts, vCodec->time_base, vStream->time_base);
   }
   */
   av_interleaved_write_frame(format,pkt);
   av_free_packet(pkt);
}

//TODO: implement
AudioHardware::AudioHardware() { }
AudioHardware::~AudioHardware() { }

VideoHardware::VideoHardware() {
#if defined(LINUX)
   QString fmt = "v4l2";
   QPair<QString,QString> device = qMakePair("/dev/video0","Default webcam");
   cameras.append(device);
#elif defined(WIN32)
   QString fmt = "dshow";
#endif
   QList< QPair<QString,QString> >::iterator i;
   for(i=cameras.begin();i!=cameras.end();i++) {
      Input input = new InputGeneric(fmt,(*i)[0]);
      inputs.append(input);
      streams.append(input->getStreams());
   }
}

VideoHardware::~VideoHardware { }

void VideoHardware::worker() {
   QList<Stream*>::iterator i;
   while(state != Paused) {
      for(i=streams.begin();i!=stream.end();i++) {
         if((*i)->getSubscribers().count() > 0) (*i)->getOwner()->setState(Playing);
         else (*i)->getOwner()->setState(Paused);
      }
      sleep(100);
   }
}
