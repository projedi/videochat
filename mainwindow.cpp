#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ffmpeg = new FFmpeg();
    connect(ffmpeg,SIGNAL(newRawCameraFrame(uint8_t*,int,int))
           ,this,SLOT(updateFrame(uint8_t*,int,int)));
    connect(ffmpeg,SIGNAL(newRawRemoteFrame(uint8_t*,int,int))
           ,this,SLOT(updateRemoteFrame(uint8_t*,int,int)));
    /*
    webcam = new Webcam("/dev/video0","v4l2",CODEC_ID_H264);
    connect(webcam,SIGNAL(frameArrived(QImage*)),this,SLOT(updateFrame(QImage*)));
    connect(webcam,SIGNAL(remoteFrameArrived(QImage*))
           ,this,SLOT(updateRemoteFrame(QImage*)));
    webcam->start();
    */
}

MainWindow::~MainWindow()
{
    delete ui;
    //delete webcam;
    delete ffmpeg;
}

void MainWindow::updateFrame(uint8_t* data, int width, int height){
   QImage* frame = new QImage(data,width,height,QImage::Format_RGB32);
   ui->labelWebcam->setPixmap(QPixmap::fromImage(*frame));
   delete frame;
}

void MainWindow::updateRemoteFrame(uint8_t* data, int width, int height){
   QImage* frame = new QImage(data,width,height,QImage::Format_RGB32);
   ui->labelRemote->setPixmap(QPixmap::fromImage(*frame));
   delete frame;
}
