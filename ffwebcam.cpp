#include "ffwebcam.h"

const PixelFormat QT_PIX_FMT = PIX_FMT_RGB32;
const PixelFormat STREAM_PIX_FMT = PIX_FMT_YUV420P;

WebcamParams defaultParams() {
    WebcamParams params;
    params.video_size = "sd";
    params.device_id = 0;
    params.fps = 25;
    return params;
}

Webcam::Webcam(WebcamParams params) {
   this->params = params;
   AVDictionary* options = 0;
   av_dict_set(&options, "video_size", params.size, 0);
   av_dict_set(&options, "framerate", params.fps, 0);
   if(avformat_open_input(&formatContext,params.device_name,0,0) < 0)
      throw "Can't open input";
   av_dict_free(&options);
   if(avformat_find_stream_info(formatContext,0) < 0) throw "Can't find stream info";
   int streamID = av_find_best_stream(formatContext,AVMEDIA_TYPE_VIDEO,-1,-1,&decoder,0);
   if(streamID < 0) throw "Can't find best stream";
   codecContext = formatContext->streams[streamID]->codec;
   if(avcodec_open2(codecContext,decoder,0) < 0) cout << "cant associate codec on input\n";
   QtConcurrent::run(Webcam::mainLoop);
}

void Webcam::mainLoop() {
   AVPacket pkt;
   AVFrame* frame;
   int frameFinished;
   while(1) {
      frame = avcodec_alloc_frame();
      frameFinished = 0;
      while(!frameFinished) {
         if(av_read_frame(formatContext,&pkt) < 0) cout << "cant read frame\n";
         if(avcodec_decode_video2(codecContext,frame,&frameFinished,&pkt) < 0)
            cout << "cant decode pkt\n";
      }
      // TODO should copy instead
      emit frameArrived(frame);
   }
   av_free_packet(&pkt);
}

Server::Server(ServerParams params) {
   this->params = params;
   const AVOption* opt = 0;

   //Constructing output sink
   char* filename = "udp://localhost:8080?ttl=10";
   //char* filename = "/tmp/out.m4v";
   avformat_alloc_output_context2(&outputFormat,0,"m4v",filename);
   AVOutputFormat *fmt = outputFormat->oformat;
   CodecID codec_id = CODEC_ID_H264;
   //CodecID codec_id = fmt->video_codec;
   AVCodec* encoder = avcodec_find_encoder(codec_id);
   video_st = avformat_new_stream(outputFormat,encoder);
   outputCodec = video_st->codec;
   avcodec_get_context_defaults3(outputCodec,encoder);
   outputCodec->codec_id = codec_id;
   outputCodec->width = inputCodec->width;
   outputCodec->height = inputCodec->height;
   outputCodec->pix_fmt = STREAM_PIX_FMT;
   outputCodec->bit_rate = 400000;
   outputCodec->time_base.num = 1;
   outputCodec->time_base.den = 25;
   outputCodec->gop_size = 12;
   if(fmt->flags & AVFMT_GLOBALHEADER) outputCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
   //const AVOption* opt = av_opt_find(outputFormat,"ts",0,0,0);
   //cout << opt->name << ": " << opt->help << endl;
   /*
   cout << "Format options:\n";
   for(;;) {
      opt = av_opt_next(outputFormat, opt);
      if(!opt) break;
      cout << opt->name;
      if(opt->unit != 0) cout << "(" << opt->unit << ")";
      if(opt->help != 0) cout << ": " << opt->help;
      cout << endl;
   }
   cout << "Codec options:\n";
   for(;;) {
      opt = av_opt_next(outputCodec, opt);
      if(!opt) break;
      cout << opt->name;
      if(opt->unit != 0) cout << "(" << opt->unit << ")";
      if(opt->help != 0) cout << ": " << opt->help;
      cout << endl;
   }
   */
   if(avcodec_open2(outputCodec,encoder,0) < 0) 
      cout << "cant associate codec on output\n";

   if(!(fmt->flags & AVFMT_NOFILE)) {
      cout << "We have a file over here\n";
      if(avio_open(&outputFormat->pb, filename,AVIO_FLAG_WRITE) < 0) {
         cout << "But I cant write into it\n";
      }
   }
   avformat_write_header(outputFormat,0);

   convertQt = sws_getContext(inputCodec->width,inputCodec->height,inputCodec->pix_fmt
                             ,inputCodec->width,inputCodec->height,QT_PIX_FMT
                             ,SWS_BICUBIC,0,0,0);
   convertStream = sws_getContext(inputCodec->width,inputCodec->height,inputCodec->pix_fmt
                                 ,inputCodec->width,inputCodec->height,STREAM_PIX_FMT
                                 ,SWS_BICUBIC,0,0,0);

   this->pts = 0;
}

Webcam::~Webcam() {
   grabTimer->stop();
   delete grabTimer;

   av_free(convertQt);
   av_free(convertStream);

   avcodec_close(inputCodec);
   av_free(inputCodec);
   avformat_free_context(inputFormat);
}

void Webcam::start() { grabTimer->start(1000/params.fps); }

void Webcam::stop() { grabTimer->stop(); }

void frameToOutput(AVFormatContext* fmt, AVCodecContext* codec, AVStream* st, AVFrame* frame) {
   int bufsize = 100000 + 12 * codec->width * codec->height;
   uint8_t* buffer = new uint8_t[bufsize];
   int outsize = avcodec_encode_video(codec,buffer,bufsize,frame);
   if(outsize > 0) {
      AVPacket pkt;
      av_init_packet(&pkt);
      if(codec->coded_frame->pts != AV_NOPTS_VALUE)
         pkt.pts = av_rescale_q(codec->coded_frame->pts,codec->time_base, st->time_base);
      if(codec->coded_frame->key_frame) pkt.flags|=AV_PKT_FLAG_KEY;
      pkt.stream_index = st->index;
      pkt.data = buffer;
      pkt.size = outsize;
      av_interleaved_write_frame(fmt,&pkt);
   }
   delete[] buffer;
}

AVFrame* grabFrame(AVFormatContext* fmt, AVCodecContext* codec) {
   AVPacket pkt;
   AVFrame* frame = avcodec_alloc_frame();
   int frameFinished = 0;
   while(!frameFinished) {
      if(av_read_frame(fmt,&pkt) < 0) cout << "cant read frame\n";
      if(avcodec_decode_video2(codec,frame,&frameFinished,&pkt) < 0)
         cout << "cant decode pkt\n";
   }
   av_free_packet(&pkt);
   return frame;
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

void Webcam::grabTimerTimeout() {
   AVFrame* frame = grabFrame(this->inputFormat,this->inputCodec);
   frame->pts = this->pts++;
   AVFrame* qtframe = convertFrame(convertQt,QT_PIX_FMT,frame);
   AVFrame* sframe = convertFrame(convertStream,STREAM_PIX_FMT,frame);
   emit frameArrived(new QImage(qtframe->data[0],qtframe->width
                    ,qtframe->height,QImage::Format_RGB32));
   frameToOutput(this->outputFormat,this->outputCodec,this->video_st,sframe);
   av_free(frame);
   av_free(qtframe);
   av_free(sframe);
}

