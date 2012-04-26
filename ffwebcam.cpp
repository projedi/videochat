#include "ffwebcam.h"

WebcamParams defaultParams() {
    WebcamParams params;
    params.device_id = 0;
    params.fps = 25;
    params.bufferSize = 10;
    params.port = 8080;
    return params;
}

Webcam::Webcam(WebcamParams params) {
    this->params = params;
    grabTimer = new QTimer();
    connect(grabTimer,SIGNAL(timeout()),this,SLOT(grabTimerTimeout()));
    inputFormatContext = avformat_alloc_context();
    //TODO v4l2 and device number
    if(avformat_open_input(&inputFormatContext,"/dev/video0",av_find_input_format("v4l2"),0) < 0) cout << "Can't open device\n";
    if(avformat_find_stream_info(inputFormatContext,0) < 0) cout << "Cant find stream info\n";
    //TODO stream number
    inputStream = inputFormatContext->streams[0];
    avctx = inputStream->codec;
    if(!avctx) cout << "Cant get codec context\n";
    AVCodec* codec = avcodec_find_decoder(avctx->codec_id);
    if(!codec) cout << "Can't find decoder\n";
    if(avcodec_open2(avctx,codec,0) < 0) cout << "cant associate codec\n";
    PixelFormat pix_fmt = PIX_FMT_RGB32;
    img_convert_ctx = sws_getContext(avctx->width,avctx->height,avctx->pix_fmt,avctx->width,avctx->height,pix_fmt,SWS_BICUBIC,0,0,0);
    if(!img_convert_ctx) cout << "Cant construct swscontext\n";
    int bufsize = avpicture_get_size(pix_fmt,avctx->width,avctx->height);
    cout << "Desired bufsize: " << bufsize << endl;
    uint8_t* buffer = (uint8_t*)av_malloc(bufsize*sizeof(uint8_t));
    pict = new AVPicture;
    avpicture_fill(pict,buffer,pix_fmt,avctx->width,avctx->height);
}

void Webcam::start() {
    grabTimer->start(1000/params.fps);
}

void Webcam::stop() {
    grabTimer->stop();
}

void Webcam::grabTimerTimeout() {
    AVPacket pkt;
    AVFrame* avframe = avcodec_alloc_frame();
    int frameFinished = 0;
    while(!frameFinished){
        if(av_read_frame(inputFormatContext,&pkt) < 0) cout << "cant read frame\n";
        if(avcodec_decode_video2(avctx,avframe,&frameFinished,&pkt) < 0) cout << "cant decode pkt\n";
    }
    av_free_packet(&pkt);
    sws_scale(img_convert_ctx,avframe->data,avframe->linesize,0,avctx->height,pict->data,pict->linesize);
    av_free(avframe);
    QImage frame(pict->data[0],avctx->width,avctx->height,QImage::Format_RGB32);
    emit frameArrived(frame);
}
