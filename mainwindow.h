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
#include "ffmpeg.h"

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
   void connected();
   void disconnected();
   void startCall();
   void sendFile();
   void fileTransferRequest(QXmppTransferJob*);
   void fileTransferStarted(QXmppTransferJob*);
   void fileTransferFinished(QXmppTransferJob*);
   void fileTransferProgress(qint64,qint64);
   void fileTransferError(QXmppTransferJob::Error);
   void callReceived(QXmppCall*);
   void callConnected();
   void callFinished();
   void callAudioModeChanged(QIODevice::OpenMode);
   void callVideoModeChanged(QIODevice::OpenMode);
   void readFrames();
private:
   void setupXmpp();
   void updateHardware();
   void setupCamera(int camIndex);
   void setupMicrophone(int micIndex);
   Ui::MainWindow *ui;
   QXmppClient client;
   QXmppServer server;
   QXmppTransferManager transferManager;
   QXmppCallManager callManager;
   Input *camera, *microphone;
   OutputStream *serverVideoStream, *playerVideoStream;
   VideoHardware* cameras;
   AudioHardware* microphones;
   QTimer timer;
   bool doExit;
   QXmppCall* call;
};
