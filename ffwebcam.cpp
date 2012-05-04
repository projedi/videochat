#include "ffwebcam.h"

Client* getRemote(char* uri) {
   return new Client(uri);
}

FFmpeg::FFmpeg() {
   char* serverURI = "udp://localhost:8080";
   server = new Server(serverURI,"m4v",CODEC_ID_VP8);
   camera = new Client("/dev/video0","v4l2");
   int width = 640;
   int height = 480;
   PixelFormat pix_fmt = PIX_FMT_RGB32;
   cameraToServer = sws_getContext(camera->getWidth(),camera->getHeight()
                                  ,camera->getPixelFormat(),server->getWidth()
                                  ,server->getHeight(),server->getPixelFormat()
                                  ,SWS_BICUBIC,0,0,0);
   cameraToLocal = sws_getContext(camera->getWidth(),camera->getHeight()
                                 ,camera->getPixelFormat(),width,height,pix_fmt
                                 ,SWS_BICUBIC,0,0,0);
   connect(camera,SIGNAL(newFrame(AVFrame*)),this,SLOT(onCameraFrame(AVFrame*)));
   connect(this,SIGNAL(newServerFrame(AVFrame*)),server,SLOT(sendFrame(AVFrame*)));

   cout << "Just before remote\n";
   remoteFuture.setFuture(QtConcurrent::run(getRemote,serverURI));
   connect(&remoteFuture,SIGNAL(finished()),this,SLOT(onRemoteConstruction()));
   cout << "FFmpeg constructor finished\n";
}

void FFmpeg::onRemoteConstruction() {
   int width = 640;
   int height = 480;
   PixelFormat pix_fmt = PIX_FMT_RGB32;
   remote = remoteFuture.result(); 
   cout << "Just after remote\n";
   cameraToLocal = sws_getContext(remote->getWidth(),remote->getHeight()
                                 ,remote->getPixelFormat(),width,height,pix_fmt
                                 ,SWS_BICUBIC,0,0,0);
   connect(remote,SIGNAL(newFrame(AVFrame*)),this,SLOT(onRemoteFrame(AVFrame*)));
   cout << "Remote done\n";
}

FFmpeg::~FFmpeg() {
   delete server;
   delete camera;
   if(remote) delete remote;
   av_free(cameraToServer);
   av_free(cameraToLocal);
   av_free(remoteToLocal);
}

AVFrame* convertFrame(SwsContext* convert, PixelFormat pix_fmt, AVFrame* frame) {
   AVFrame* newframe = new AVFrame;
   int bufsize = avpicture_get_size(pix_fmt,frame->width,frame->height);
   uint8_t* buffer = (uint8_t*)av_malloc(bufsize*sizeof(uint8_t));
   avpicture_fill((AVPicture*)newframe,buffer,pix_fmt,frame->width,frame->height);
   sws_scale(convert,frame->data,frame->linesize,0,frame->height
            ,newframe->data,newframe->linesize);
   // TODO is there a way to copy all metas to new frame automagically?
   newframe->width = frame->width;
   newframe->height = frame->height;
   newframe->pts = frame->pts;
   return newframe;
}

void FFmpeg::onCameraFrame(AVFrame* frame) {
   AVFrame* toLocal = convertFrame(cameraToLocal,PIX_FMT_RGB32,frame);
   AVFrame* toServer = convertFrame(cameraToServer,server->getPixelFormat(),frame);
   av_free(frame);
   QtConcurrent::run(server, &Server::sendFrame, toServer);
   emit newRawCameraFrame(toLocal->data[0],toLocal->width,toLocal->height);
}

void FFmpeg::onRemoteFrame(AVFrame* frame) {
   AVFrame* toLocal = convertFrame(remoteToLocal,PIX_FMT_RGB32,frame);
   av_free(frame);
   emit newRawRemoteFrame(toLocal->data[0],toLocal->width,toLocal->height);
}

Server::Server(char* filename, char* fmtname, CodecID codecID) {
   avformat_alloc_output_context2(&format,0,fmtname,filename);
   AVOutputFormat *fmt = format->oformat;
   AVCodec* encoder = avcodec_find_encoder(codecID);
   video_st = avformat_new_stream(format,encoder);
   codec = video_st->codec;
   avcodec_get_context_defaults3(codec,encoder);
   codec->codec_id = codecID;
   codec->pix_fmt = encoder->pix_fmts[0];
   codec->width = 640;
   codec->height = 480;
   codec->bit_rate = 400000;
   codec->time_base.num = 1;
   codec->time_base.den = 25;
   codec->gop_size = 12;
   if(fmt->flags & AVFMT_GLOBALHEADER) codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
   if(avcodec_open2(codec,encoder,0) < 0) 
      cout << "cant associate codec on output\n";

   if(!(fmt->flags & AVFMT_NOFILE)) {
      if(avio_open(&format->pb, filename,AVIO_FLAG_WRITE) < 0) {
         cout << "Can't open a file for writing\n";
      }
   }
   avformat_write_header(format,0);
   bufsize = 100000 + 12 * codec->width * codec->height;
   pkt.data = new uint8_t[bufsize];
   isBusy = false;
}

Server::~Server() {
   delete[] pkt.data;
   avcodec_close(codec);
   avformat_free_context(format);
}

void Server::sendFrame(AVFrame* frame) {
   if(isBusy) return;
   isBusy = true;
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
   isBusy = false;
}

Client::Client(char* filename, char* fmtname) {
   format = avformat_alloc_context();
   if(avformat_open_input(&format,filename,av_find_input_format(fmtname),0) < 0)
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
   if(avformat_find_stream_info(format,0) < 0) cout << "Cant find stream info\n";
   codec = format->streams[0]->codec;
   if(!codec) cout << "Cant get codec context\n";
   AVCodec* decoder = avcodec_find_decoder(codec->codec_id);
   if(!decoder) cout << "Can't find decoder\n";
   if(avcodec_open2(codec,decoder,0) < 0) cout << "cant associate codec on input\n";

   pts = 0;
   isStopped = false;
}

void Client::worker() {
   initFuture.waitForFinished();
   AVPacket pkt;
   AVFrame* frame;
   isStopped = false;
   while(!isStopped) {
      cout << "worker:3\n";
      frame = avcodec_alloc_frame();
      int frameFinished = 0;
      while(!frameFinished) {
         if(av_read_frame(format,&pkt) < 0) cout << "cant read frame\n";
         if(avcodec_decode_video2(codec,frame,&frameFinished,&pkt) < 0)
            cout << "cant decode pkt\n";
      }
      frame->pts = this->pts++;
      cout << "worker:4\n";
      emit newFrame(frame);
   }
   av_free_packet(&pkt);
}
