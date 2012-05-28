#include "ffwebcam.h"

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

Client::Client(QString filename, QString formatname) {
   QByteArray fnameArr = filename.toAscii();
   char* fname = fnameArr.data();
   QByteArray fmtnameArr = formatname.toAscii();
   char* fmtname = formatname.isEmpty() ? 0 : fmtnameArr.data();
   format = avformat_alloc_context();
   if(avformat_open_input(&format,fname,av_find_input_format(fmtname),0) < 0)
      cout << "Can't open input\n";
   initFuture.setFuture(QtConcurrent::run(this, &Client::init));
   workerFuture.setFuture(QtConcurrent::run(this, &Client::worker));
}

Client::~Client() {
   isStopped = true;
   workerFuture.waitForFinished();
   avcodec_close(codec);
   avformat_free_context(format);
}

void Client::init() {
   cout << "Client inits\n";
   if(avformat_find_stream_info(format,0) < 0) cout << "Cant find stream info\n";
   codec = format->streams[0]->codec;
   if(!codec) cout << "Cant get codec context\n";
   AVCodec* decoder;
   if(!codec->codec_id) {
      cout << "Improper hi\n";
      decoder = avcodec_find_decoder(CODEC_ID_JPEG2000);
      avcodec_get_context_defaults3(codec,decoder);
      codec->codec_id = CODEC_ID_JPEG2000;
      codec->pix_fmt = decoder->pix_fmts[0];
      codec->width = 640;
      codec->height = 480;
      codec->bit_rate = 400000;
      codec->time_base.num = 1;
      codec->time_base.den = 25;
      codec->gop_size = 12;
   } else {
      cout << "Proper hi\n";
      decoder = avcodec_find_decoder(codec->codec_id);
   }
   if(!decoder) cout << "Can't find decoder\n";
   cout << "Just before opening codec\n";
   if(avcodec_open2(codec,decoder,0) < 0) cout << "cant associate codec on input\n";
   cout << "Client inits properly\n";
}

void Client::worker() {
   initFuture.waitForFinished();
   AVFrame* frame;
   isStopped = false;
   pts = 0;
   while(!isStopped) {
      AVPacket pkt;
      //cout << "worker:3\n";
      frame = avcodec_alloc_frame();
      int frameFinished = 0;
      while(!frameFinished) {
         if(av_read_frame(format,&pkt) < 0) cout << "cant read frame\n";
         //cout << "worker:3.5\n";
         if(avcodec_decode_video2(codec,frame,&frameFinished,&pkt) < 0)
            cout << "cant decode pkt\n";
      }
      frame->pts = this->pts++;
      //cout << "worker:4\n";
      emit newFrame(frame);
      av_free_packet(&pkt);
   }
}
