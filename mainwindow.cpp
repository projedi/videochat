#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    webcam = new Webcam(defaultParams());
    connect(webcam,SIGNAL(frameArrived(QImage)),this,SLOT(updateFrame(QImage)));
    webcam->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFrame(QImage frame){
    cout << "Wouza!" << endl;
    //AVFormatContext* pFormatContext = avformat_alloc_context();
    //avformat_open_input(&pFormat,"/dev/video/0",av_find_input_format("v4l2"),0);
    //uchar* data = getData(webcam);
    //if(hist.size() == 10){
    //    convert(hist);
    //    hist.clear();
    //}
    //hist.push_back(data);
    //QImage img(data,webcam->width,webcam->height,QImage::Format_RGB888);
    //img.scaled(ui->label->width(), ui->label->height());
    ui->labelWebcam->setPixmap(QPixmap::fromImage(frame));
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    //timerDraw->stop();
    //timerDraw->start(arg1);
}
