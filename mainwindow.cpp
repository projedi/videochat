#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ffmpeg = new FFmpeg();
    FFInput* camera = ffmpeg->addInput("/dev/video0", "v4l2");
    FFOutput* server = ffmpeg->addOutput("udp://localhost:8080", "m4v", "h263");
    ffmpeg->connectIO(camera,server);
    connect(ffmpeg,SIGNAL(cameraFrame(uint8_t*,int,int))
           ,this,SLOT(updateFrame(uint8_t*,int,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFrame(uint8_t* frame, int width, int height){
    ui->labelWebcam->setPixmap(QPixmap::fromImage(*frame));
    delete frame;
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
}
