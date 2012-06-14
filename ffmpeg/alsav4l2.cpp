#include "alsav4l2.h"

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
