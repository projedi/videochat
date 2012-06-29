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
   AVPacket *pkt = new AVPacket;
   av_init_packet(pkt);
   pkt->data = 0;
   pkt->size = 0;
   int got_packet;
   int res = avcodec_encode_video2(vCodec, pkt, frame.data(), &got_packet);
   if(res >= 0 && got_packet) {
      if(pkt->pts != AV_NOPTS_VALUE) {
         pkt->pts = av_rescale_q(pkt->pts, vCodec->time_base, vStream->time_base);
      }
      if(pkt->dts != AV_NOPTS_VALUE) {
         pkt->dts = av_rescale_q(pkt->dts, vCodec->time_base, vStream->time_base);
      }
      //if(vCodec->coded_frame->pts != AV_NOPTS_VALUE)
      //   pkt->pts = av_rescale_q(vCodec->coded_frame->pts,vCodec->time_base
      //                         ,vStream->time_base);
      if(vCodec->coded_frame->key_frame) pkt->flags|=AV_PKT_FLAG_KEY;
      pkt->stream_index = vStream->index; 
      int ret = av_interleaved_write_frame(format,pkt);
      av_free_packet(pkt);
   }
}

void Server::newAudioFrame(QAVFrame frame) {
   /*
   initFuture.waitForFinished();
   AVPacket pkt;
   av_init_packet(&pkt);
   int got_packet = 0;
   if(!avcodec_encode_audio2(aCodec, &pkt, frame.data(), &got_packet) && got_packet) {
      if(aCodec->coded_frame->pts != AV_NOPTS_VALUE)
         pkt.pts = av_rescale_q(aCodec->coded_frame->pts,aCodec->time_base
                               ,aStream->time_base);
      if(aCodec->coded_frame->key_frame) pkt.flags|=AV_PKT_FLAG_KEY;
      pkt.stream_index = aStream->index; 
      int ret = av_interleaved_write_frame(format,&pkt);
      cout << ret << endl;
   }
   //TODO: When to free this pkt?
   */
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
  /* 
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
   */
}

/*
Server::Server(QString filename, QString formatname, CodecID codecID
              ,int w, int h, int bitrate, int fps, int gop_size) {
   QByteArray fnameArr = filename.toAscii();
   char* fname = fnameArr.data();
   QByteArray fmtnameArr = formatname.toAscii();
   char* fmtname = fmtnameArr.data();
   avformat_alloc_output_context2(&format,0,fmtname,fname);
   AVOutputFormat *fmt = format->oformat;
   AVCodec* encoder = avcodec_find_encoder(codecID);
   video_st = avformat_new_stream(format,encoder);
   codec = video_st->codec;
   avcodec_get_context_defaults3(codec,encoder);
   codec->codec_id = codecID;
   codec->pix_fmt = encoder->pix_fmts[0];
   codec->width = w;
   codec->height = h;
   codec->bit_rate = bitrate;
   codec->time_base.num = 1;
   codec->time_base.den = fps;
   codec->gop_size = gop_size;
   codec->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
   //if(fmt->flags & AVFMT_GLOBALHEADER) codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
   if(avcodec_open2(codec,encoder,0) < 0) 
      cout << "cant associate codec on output\n";

   if(!(fmt->flags & AVFMT_NOFILE)) {
      if(avio_open(&format->pb, fname,AVIO_FLAG_WRITE) < 0) {
         cout << "Can't open a file for writing\n";
      }
   }
   avformat_write_header(format,0);
   bufsize = 100000 + 12 * codec->width * codec->height;
   pkt.data = new uint8_t[bufsize];
   //isBusy = false;
}

Server::~Server() {
   delete[] pkt.data;
   avcodec_close(codec);
   avformat_free_context(format);
}

void Server::onFrame(AVFrame* frame) {
   //if(!mutex.tryLock()) return;
   QMutexLocker locker(&mutex);
   //if(isBusy) return;
   //isBusy = true;
   av_init_packet(&pkt);
   int outsize = avcodec_encode_video(codec,pkt.data,bufsize,frame);
   if(outsize > 0) {
      if(codec->coded_frame->pts != AV_NOPTS_VALUE)
         pkt.pts = av_rescale_q(codec->coded_frame->pts,codec->time_base
                               ,video_st->time_base);
      if(codec->coded_frame->key_frame) pkt.flags|=AV_PKT_FLAG_KEY;
      pkt.stream_index = video_st->index;
      pkt.size = outsize;
      av_interleaved_write_frame(format,&pkt);
   }
   //isBusy = false;
   //mutex.unlock();
}

Source::Source(Device video, Device audio) {
   QtConcurrent::run(this, &Source::init, video, audio);
}

Source::~Source() {
   //TODO Gracefully handle worker and init threads
   avcodec_close(codec);
   avformat_free_context(format);
   av_free(codec);
   av_free(format);
}

int Source::getWidth() { QMutexLocker(initLocker); return codec->width; }
int Source::getHeight() { QMutexLocker(initLocker); return codec->height; }
PixelFormat Source::getPixelFormat() {
   QMuteLocker(initLocker);
   return codec->pix_fmt;
}

void Source::init(Device videoDevice, Device audioDevice) {
   initLocker.lock();
   cout << "Client inits\n";
   QByteArray videoFormatName = videoDevice.ffmpegFormat.toAscii();
   QByteArray videoFileName = videoDevice.ffmpegName.toAscii();
   QByteArray audioFormatName = audioDevice.ffmpegFormat.toAscii();
   QByteArray audioFileName = audioDevice.ffmpegName.toAscii();

   videoFormat = avformat_alloc_context();
   if(avformat_open_input(&videoFormat,videoFileName.data()
                         ,av_find_input_format(videoFormatName.data()),0) < 0)
      cout << "Can't open input\n";
   if(avformat_find_stream_info(format,0) < 0) cout << "Cant find stream info\n";
   cout << "Client finds stream info\n";
   //TODO proper video lookup
   codec = format->streams[0]->codec;
   if(!codec) cout << "Cant get codec context\n";
   AVCodec* decoder = avcodec_find_decoder(codec->codec_id);
   if(!decoder) cout << "Can't find decoder\n";
   cout << "Just before opening codec\n";
   if(avcodec_open2(codec,decoder,0) < 0) cout << "cant associate codec on input\n";
   cout << "Client inits properly\n";
   initLocker.unlock();
   worker();
   //workerFuture.setFuture(QtConcurrent::run(this, &Client::worker));
}

void Source::worker() {
   AVFrame* frame;
   int pts = 0;
   while(true) {
      AVPacket pkt;
      frame = avcodec_alloc_frame();
      int frameFinished = 0;
      while(!frameFinished) {
         if(av_read_frame(format,&pkt) < 0) cout << "cant read frame\n";
         if(avcodec_decode_video2(codec,frame,&frameFinished,&pkt) < 0)
            cout << "cant decode pkt\n";
      }
      av_free_packet(&pkt);
      //TODO proper pts handling
      frame->pts = pts++;
      emit newFrame(frame);
   }
}
*/
