#pragma once

#include <QDialog>
#include <QTcpSocket>
#include <QFuture>
#include <QtConcurrentRun>

namespace Ui { class CallResponse; }

class CallResponse: public QDialog {
Q_OBJECT
public:
   explicit CallResponse(QAbstractSocket* socket, QWidget *parent = 0);
   ~CallResponse();
   QString getLocalURI() const { return localURI; }
   QString getRemoteURI() const { return remoteURI; }
private slots:
   void acceptCall();
   void rejectCall();
   void discuss();
   void onSocketError(QAbstractSocket::SocketError);
private:
   int state;
   QAbstractSocket* socket;
   Ui::CallResponse *ui;
   QString localURI;
   QString remoteURI;
};
