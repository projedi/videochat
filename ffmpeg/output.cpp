#include "ffmpeg.h"
#include <iostream>
using namespace std;

OutputStream::OutputStream(StreamInfo info,AVCodec* encoder,Output* owner,int index) {
   this->owner = owner;
   this->index = index;
   type = info.type;
   //Oh, C++, you can't do that yourself.
   codec = 0;
   scaler = 0;
   if(encoder) {
      codec = avcodec_alloc_context3(encoder);
      codec->codec_id = encoder->id;
      codec->bit_rate = info.bitrate;
      //TODO: remove for case of file output
      codec->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
      if(type == Video) {
         if(encoder->pix_fmts) {
            const PixelFormat* pix_fmt = encoder->pix_fmts;
            for(; *pix_fmt != PIX_FMT_NONE; pix_fmt++) {
               if(info.video.pixelFormat == *pix_fmt) {
                  codec->pix_fmt = *pix_fmt;
                  break;
               }
            }
            if(*pix_fmt == PIX_FMT_NONE) codec->pix_fmt = encoder->pix_fmts[0];
         } else codec->pix_fmt = info.video.pixelFormat;
         codec->width = info.video.width;
         codec->height = info.video.height;
         codec->time_base.num = 1;
         codec->time_base.den = info.video.fps;
         //TODO: move to settings
         codec->gop_size = 5;
//#ifdef LINUX
         codec->bit_rate = 1000000;
//#endif
      } else {
         //TODO: implement correctly
         codec->sample_fmt = encoder->sample_fmts[0];
         codec->sample_rate = info.audio.sampleRate;
         codec->channel_layout = info.audio.channelLayout;
      }
      if(avcodec_open2(codec,encoder,0)<0) throw 1;
   }
}

OutputStream::~OutputStream() {
   if(codec) avcodec_close(codec);
   if(type == Video && scaler) sws_freeContext(scaler);
   if(type == Audio && resampler) swr_free(&resampler);
}

void OutputStream::process(AVFrame* frame) {
   if(!(type == Video || type == Audio)) return;
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
      //TODO: not so sure
      newFrame->pts=frame->pts;
      //cout << "On frame: pts=" << frame->pts << endl;
      res = avcodec_encode_video2(codec, pkt, newFrame, &got_packet);
   } else if(type == Audio) {
      //TODO: Implement
   }
   if(res < 0 ) return;
   if(!got_packet) { av_free_packet(pkt); return; }
   avpicture_free((AVPicture*)newFrame);
   av_free(newFrame);
   pkt->stream_index = index;
   if(codec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
   owner->sendPacket(pkt);
}

StreamInfo OutputStream::info() {
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

AVCodecContext* OutputStream::getCodec() { return codec; }

Output::~Output() {
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
}

QList<OutputStream*> Output::getStreams() const { return streams; }

OutputGeneric::OutputGeneric(QString fmt, QString file) {
   QByteArray formatName = fmt.toAscii();
   QByteArray fileName = file.toAscii();
   avformat_alloc_output_context2(&format,0,formatName.data(),fileName.data());
   avio_open(&format->pb,fileName.data(),AVIO_FLAG_WRITE);
   avformat_write_header(format,0);
}

OutputGeneric::~OutputGeneric() {
   av_write_trailer(format);
   logger("Closing file");
   avio_close(format->pb);
   // It seems like duplication from supertype destructor. But avformat_free_context
   // bluntly frees(without closing) codecs and i have to close each codec before doing
   // that.
   for(int i = 0; i < streams.count(); i++) { if(streams[i]) delete streams[i]; }
   streams.clear();
   logger("freeing context");
   avformat_free_context(format);
}

OutputStream* OutputGeneric::addStream(StreamInfo info) {
   try {
      CodecID codec_id = (info.type==Video) ? CODEC_ID_MJPEG
                       : (info.type==Audio) ? CODEC_ID_MP2
                       : CODEC_ID_NONE;
      //TODO: what will it do when CODEC_ID_NONE
      AVCodec *encoder = avcodec_find_encoder(codec_id);
      OutputStream* stream = new OutputStream(info,encoder,this,format->nb_streams);
      AVStream* avstream = avformat_new_stream(format,encoder);
      avstream->codec = stream->getCodec();
      //TODO: Check what happens if i don't do that. Especially when writing to a file.
      // As a matter of fact it's interesting what happens if I leave that and still will
      // be writing to a file. To be more precise I have no idea how to handle all that
      // dynamic streams when i have a file. Maybe I should implement commit function
      // which locks all that dynamic behavior up.
      //TODO: I remember that I should put GLOBAL_HEADER flag somewhere when it's
      // required
      avformat_write_header(format,0);
      streams.append(stream);
      return stream;
   } catch(...) { return 0; }
}

void OutputGeneric::removeStream(OutputStream* stream) {
   int str_index = streams.indexOf(stream);
   streams.removeAt(str_index);
   delete stream;
   AVStream* avstream = format->streams[str_index];
   format->nb_streams--;
   for(int i = str_index; i < format->nb_streams; i++) {
      format->streams[i] = format->streams[i+1];
   }
   avformat_write_header(format,0);

   if(avstream->parser) av_parser_close(avstream->parser);
   if(avstream->attached_pic.data) av_free_packet(&avstream->attached_pic);
   av_dict_free(&avstream->metadata);
   av_freep(&avstream->index_entries);
   av_freep(&avstream->codec->extradata);
   av_freep(&avstream->codec->subtitle_header);
   av_freep(&avstream->codec);
   av_freep(&avstream->priv_data);
   av_freep(&avstream->info);
   av_freep(&avstream);
}

void OutputGeneric::sendPacket(AVPacket* pkt) {
   //cout << "On out: pts=" << pkt->pts << ";dts=" << pkt->dts << endl;
   av_interleaved_write_frame(format,pkt);
   av_free_packet(pkt);
}
