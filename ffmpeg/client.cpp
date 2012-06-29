#include "client.h"

Client::Client(QString file) {
   initFuture = QtConcurrent::run(this, &Client::init, file);
}

Client::~Client() {
   avcodec_close(aCodec); delete aCodec;
   avcodec_close(vCodec); delete vCodec;
   avformat_free_context(format); delete format;
}

void Client::init(QString file) {
   QByteArray fileName = file.toAscii();

   format = avformat_alloc_context();
   if(avformat_open_input(&format,fileName.data(),0,0) < 0)
      cout << "Can't open client input" << endl;
   if(avformat_find_stream_info(format,0) < 0)
      cout << "Can't find stream info for client" << endl;
   
   AVCodec* videoDecoder;
   //TODO: Handle error codes better
   vIndex = av_find_best_stream(format,AVMEDIA_TYPE_VIDEO,-1,-1,&videoDecoder,0);
   if(!videoDecoder) cout << "Can't find video decoder for client" << endl;
   vCodec = videoFormat->streams[vIndex]->codec;
   if(!vCodec) cout << "Can't get video codec context for client" << endl;
   if(avcodec_open2(vCodec,videoDecoder,0) < 0)
      cout << "Can't associate video codec on input for client" << endl;

   AVCodec* audioCoder
   aIndex = av_find_best_stream(format,AVMEDIA_TYPE_AUDIO,-1,-1,&audioDecoder,0);
   if(!audioDecoder) cout << "Can't find audio decoder for client" << endl;
   aCodec = audioFormat->streams[aIndex]->codec;
   if(!aCodec) cout << "Can't get audio codec context for client" << endl;
   if(avcodec_open2(aCodec,audioDecoder,0) < 0)
      cout << "Can't associate audio codec on input for client" << endl;

   QtConcurrent::run(this, &Client::worker);
}

//TODO: Proper memory management
void Client::worker() {
   while(true) {
      AVPacket pkt;
      if(av_read_frame(videoFormat,&pkt) < 0) cout << "Can't read frame" << endl;
      if(pkt.stream_index == vIndex) {
         int got_picture;
         AVFrame* frame = avcodec_alloc_frame();
         if(avcodec_decode_video2(vCodec,frame,&got_picture,&pkt) < 0) {
            cout << "Can't decode pkt" << endl;
            continue;
         }
         if(!got_picture) continue;
         frame->pts = av_frame_get_best_effort_timestamp(frame);
         emit onNewVideoFrame(QAVFrame(frame));
         av_free_packet(&pkt);
      } else if(pkt.stream_index == audioStreamIndex) {
         /*
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
         */
      }
   }
}
