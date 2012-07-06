#include "callresponse.h"
#include "ui_callresponse.h"

CallResponse::CallResponse( QAbstractSocket* socket, QWidget *parent)
                          : QDialog(parent), ui(new Ui::CallResponse) {
   ui->setupUi(this);
   ui->labelContact->setText(socket.peerName());
   this->socket = socket;
}

CallResponse::~CallResponse() {
   delete socket;
   delete ui;
}

QString getLocalURI() { return localURI; }
QString getRemoteURI() { return remoteURI; }

void CallResponse::on_buttonAccept_clicked() {
   char buffer[51];
   int len;
   QString localPort = "8080";
   socket->writeData("ACCEPT 8080",11);
   if(!socket->waitForReadyRead()) reject();
   len = socket->read(buffer,50);
   buffer[len] = 0;
   QString remotePort(buffer);
   localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
   remoteURI = "udp://" + socket->peerAddress().toString() + ":" + remotePort;
   accept();
}

void CallResponse::on_buttonDecline_clicked() {
   socket->writeData("DECLINE",7);
   reject();
}
