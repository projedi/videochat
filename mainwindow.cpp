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
   QList<Input::Stream*> cameraStreams = camera->getStreams();
   Input::Stream* curStream = cameraStreams[ui->comboBoxVideoDevs->currentIndex()];
   //server = new OutputGeneric("mpegts","udp://localhost:8080");
   //Output::Stream* serverStream = server->addStream(curStream->info());
   //curStream->subscribe(serverStream);
   camera->setState(Input::Playing);
   /*client = new InputGeneric("mpegts","udp://localhost:8080");
   QList<Input::Stream*>::iterator i;
   QList<QString> remotes;
   int j = 0;
   for(i=client->getStreams().begin();i!=client->getStreams().end();i++)
      remotes.append(QString::number(j++));
   */
   Output::Stream* playerStream = ui->playerLocal->addStream(curStream->info());
   //cout << "Got player stream" << endl;
   curStream->subscribe(playerStream);
}

void MainWindow::on_buttonSendFile_clicked() {

}
