#include "callresponse.h"
#include "ui_callresponse.h"
#include <QHostAddress>
#include <iostream>
using namespace std;

CallResponse::CallResponse( QAbstractSocket* socket, QWidget *parent)
                          : QDialog(parent), ui(new Ui::CallResponse) {
   ui->setupUi(this);
   ui->labelContact->setText(socket->peerName());
   this->socket = socket;
   validityFuture = QtConcurrent::run(this, &CallResponse::checkValidity);
}

CallResponse::~CallResponse() {
   delete socket;
   delete ui;
}

QString CallResponse::getLocalURI() { return localURI; }
QString CallResponse::getRemoteURI() { return remoteURI; }

void CallResponse::on_buttonAccept_clicked() {
   validityFuture.waitForFinished();
   char buffer[51];
   int len;
   QString localPort = "8080";
   cout << "Wrote acceptance" << endl;
   socket->write("ACCEPT 8080",11);
   if(!socket->waitForReadyRead()) reject();
   len = socket->read(buffer,50);
   buffer[len] = 0;
   cout << "read data " << len << " long: " << buffer << endl;
   QString remotePort(buffer);
   localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
   remoteURI = "udp://" + socket->peerAddress().toString() + ":" + remotePort;
   accept();
}

void CallResponse::on_buttonDecline_clicked() {
   validityFuture.waitForFinished();
   socket->write("DECLINE",7);
   reject();
}

void CallResponse::showEvent(QShowEvent*) {
   if(!socket) { socket->close(); reject(); }
}

void CallResponse::checkValidity() {
   char buffer[100];
   int len;
   if(!socket->waitForReadyRead()) return;
   len = socket->read(buffer,99);
   cout << "it is " << len << " long" << endl;
   buffer[len] = 0;
   QString init(buffer);
   cout << "This is what i read: " << buffer << endl;
   if(init != "VIDEOCHAT") {
      socket->close();
      delete socket;
   }
}
