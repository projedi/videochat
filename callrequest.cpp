#include "callrequest.h"
#include "ui_callrequest.h"
#include <QHostAddress>
#include <iostream>
using namespace std;

CallRequest::CallRequest(QString contactName, QWidget *parent): QDialog(parent)
                                                              , ui(new Ui::CallRequest) {
   ui->setupUi(this);
   ui->labelContact->setText(contactName);
   connect(ui->buttonAbort,SIGNAL(clicked()),SLOT(reject()));
   socket = new QTcpSocket();
   connect(socket,SIGNAL(connected()),SLOT(discuss()));
   socket->connectToHost(QHostAddress(contactName), 8000);
}

CallRequest::~CallRequest() {
   socket->close();
   delete socket;
   delete ui;
}

QString CallRequest::getLocalURI() { return localURI; }
QString CallRequest::getRemoteURI() { return remoteURI; }

void CallRequest::discuss() {
   QtConcurrent::run(this, &CallRequest::discussWorker);
}

void CallRequest::discussWorker() {
   char buffer[50];
   socket->write("VIDEOCHAT",9);
   cout << "wrote conversation starter" << endl;
   if(!socket->waitForReadyRead()) reject();
   cout << "read some data" << endl;
   int len = socket->read(buffer,50);
   buffer[len] = 0;
   QString reply(buffer);
   cout << "this is the data I read: " << buffer << endl;
   if(reply.startsWith("ACCEPT")) {
      reply.remove(0,7);
      cout << "extracting port: " << reply.toAscii().data() << endl;
      cout << "wrote my port" << endl;
      socket->write("8081",4);
      socket->flush();
      QString localPort = "8081";
      remoteURI = "udp://" + socket->peerAddress().toString() + ":" + reply;
      localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
      socket->disconnectFromHost();
      accept();  
   } else reject();
}
