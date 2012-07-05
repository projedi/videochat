#include "ffmpeg.h"
#include <iostream>
using namespace std;

Output::Stream::Stream(StreamInfo info,AVCodec** encoder,Output* owner,int index) throw(int) {
   this->owner = owner;
   this->index = index;
   type = info.type;
   if(!*encoder) {
      CodecID codec_id = (type==Video) ? CODEC_ID_H264
                       : (type==Audio) ? CODEC_ID_MP2
                       : CODEC_ID_NONE;
      *encoder = avcodec_find_encoder(codec_id);
   }
   if(*encoder) {
      codec = avcodec_alloc_context3(*encoder);
      codec->codec_id = (*encoder)->id;
      codec->bit_rate = info.bitrate;
      codec->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
         AVDictionary *dic = 0;
      if(type == Video) {
         //TODO: increase sanity
         if((*encoder)->id == CODEC_ID_RAWVIDEO) codec->pix_fmt = PIX_FMT_RGB32;
         else codec->pix_fmt = (*encoder)->pix_fmts[0];
         codec->width = info.video.width;
         codec->height = info.video.height;
         codec->time_base.num = 1;
         codec->time_base.den = info.video.fps;
         codec->gop_size = 13;
         if(codec->codec_id == CODEC_ID_H264) {
            av_dict_set(&dic,"profile","baseline",0);
            av_dict_set(&dic,"level","13",0);
            codec->pix_fmt = PIX_FMT_YUV420P;
         }
         //Oh, C++, you can't do that yourself.
         scaler = 0;
      } else {
         codec->sample_fmt = (*encoder)->sample_fmts[0];
         codec->sample_rate = info.audio.sampleRate;
         codec->channel_layout = info.audio.channelLayout;
      }
      if(avcodec_open2(codec,*encoder,&dic)<0) throw 1;
   }
}

Output::Stream::~Stream() {
   if(codec) { avcodec_close(codec); av_free(codec); }
   if(type == Video && scaler) sws_freeContext(scaler);
   //else if(type == Audio && resampler) swr_freeContext(&resampler);
}

AVPacket* Output::Stream::encode(AVFrame* frame) {
   if(!(type == Video || type == Audio)) return 0;
   AVFrame* newFrame = avcodec_alloc_frame();
   AVPacket* pkt = new AVPacket();
   av_init_packet(pkt);
   pkt->size = 0;
   int got_packet, res = 0;
   if(type == Video) {
      scaler = sws_getCachedContext( scaler, frame->width, frame->height
                                   , (PixelFormat)frame->format
                                   , codec->width, codec->height, codec->pix_fmt
                                   , SWS_BICUBIC, 0, 0, 0);
      avpicture_alloc( (AVPicture*)newFrame, codec->pix_fmt, codec->width, codec->height);
      sws_scale( scaler, frame->data, frame->linesize, 0, frame->height
               , newFrame->data, newFrame->linesize);
      newFrame->width=codec->width;
      newFrame->height=codec->height;
      newFrame->pts=frame->pts;
      //cout << "On frame: pts=" << frame->pts << endl;
      //av_free(frame);
      res = avcodec_encode_video2(codec, pkt, newFrame, &got_packet);
   } else if(type == Audio) {
      //TODO: Implement
   }
   if(res < 0) { cout << "Couldn't encode" << endl; pkt = 0; }
   if(!res) {
      if(!got_packet) { cout << "Didn't get packet" << endl; av_free_packet(pkt); pkt = 0; }
      else {
         pkt->stream_index = index;
         if(codec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
      }
   }
   avpicture_free((AVPicture*)newFrame);
   av_free(newFrame);
   return pkt;
}

void Output::Stream::sendToOwner(AVPacket* pkt) {
   if(!pkt) cout << "zero packet" << endl;
   else owner->sendPacket(pkt);
}

StreamInfo Output::Stream::info() {
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

int Output::Stream::getIndex() { return index; }

AVCodecContext* Output::Stream::getCodec() { return codec; }

Output::~Output() { }

QList<Output::Stream*> Output::getStreams() const { return streams; }

OutputGeneric::OutputGeneric(QString fmt, QString file) {
   QByteArray formatName = fmt.toAscii();
   QByteArray fileName = file.toAscii();
   avformat_alloc_output_context2(&format,0,formatName.data(),fileName.data());
   avio_open(&format->pb,fileName.data(),AVIO_FLAG_WRITE);
   avformat_write_header(format,0);
}

OutputGeneric::~OutputGeneric() {
   av_write_trailer(format);
   avio_close(format->pb);
   avformat_free_context(format);
}

Output::Stream* OutputGeneric::addStream(StreamInfo info) {
   AVCodec* encoder = 0;
   try {
      Stream* stream = new Stream(info,&encoder,this,format->nb_streams);
      AVStream* avstream = avformat_new_stream(format,encoder);
      avstream->codec = stream->getCodec();
      avformat_write_header(format,0);
      streams.append(stream);
      return stream;
   } catch(...) { return 0; }
}

void OutputGeneric::sendPacket(AVPacket* pkt) {
   //cout << "On out: pts=" << pkt->pts << ";dts=" << pkt->dts << endl;
   pkt->pts = av_rescale_q(pkt->pts,streams[pkt->stream_index]->getCodec()->time_base
                         ,format->streams[pkt->stream_index]->time_base);
   pkt->dts = av_rescale_q(pkt->dts,streams[pkt->stream_index]->getCodec()->time_base
                         ,format->streams[pkt->stream_index]->time_base);
   av_interleaved_write_frame(format,pkt);
   av_free_packet(pkt);
}
