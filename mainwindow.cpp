#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    webcam = new Webcam(defaultParams());
    connect(webcam,SIGNAL(frameArrived(QImage*)),this,SLOT(updateFrame(QImage*)));
    webcam->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFrame(QImage* frame){
    ui->labelWebcam->setPixmap(QPixmap::fromImage(*frame));
    delete frame;
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
}
