#include "client.h"

Client::Client(QString file) {
   QtConcurrent::run(this, &Client::init, file);
}

Client::~Client() {
   avcodec_close(audioCodec);
   avcodec_close(videoCodec);
   avformat_free_context(format);
   //TODO: Where will it SEGFAULT?
   av_free(audioCodec);
   av_free(videoCodec);
   av_free(format);
}

void Client::init(QString file) {
   QMutexLocker(&initLocker);
   QByteArray fileName = file.toAscii();

   format = avformat_alloc_context();
   if(avformat_open_input(&format,fileName.data(),0,0) < 0)
      cout << "Can't open client input" << endl;
   if(avformat_find_stream_info(format,0) < 0)
      cout << "Can't find stream info for client" << endl;
   
   AVCodec* videoDecoder;
   //TODO: Handle error codes better
   videoStreamIndex = av_find_best_stream(format,AVMEDIA_TYPE_VIDEO,-1,-1,&videoDecoder,0);
   if(!videoDecoder) cout << "Can't find video decoder for client" << endl;
   videoCodec = videoFormat->streams[videoStreamIndex]->codec;
   if(!videoCodec) cout << "Can't get video codec context for client" << endl;
   if(avcodec_open2(videoCodec,videoDecoder,0) < 0)
      cout << "Can't associate video codec on input for client" << endl;

   AVCocec* audioCoder
   audioStreamIndex = av_find_best_stream(format,AVMEDIA_TYPE_AUDIO,-1,-1,&audioDecoder,0);
   if(!audioDecoder) cout << "Can't find audio decoder for client" << endl;
   audioCodec = audioFormat->streams[audioStreamIndex]->codec;
   if(!audioCodec) cout << "Can't get audio codec context for client" << endl;
   if(avcodec_open2(audioCodec,audioDecoder,0) < 0)
      cout << "Can't associate audio codec on input for client" << endl;

   QtConcurrent::run(this, &Client::worker);
}

//TODO: Proper memory management
void Client::worker() {
   AVFrame* frame;
   int pts = 0;
   int frameFinished;
   int got_frame;
   while(true) {
      AVPacket pkt;
      if(av_read_frame(videoFormat,&pkt) < 0) cout << "Can't read frame" << endl;
      if(pkt.stream_index == videoStreamIndex) {
         frame = avcodec_alloc_frame();
         //TODO: technically I should check that frame is finished
         if(avcodec_decode_video2(videoCodec,frame,&frameFinished,&pkt) < 0)
            cout << "Can't decode pkt" << endl;
         //TODO: proper pts handling
         frame->pts = pts++;
         emit onNewVideoFrame(frame);
      } else if(pkt.stream_index == audioStreamIndex) {
         while(pkt.size > 0) {
            frame = avcodec_alloc_frame();
            int len = avcodec_decode_audio4(audioCodec,frame,&got_frame,&pkt);
            //TODO: Check if this is a correct behaviour.
            if(!got_frame) continue;
            if(len < 0) cout << "Can't decode pkt" << endl;
            pkt.data += len;
            pkt.size -= len;
            //TODO: proper pts handling
            frame->pts = pts++;
            emit onNewAudioFrame(frame);
         }
      }
      //TODO: Do I need to do that?
      av_free_packet(&pkt);
   }
}
