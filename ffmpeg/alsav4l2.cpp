#include "alsav4l2.h"

//TODO: PROPERLY
QList<QPair<QString,QString> > ALSAV4L2::cameras() {
   QList<QPair<QString,QString> > list;
   list.append(QPair<QString,QString>("WebCam SCB-1900N","/dev/video0"));
   return list;
}

//TODO: PROPERLY
QList<QPair<QString,QString> > ALSAV4L2::microphones() {
   QList<QPair<QString,QString> > list;
   list.append(QPair<QString,QString>("ALC269 Analog","hw:0"));
   return list;
}

ALSAV4L2::ALSAV4L2(QString camera, QString microphone) {
   QtConcurrent::run(this, &ALSAV4L2::init, camera, microphone);
}

ALSAV4L2::~ALSAV4L2() {
   avcodec_close(aCodec); delete aCodec;
   avformat_free_context(aFormat); delete aFormat;
   avcodec_close(vCodec); delete vCodec;
   avformat_free_context(vFormat); delete vFormat;
}

void ALSAV4L2::init(QString camera, QString microphone) {
   QMutexLocker l(&initLocker);
   QByteArray camName = camera.toAscii();
   QByteArray micName = microphone.toAscii();

   vFormat = avformat_alloc_context();
   if(avformat_open_input(&vFormat,camName.data(),av_find_input_format("v4l2"),0) < 0)
      cout << "Can't open v4l2 input" << endl;
   if(avformat_find_stream_info(vFormat,0) < 0)
      cout << "Can't find stream info for v4l2" << endl;
   vCodec = vFormat->streams[0]->codec;
   if(!vCodec) cout << "Can't get codec context for v4l2" << endl;
   AVCodec* vDecoder = avcodec_find_decoder(vCodec->codec_id);
   if(!vDecoder) cout << "Can't find decoder for v4l2" << endl;
   if(avcodec_open2(vCodec,vDecoder,0) < 0)
      cout << "Can't associate codec on input for v4l2" << endl;

   aFormat = avformat_alloc_context();
   if(avformat_open_input(&aFormat,micName.data(),av_find_input_format("alsa"),0) < 0)
      cout << "Can't open ALSA input" << endl;
   if(avformat_find_stream_info(aFormat,0) < 0)
      cout << "Can't find stream info for ALSA" << endl;
   aCodec = aFormat->streams[0]->codec;
   if(!aCodec) cout << "Cant get codec context for ALSA" << endl;
   AVCodec* aDecoder = avcodec_find_decoder(aCodec->codec_id);
   if(!aDecoder) cout << "Can't find decoder for ALSA" << endl;
   if(avcodec_open2(aCodec,aDecoder,0) < 0)
      cout << "Can't associate codec on input for ALSA" << endl;

   QtConcurrent::run(this, &ALSAV4L2::videoWorker);
   QtConcurrent::run(this, &ALSAV4L2::audioWorker);
}

void ALSAV4L2::videoWorker() {
   int pts = 0;
   int frameFinished;
   while(true) {
      AVPacket pkt;
      AVFrame* frame = avcodec_alloc_frame();
      //TODO: technically I should check that frame is finished
      if(av_read_frame(vFormat,&pkt) < 0) cout << "Can't read frame" << endl;
      if(avcodec_decode_video2(vCodec,frame,&frameFinished,&pkt) < 0)
         cout << "Can't decode pkt" << endl;
      //TODO: proper pts handling
      frame->pts = pts++;
      emit onNewVideoFrame(QAVFrame(frame));
      av_free_packet(&pkt);
   }
}

void ALSAV4L2::audioWorker() {
   /*
   int pts = 0;
   int got_frame;
   while(true) {
      AVPacket pkt;
      if(av_read_frame(aFormat,&pkt) < 0) cout << "Can't read frame" << endl;
      while(pkt.size > 0) {
         AVFrame* frame = avcodec_alloc_frame();
         int len = avcodec_decode_audio4(aCodec,frame,&got_frame,&pkt);
         //TODO: Check if this is a correct behaviour.
         if(!got_frame) continue;
         if(len < 0) cout << "Can't decode pkt" << endl;
         pkt.data += len;
         pkt.size -= len;
         //TODO: proper pts handling
         frame->pts = pts++;
         emit onNewAudioFrame(QAVFrame(frame));
      }
      av_free_packet(&pkt);
   }
   */
}
