#include "callrequest.h"
#include "ui_callrequest.h"
#include <QHostAddress>
#include <iostream>
using namespace std;

CallRequest::CallRequest(QString contactName, QWidget *parent): QDialog(parent)
                                                              , ui(new Ui::CallRequest) {
   cout << "Starting request" << endl;
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
   cout << "Deleting request" << endl;
   //socket->disconnectFromHost();
   //delete socket;
   delete ui;
}

void CallRequest::rejectCall() {
   cout << "Rejecting call" << endl;
   socket->disconnectFromHost();
   reject();
}

void CallRequest::discuss() {
   char buffer[51];
   if(state == 0) {
      state = 1;
      socket->write("VIDEOCHAT",9);
      socket->flush();
      cout << "wrote conversation starter" << endl;
      return;
   } else if(state == 1) {
      state = 2;
      int len = socket->read(buffer,50);
      if(len < 0) rejectCall();
      buffer[len] = 0;
      cout << "read some data of length " << len << " and here it is: " << buffer << endl;
      QString reply(buffer);
      if(reply.startsWith("ACCEPT ")) {
         reply.remove(0,7);
         cout << "extracting port: " << reply.toAscii().data() << endl;
         socket->write("8081",4);
         socket->flush();
         cout << "wrote my port: 8081" << endl;
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
