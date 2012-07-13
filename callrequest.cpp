#include "logging.h"
#include "callrequest.h"
#include "ui_callrequest.h"
#include <QHostAddress>
#include <iostream>
using namespace std;

CallRequest::CallRequest(QString contactName, QWidget *parent): QDialog(parent)
                                                              , ui(new Ui::CallRequest) {
   ui->setupUi(this);
   ui->labelContact->setText("Contacting " + contactName);
   setWindowTitle(ui->labelContact->text());
   connect(ui->buttonAbort,SIGNAL(clicked()),SLOT(rejectCall()));
   socket = new QTcpSocket();
   connect(socket,SIGNAL(connected()),SLOT(discuss()));
   //connect(socket,SIGNAL(disconnected()),SLOT(reject()));
   connect(socket,SIGNAL(readyRead()),SLOT(discuss()));
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   state = 0;
   socket->connectToHost(QHostAddress(contactName), 8000);
}

CallRequest::~CallRequest() {
   //socket->disconnectFromHost();
   //delete socket;
   delete ui;
}

void CallRequest::rejectCall() {
   socket->disconnectFromHost();
   reject();
}

void CallRequest::discuss() {
   char buffer[51];
   if(state == 0) {
      state = 1;
      socket->write("VIDEOCHAT",9);
      socket->flush();
      return;
   } else if(state == 1) {
      state = 2;
      int len = socket->read(buffer,50);
      if(len < 0) rejectCall();
      buffer[len] = 0;
      QString reply(buffer);
      if(reply.startsWith("ACCEPT ")) {
         reply.remove(0,7);
         socket->write("8081",4);
         socket->flush();
         QString localPort = "8081";
         remoteURI = "udp://" + socket->peerAddress().toString() + ":" + reply;
         localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
         accept();  
         return;
      } else rejectCall();
      return;
   }
}

void CallRequest::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) rejectCall();
}
