#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timerDraw = new QTimer(this);
    QTimer::connect(timerDraw,SIGNAL(timeout()),this,SLOT(update()));
    timerDraw->start(ui->webcamTimerSpinBox->value());
    webcam = init();
    ffinit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    cout << "Wouza!" << endl;
    uchar* data = getData(webcam);
    if(hist.size() == 10){
        convert(hist);
        hist.clear();
    }
    hist.push_back(data);
    QImage img(data,webcam->width,webcam->height,QImage::Format_RGB888);
    //img.scaled(ui->label->width(), ui->label->height());
    ui->labelWebcam->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    timer->stop();
    timer->start(arg1);
}
