#include "server.h"

Server::Server(QString file) {
   initFuture = QtConcurrent::run(this, &Server::init, file);
}

//TODO: Implement
Server::~Server() {
   
}

void Server::newVideoFrame(QAVFrame frame) {
   initFuture.waitForFinished();
   if(videoWorkerFuture.isRunning()) return;
   videoWorkerFuture = QtConcurrent::run(this, &Server::videoWorker, frame);
}

//TODO: Add some thread pausing 
void Server::videoWorker(QAVFrame frame) {
   QAVFrame newFrame = rescale(frame);
   AVPacket pkt = new AVPacket;
   av_init_packet(pkt);
   int got_packet;
   int res = avcodec_encode_video2(vCodec, pkt, newFrame.data(), &got_packet);
   if(res >= 0 && got_packet) {
      if(pkt->pts != AV_NOPTS_VALUE) {
         pkt->pts = av_rescale_q(pkt->pts, vCodec->time_base, vStream->time_base);
      }
      if(pkt->dts != AV_NOPTS_VALUE) {
         pkt->dts = av_rescale_q(pkt->dts, vCodec->time_base, vStream->time_base);
      }
      if(vCodec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
      pkt->stream_index = vStream->index; 
      av_interleaved_write_frame(format,pkt);
      av_free_packet(pkt);
   }
}

void Server::newAudioFrame(QAVFrame frame) {
   initFuture.waitForFinished();
   if(audioWorker.isRunning()) return;
   audioWorkerFuture = QtConcurrent::run(this, &Server::audioWorker, frame);
}

void Server::audioWorker(QAVFrame frame) {
   QAVFrame newFrame = resample(frame);
   AVPacket pkt = new AVPacket;
   av_init_packet(pkt);
   int got_packet;
   int res = avcodec_encode_audio2(aCodec, pkt, newFrame.data(), &got_packet);
   if(res >= 0 && got_packet) {
      if(pkt->pts != AV_NOPTS_VALUE) {
         pkt->pts = av_rescale_q(pkt->pts, aCodec->time_base, aStream->time_base);
      }
      if(pkt->dts != AV_NOPTS_VALUE) {
         pkt->dts = av_rescale_q(pkt->dts, aCodec->time_base, aStream->time_base);
      }
      if(aCodec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
      pkt->stream_index = aStream->index; 
      av_interleaved_write_frame(format,pkt);
      av_free_packet(pkt);
   }
}

void Server::init(QString file) {
   QByteArray fileName = file.toAscii();
   //TODO: Should be in settings and by default values
   //TODO: from codec.
   int w = 640;
   int h = 480;
   int v_bitrate = 400000;
   int fps = 25;
   int gop_size = 13;
   int a_bitrate = 64000;
   AVSampleFormat sample_fmt = AV_SAMPLE_FMT_S16;
   int sample_rate = 32000;
   int channels = 2;

   cout << "Opening to" << fileName.data() << endl;
   avformat_alloc_output_context2(&format,0,"mpegts",fileName.data());

   //TODO: make x264 use lower settings.
   AVCodec* vEncoder = avcodec_find_encoder(CODEC_ID_H264);
   vStream = avformat_new_stream(format,vEncoder);
   vCodec = vStream->codec;
   avcodec_get_context_defaults3(vCodec,vEncoder);
   //It's really required. No idea why. Seems like a bug in avcodec.
   vCodec->codec_id = CODEC_ID_H264;
   vCodec->pix_fmt = vEncoder->pix_fmts[0];
   vCodec->width = w;
   vCodec->height = h;
   vCodec->bit_rate = v_bitrate;
   vCodec->time_base.num = 1;
   vCodec->time_base.den = fps;
   vCodec->gop_size = gop_size;
   vCodec->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
   if(avcodec_open2(vCodec,vEncoder,0) < 0) 
      cout << "Can't associate video codec on output for the server" << endl;
   
   //TODO: Check with AC-3, AAC, DCA, MP1/2/3, mp4a, pcm, speex
   AVCodec* aEncoder = avcodec_find_encoder(CODEC_ID_MP2);
   aStream = avformat_new_stream(format,aEncoder);
   aCodec = aStream->codec;
   avcodec_get_context_defaults3(aCodec,aEncoder);
   aCodec->codec_id = CODEC_ID_MP2;
   aCodec->bit_rate = a_bitrate;
   aCodec->sample_fmt = sample_fmt;
   aCodec->sample_rate = sample_rate;
   aCodec->channels = channels;
   if(avcodec_open2(aCodec,aEncoder,0) < 0) 
      cout << "Can't associate audio codec on output for the server" << endl;

   if(avio_open(&format->pb,fileName.data(),AVIO_FLAG_WRITE) < 0)
      cout << "Can't open a file for writing" << endl;
   avformat_write_header(format,0);
}
