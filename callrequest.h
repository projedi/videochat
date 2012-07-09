#pragma once

#include <QDialog>
#include <QTcpSocket>
#include <QtConcurrentRun>

namespace Ui { class CallRequest; }

class CallRequest: public QDialog {
Q_OBJECT
public:
   explicit CallRequest(QString contactName, QWidget *parent = 0);
   ~CallRequest();
   QString getLocalURI() const { return localURI; }
   QString getRemoteURI() const { return remoteURI; }
   QAbstractSocket* getSocket() { return socket; }
private slots:
   void rejectCall();
   void discuss();
   void onSocketError(QAbstractSocket::SocketError);
private:
   QAbstractSocket* socket;
   Ui::CallRequest *ui;
   QString localURI;
   QString remoteURI;
   int state;
};
