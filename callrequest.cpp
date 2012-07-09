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
   connect(ui->buttonAbort,SIGNAL(clicked()),SLOT(reject()));
   socket = new QTcpSocket();
   connect(socket,SIGNAL(connected()),SLOT(discuss()));
   connect(socket,SIGNAL(disconnected()),SLOT(reject()));
   connect(socket,SIGNAL(readyRead()),SLOT(discuss()));
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   state = 0;
   socket->connectToHost(QHostAddress(contactName), 8000);
}

CallRequest::~CallRequest() {
   cout << "Deleting request" << endl;
   //socket->disconnectFromHost();
   delete socket;
   delete ui;
}

void CallRequest::discuss() {
   char buffer[51];
   if(state == 0) {
      socket->write("VIDEOCHAT",9);
      cout << "wrote conversation starter" << endl;
      state = 1;
      return;
   } else if(state == 1) {
      int len = socket->read(buffer,50);
      if(len < 0) { reject(); return; }
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
      } else reject();
      state = 2;
      return;
   }
}

void CallRequest::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) socket->disconnectFromHost();
}
