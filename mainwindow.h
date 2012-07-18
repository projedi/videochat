#pragma once

#include <QMainWindow>
//#include <QTcpServer>
#include <qxmpp/QXmppServer.h>
#include <qxmpp/QXmppPasswordChecker.h>
#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppTransferManager.h>

namespace Ui { class MainWindow; }

class MyPasswordChecker: public QXmppPasswordChecker {
public:
   MyPasswordChecker() { }
   ~MyPasswordChecker() { }
   bool hasGetPassword() const { return true; }
protected:
   QXmppPasswordReply::Error getPassword(const QXmppPasswordRequest&,QString& pass) {
      pass = "password";
      return QXmppPasswordReply::NoError;
   }
};

class MainWindow: public QMainWindow {
   Q_OBJECT
public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();
private slots:
   void connected();
   void startCall();
   void sendFile();
   void fileTransferRequest(QXmppTransferJob*);
   void fileTransferStarted(QXmppTransferJob*);
   void fileTransferFinished(QXmppTransferJob*);
   void fileTransferProgress(qint64,qint64);
   void fileTransferError(QXmppTransferJob::Error);
private:
   void setupXmpp();
   Ui::MainWindow *ui;
   QXmppClient client;
   QXmppServer server;
   QXmppTransferManager transferManager;
};
