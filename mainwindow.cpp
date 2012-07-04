#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
    delete camera;
    delete client;
    delete server;
}

void MainWindow::on_buttonCall_clicked() {
   camera = new VideoHardware();
}

void MainWindow::on_buttonSendFile_clicked() {

}
