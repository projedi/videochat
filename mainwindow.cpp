#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

AVFrame* convertFrame(SwsContext* convert, PixelFormat pix_fmt, int w, int h, AVFrame* frame) {
   AVFrame* newframe = new AVFrame;
   int bufsize = avpicture_get_size(pix_fmt,w,h);
   uint8_t* buffer = (uint8_t*)av_malloc(bufsize*sizeof(uint8_t));
   avpicture_fill((AVPicture*)newframe,buffer,pix_fmt,w,h);
   sws_scale(convert,frame->data,frame->linesize,0,frame->height
            ,newframe->data,newframe->linesize);
   // TODO is there a way to copy all metas to new frame automagically?
   newframe->width = w;
   newframe->height = h;
   newframe->pts = frame->pts;
   return newframe;
}

void MainWindow::onRemoteFrame(AVFrame* frame){
   int w = ui->labelRemote->width();
   int h = ui->labelRemote->height();
   AVFrame* newframe = convertFrame(remoteToLocal,PIX_FMT_RGB32,w,h,frame);
   QImage img(newframe->data[0],newframe->width,newframe->height,QImage::Format_RGB32);
   ui->labelRemote->setPixmap(QPixmap::fromImage(img));
   av_free(frame);
   av_free(newframe);
}

void MainWindow::onCameraFrame(AVFrame* frame){
   int w = ui->labelWebcam->width();
   int h = ui->labelWebcam->height();
   AVFrame* localFrame = convertFrame(cameraToLocal,PIX_FMT_RGB32,w,h,frame);
   QImage img(localFrame->data[0],localFrame->width,localFrame->height
             ,QImage::Format_RGB32);
   ui->labelWebcam->setPixmap(QPixmap::fromImage(img));
   AVFrame* serverFrame = convertFrame(cameraToServer,server->getPixelFormat()
                                      ,server->getWidth(),server->getHeight(),frame);
   emit newServerFrame(serverFrame);
   av_free(frame);
   av_free(localFrame);
}

void MainWindow::on_pushButtonStartReceive_clicked(){
   QString name = ui->lineEditRemoteSource->text();
   remote = new Client(name,QString());
   int rw = remote->getWidth();
   int rh = remote->getHeight();
   PixelFormat rpf = remote->getPixelFormat();
   int w = ui->labelRemote->width();
   int h = ui->labelRemote->height();
   PixelFormat pf = PIX_FMT_RGB32;
   remoteToLocal = sws_getCachedContext(0,rw,rh,rpf,w,h,pf,SWS_BICUBIC,0,0,0);
   connect(remote,SIGNAL(newFrame(AVFrame*)),this,SLOT(onRemoteFrame(AVFrame*))
          ,Qt::QueuedConnection);
}

void MainWindow::on_pushButtonStartSend_clicked() {
    cout << "sending button clicked\n";
   QString dri = ui->comboBoxLocalFormat->currentText();
   QString name = ui->lineEditLocalPath->text();

   camera = new Client(name,dri);
   QString uri = "udp://localhost:";
   QString port = ui->lineEditLocalPort->text();
   uri.append(port);
   QString fmt = ui->lineEditServerFormat->text();
   QString codecName = ui->comboBoxServerVideoCodec->currentText();
   CodecID codec = CODEC_ID_NONE;
   if(codecName == "h264") codec = CODEC_ID_H264;
   else if(codecName == "mpg2") codec = CODEC_ID_MPEG2VIDEO;
   else if(codecName == "mpg4") codec = CODEC_ID_MPEG4;
   else if(codecName == "mjpeg") codec = CODEC_ID_JPEG2000;
   int wi = ui->lineEditVideoWidth->text().toInt();
   int he = ui->lineEditVideoHeight->text().toInt();
   int bitrate = 400000;
   int fps = 25;
   int gop_size = 12;
   server = new Server(uri,fmt,codec,wi,he,bitrate,fps,gop_size);
   cout << "After server construction\n";

   int cw = camera->getWidth();
   int ch = camera->getHeight();
   PixelFormat cpf = camera->getPixelFormat();
   cout << "After getting camera props\n";
   int sw = server->getWidth();
   int sh = server->getHeight();
   PixelFormat spf = server->getPixelFormat();
   int w = ui->labelWebcam->width();
   int h = ui->labelWebcam->height();
   PixelFormat pf = PIX_FMT_RGB32;
   cout << "Before sws\n";
   cameraToLocal = sws_getCachedContext(0,cw,ch,cpf,w,h,pf,SWS_BICUBIC,0,0,0);
   connect(camera,SIGNAL(newFrame(AVFrame*)),this,SLOT(onCameraFrame(AVFrame*))
          ,Qt::QueuedConnection);
   cameraToServer = sws_getCachedContext(0,cw,ch,cpf
                                        ,sw,sh,spf,SWS_BICUBIC,0,0,0);
   connect(this,SIGNAL(newServerFrame(AVFrame*)),server,SLOT(onFrame(AVFrame*))
          ,Qt::QueuedConnection);
   cout << "At the end actually\n";

}
