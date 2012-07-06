#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   ui->setupUi(this);
   camera = new VideoHardware();
   QList<QString> devices = camera->getDevices();
   ui->comboBoxVideoDevs->addItems(devices);
}

MainWindow::~MainWindow() {
    delete ui;
    delete camera;
    delete client;
    delete server;
}

void MainWindow::on_buttonCall_clicked() {
   camera->setState(Input::Playing);
   QList<Input::Stream*> cameraStreams = camera->getStreams();
   Input::Stream* curStream = cameraStreams[ui->comboBoxVideoDevs->currentIndex()];

   server = new OutputGeneric("mpegts",ui->lineEditLocal->text());
   Output::Stream* serverStream = server->addStream(curStream->info());
   curStream->subscribe(serverStream);

   client = new InputGeneric("mpegts",ui->lineEditRemote->text());
   Input::Stream* clientStream = client->getStreams()[0];
   client->setState(Input::Playing);

   Output::Stream* playerStream = ui->playerLocal->addStream(curStream->info());
   curStream->subscribe(playerStream);

   Output::Stream* playerRemoteStream = ui->playerRemote->addStream(clientStream->info());
   clientStream->subscribe(playerRemoteStream);

}

void MainWindow::on_buttonSendFile_clicked() {

}
