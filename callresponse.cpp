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
   connect(ui->buttonAccept, SIGNAL(clicked()),SLOT(discuss()));
   connect(socket,SIGNAL(disconnected()),SLOT(reject()));
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   // Validity check
   char buffer[51];
   if(!socket->waitForReadyRead()) throw -1;
   int len = socket->read(buffer,50);
   buffer[len] = 0;
   cout << "read data " << len << " long: " << buffer << endl;
   QString init(buffer);
   if(!init.startsWith("VIDEOCHAT")) throw -1;
}

CallResponse::~CallResponse() {
   delete socket;
   delete ui;
}

void CallResponse::discuss() {
   //validityFuture.waitForFinished();
   char buffer[51];
   QString localPort = "8080";
   cout << "Wrote acceptance" << endl;
   socket->write("ACCEPT 8080",11);
   if(!socket->waitForReadyRead()) reject();
   int len = socket->read(buffer,50);
   buffer[len] = 0;
   cout << "read data " << len << " long: " << buffer << endl;
   QString remotePort(buffer);
   localURI = "udp://" + socket->localAddress().toString() + ":" + localPort;
   remoteURI = "udp://" + socket->peerAddress().toString() + ":" + remotePort;
   accept();
}

void CallResponse::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) socket->disconnectFromHost();
}
