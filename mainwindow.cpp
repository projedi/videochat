#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    QTimer::connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(ui->webcamTimerSpinBox->value());
    webcam = init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    cout << "Wouza!" << endl;
    QImage img(getData(webcam),webcam->width,webcam->height,QImage::Format_RGB888);
    //img.scaled(ui->label->width(), ui->label->height());
    ui->labelWebcam->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    timer->stop();
    timer->start(arg1);
}
