#include "ffwebcam.h"

ALSAV4L2::ALSAV4L2(QString cameraName, QString microphoneName) {
   QtConcurrent::run(this, &ALSAV4L2::init, cameraName, microphoneName);
}

void ALSAV4L2::init(QString cameraName, QString microphoneName) {
   QMutexLocker(&initLocker);
   QByteArray camName = cameraName.toAscii();
   QByteArray micName = microphoneName.toAscii();

   videoFormat = avformat_alloc_context();
   if(avformat_open_input(&videoFormat,camName.data(),av_find_input_format("v4l2"),0) < 0)
      cout << "Can't open v4l2 input" << endl;
   if(avformat_find_stream_info(videoFormat,0) < 0)
      cout << "Can't find stream info for v4l2" << endl;
   videoCodec = videoFormat->streams[0]->codec;
   if(!videoCodec) cout << "Can't get codec context for v4l2" << endl;
   AVCodec* videoDecoder = avcodec_find_decoder(videoCodec->codec_id);
   if(!videoDecoder) cout << "Can't find decoder for v4l2" << endl;
   if(avcodec_open2(videoCodec,videoDecoder,0) < 0)
      cout << "Can't associate codec on input for v4l2" << endl;

   audioFormat = avformat_alloc_context();
   if(avformat_open_input(&audioFormat,micName.data(),av_find_input_format("alsa"),0) < 0)
      cout << "Can't open ALSA input" << endl;
   if(avformat_find_stream_info(audioFormat,0) < 0)
      cout << "Can't find stream info for ALSA" << endl;
   audioCodec = audioFormat->streams[0]->codec;
   if(!audioCodec) cout << "Cant get codec context for ALSA" << endl;
   AVCodec* audioDecoder = avcodec_find_decoder(audioCodec->codec_id);
   if(!audioDecoder) cout << "Can't find decoder for ALSA" << endl;
   if(avcodec_open2(audioCodec,audioDecoder,0) < 0)
      cout << "Can't associate codec on input for ALSA" << endl;
   QtConcurrent::run(this, &ALSAV4L2::videoWorker);
   QtConcurrent::run(this, &ALSAV4L2::audioWorker);
}

//TODO: Proper memory management
void ALSAV4L2::videoWorker() {
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

//TODO: Implement for a change
void ALSAV4L2::audioWorker() {

}

ALSAV4L2::~ALSAV4L2() {
   avcodec_close(audioCodec);
   avformat_free_context(audioFormat);
   avcodec_close(videoCodec);
   avformat_free_context(videoFormat);
   //TODO: Where will it SEGFAULT?
   av_free(audioCodec);
   av_free(audioFormat);
   av_free(videoCodec);
   av_free(videoFormat);
}

//TODO: PROPERLY
QList<Device> ALSAV4L2::getCameraDevices() {
   QList<Device> list;
   list.append(Device("WebCam SCB-1900N","/dev/video0"));
   return list;
}

//TODO: PROPERLY
QList<Device> ALSAV4L2::getMicrophoneDevices() {
   QList<Device> list;
   list.append(Device("ALC269 Analog","hw:0"));
   return list;
}

Server::Server(QString port) {
   QString addr = "udp://localhost:" + port;
   QByteArray addrName = addr.toAscii();
   //TODO: Should be in settings.
   int w = 640;
   int h = 480;
   int v_bitrate = 400000;
   int fps = 25;
   int gop_size = 13;

   //TODO: Locking GUI is really a bad idea. So, multithreading?
   avformat_alloc_output_context2(&format,0,"mpegts",addrName.data());

   AVCodec* videoEncoder = avcodec_find_encoder(CODEC_ID_H264);
   AVStream* videoStream = avformat_new_stream(format,videoEncoder);
   videoCodec = videoStream->codec;
   avcodec_get_context_defaults3(videoCodec,videoEncoder);
   //videoCodec->codec_id = CODEC_ID_H264;
   if(videoCodec->codec_id != CODEC_ID_H264)
      cout << "And why doesn't codec_id match the prescription?" << endl;
   videoCodec->pix_fmt = videoEncoder->pix_fmts[0];
   videoCodec->width = w;
   videoCodec->height = h;
   videoCodec->bit_rate = v_bitrate;
   videoCodec->time_base.num = 1;
   videoCodec->time_base.den = fps;
   videoCodec->gop_size = gop_size;
   //codec->flags2 |= CODEC_FLAG2_LOCAL_HEADER;
   if(avcodec_open2(videoCodec,videoEncoder,0) < 0) 
      cout << "Can't associate video codec on output for the server" << endl;

   //TODO: Check with AC-3, AAC, DCA, MP1/2/3, mp4a, pcm
   AVCodec* audioEncoder = avcodec_find_encoder(CODEC_ID_H264);
   AVStream* audioStream = avformat_new_stream(format,audioEncoder);
   audioCodec = audioStream->codec;
   avcodec_get_context_defaults3(audioCodec,audioEncoder);
   //videoCodec->codec_id = CODEC_ID_H264;
   //if(videoCodec->codec_id != CODEC_ID_H264)
   //   cout << "And why doesn't codec_id match the prescription?" << endl;
   audioCodec->bit_rate = a_bitrate;
   //TODO: get appropriate channels, sample_fmt and sample_rate
   //audioCodec->sample_fmt = 
   //audioCodec->sample_rate = 44100;
   //audioCodec->channels = 2;

   if(avcodec_open2(audioCodec,audioEncoder,0) < 0) 
      cout << "Can't associate audio codec on output for the server" << endl;

   if(!(format->oformat->flags & AVFMT_NOFILE)) {
      cout << "Warning you, going to open THE FILE" << endl;
      if(avio_open(&format->pb,addrName.data(),AVIO_FLAG_WRITE) < 0) {
         cout << "Can't open a file for writing\n";
      }
   }
   avformat_write_header(format,0);
   //bufsize = 100000 + 12 * codec->width * codec->height;
   //pkt.data = new uint8_t[bufsize];
}

//TODO: Implement
Server::~Server() {

}

//TODO: Implement
void Server::newVideoFrame(AVFrame* frame) {

}

//TODO: Implement
void Server::newAudioFrame(AVFrame* frame) {
   
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
