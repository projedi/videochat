#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
    delete local;
    delete server;
    delete client;
}

void MainWindow::on_buttonCall_clicked() {
   local = new ALSAV4L2("/dev/video0", "hw:0"); 
   server = new Server("udp://localhost:8080");
   client = new Client("udp://localhost:8081");
   server.setSource(local);
   ui->playerLocal.setSource(local);
   ui->playerClient.setSource(client);
}

void MainWindow::on_buttonSendFile_clicked() {

}
