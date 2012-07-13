#include "logging.h"
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
   connect(ui->buttonDecline,SIGNAL(clicked()),SLOT(rejectCall()));
   connect(ui->buttonAccept, SIGNAL(clicked()),SLOT(acceptCall()));
   //connect(socket,SIGNAL(disconnected()),SLOT(rejectCall()));
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   connect(socket,SIGNAL(readyRead()),SLOT(discuss()));
   state = 0;
}

CallResponse::~CallResponse() {
   //delete socket;
   delete ui;
} 

void CallResponse::acceptCall() {
   if(state != 1) return;
   state = 2;
   socket->write("ACCEPT 8080",11);
   socket->flush();
}

void CallResponse::rejectCall() {
   socket->disconnectFromHost();
   reject();
}

void CallResponse::discuss() {
   char buffer[51];
   if(state == 0) {
      state = 1;
      int len = socket->read(buffer,50);
      if(len < 0) rejectCall();
      buffer[len] = 0;
      QString init(buffer);
      if(!init.startsWith("VIDEOCHAT")) rejectCall();
      return;
   } else if(state == 2) {
      state = 3;
      int len = socket->read(buffer,50);
      if(len < 0) rejectCall();
      buffer[len] = 0;
      QString localPort = "8080";
      QString remotePort(buffer);
      localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
      remoteURI = "udp://" + socket->peerAddress().toString() + ":" + remotePort;
      accept();
      return;
   }
}

void CallResponse::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) rejectCall();
}
