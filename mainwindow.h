#pragma once

#include <QMainWindow>
#include <QTimer>
//#include <QTcpServer>
#include <qxmpp/QXmppServer.h>
#include <qxmpp/QXmppPasswordChecker.h>
#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppTransferManager.h>
#include <qxmpp/QXmppCallManager.h>
#include <qxmpp/QXmppRtpChannel.h>
#include "conversation.h"
#include <QListWidgetItem>

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
   void shutdown();
   void startCall();
   void stopCall();
   void sendFile();
   void sendMessage();
   void codecChanged(const QString&);
   void contactChanged(QListWidgetItem*, QListWidgetItem*);
   void setStabilizing(int);

   void connected();
   void disconnected();
   void messageReceived(const QXmppMessage&);
   void fileTransferRequest(QXmppTransferJob*);
   void callRequest(QXmppCall*);
private:
   void loadContactList();
   void updateHardware();
   void setupXmpp();
   Conversation* jidToConversation(QString jid);

   Ui::MainWindow *ui;
   QXmppClient client;
   QXmppServer server;
   QXmppTransferManager transferManager;
   QXmppCallManager callManager;
   Input *camera, *microphone;
   OutputStream *serverVideoStream, *playerVideoStream;
   VideoHardware* cameras;
   AudioHardware* microphones;
   QList<Conversation*> chats;
};
