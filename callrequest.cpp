#include "callrequest.h"
#include "ui_callrequest.h"

CallRequest::CallRequest(QString contactName, QWidget *parent): QDialog(parent)
                                                              , ui(new Ui::CallRequest) {
   ui->setupUi(this);
   ui->labelContact->setText(contactName);
   connect(ui->buttonAbort,SIGNAL(clicked()),SLOT(reject()));
   socket = QTcpSocket();
   connect(socket,SIGNAL(connected()),SLOT(discuss()));
   socket.connectToHost(QHostAddress(contactName), 8000);
}

CallRequest::~CallRequest() {
   socket.disconnectFromHost();
   delete ui;
}

QString getLocalURI() { return localURI; }
QString getRemoteURI() { return remoteURI; }

void CallRequest::discuss() {
   char buffer[50];
   socket.writeData("VIDEOCHAT",9);
   if(!socket.waitForReadyRead()) reject();
   socket.read(buffer,50);
   QString reply(buffer);
   delete buffer;
   if(reply.startsWith("ACCEPT")) {
      reply.remove(0,7);
      socket.writeData("8081",4);
      QString localPort = "8081";
      remoteURI = "udp://" + socket.peerAddress().toString() + ":" + reply;
      localURI = "udp://" + socket.localAddress().toString() + ":" + localPort;
      accept();  
   } else reject();
}
