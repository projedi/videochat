#include "callresponse.h"
#include "ui_callresponse.h"
#include <QHostAddress>
#include <iostream>
using namespace std;

CallResponse::CallResponse( QAbstractSocket* socket, QWidget *parent)
                          : QDialog(parent), ui(new Ui::CallResponse) {
   this->socket = socket;
   ui->setupUi(this);
   ui->labelContact->setText("Call request from " + socket->peerAddress().toString());
   setWindowTitle(ui->labelContact->text());
   connect(ui->buttonDecline,SIGNAL(clicked()),SLOT(reject()));
   connect(ui->buttonAccept, SIGNAL(clicked()),SLOT(acceptCall()));
   connect(socket,SIGNAL(disconnected()),SLOT(reject()));
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   connect(socket,SIGNAL(readyRead()),SLOT(discuss()));
   state = 0;
   //discuss();
}

CallResponse::~CallResponse() {
   delete socket;
   delete ui;
}

void CallResponse::acceptCall() {
   if(state != 1) return;
   cout << "Wrote acceptance" << endl;
   socket->write("ACCEPT 8080",11);
   state = 2;
}

void CallResponse::discuss() {
   char buffer[51];
   if(state == 0) {
      int len = socket->read(buffer,50);
      if(len < 0) reject();
      buffer[len] = 0;
      cout << "read data " << len << " long: " << buffer << endl;
      QString init(buffer);
      if(!init.startsWith("VIDEOCHAT")) throw -1;
      state = 1;
   } else if(state == 2) {
      int len = socket->read(buffer,50);
      if(len < 0) reject();
      buffer[len] = 0;
      cout << "read data " << len << " long: " << buffer << endl;
      QString localPort = "8080";
      QString remotePort(buffer);
      localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
      remoteURI = "udp://" + socket->peerAddress().toString() + ":" + remotePort;
      accept();
   }
}

void CallResponse::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) socket->disconnectFromHost();
}
