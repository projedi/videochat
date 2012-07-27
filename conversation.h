#pragma once

#include <QTimer>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <qxmpp/QXmppCallManager.h>
#include "ffmpeg.h"

namespace Ui { class MainWindow; }

class Conversation: public QObject {
   Q_OBJECT
public:
   Conversation(QString jid, Ui::MainWindow *ui);
   ~Conversation();
   bool assignedTo(QString jid);
signals:
   void canSendFile(bool);
   void canCall(bool);
   void canText(bool);
public slots:
   void start();
   void stop();
   void startCall();
   void stopCall();
   void sendFile(QString filename);
   void sendMessage(QString text);
   void callRequest(QXmppCall* call);
   void fileRequest(QXmppTransferJob* job, QString filename);
   void announceCaps();
private:
   QXmppCall* call;
   Input* remoteCamera;
   QTimer timer;
   QString jid;

   QPushButton* buttonCall;
   QPushButton* buttonHangup;
   QPushButton* buttonSendFile;
   QProgressBar* progressBarStatus;
   QLabel* labelStatus;
   QComboBox* comboBoxCodecs;
   QLineEdit* lineEditChat;
   QTextEdit* textEditChat;
};
