#include "logging.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QFileDialog>
#include "streaming.h"
#include <stabilization.h>

#if defined(LINUX)
#define LOCALHOST "192.168.0.102"
#elif defined(WIN32)
#define LOCALHOST "192.168.0.101"
#endif
#define USERNAME "username"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   cameras = 0;
   microphones = 0;
   playerVideoStream = 0;
   serverVideoStream = 0;
   camera = 0;
   microphone = 0;
   ui->setupUi(this);

   connect(ui->buttonExit, SIGNAL(clicked()),SLOT(shutdown()));
   connect(ui->buttonCall, SIGNAL(clicked()), SLOT(startCall()));
   connect(ui->buttonHangup, SIGNAL(clicked()), SLOT(stopCall()));
   connect(ui->buttonSendFile, SIGNAL(clicked()), SLOT(sendFile()));
   connect( ui->comboBoxCodecs, SIGNAL(currentIndexChanged(const QString&))
          , SLOT(codecChanged(const QString&)));
   connect(ui->lineEditChat, SIGNAL(returnPressed()), SLOT(sendMessage()));
   connect(ui->checkBoxStabilize, SIGNAL(stateChanged(int)), SLOT(setStabilizing(int)));
   connect( ui->contactList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*))
          , SLOT(contactChanged(QListWidgetItem*, QListWidgetItem*)));

   loadContactList();
   codecChanged(ui->comboBoxCodecs->currentText());
   updateHardware();
   setupXmpp();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::loadContactList() {
   //TODO: Load contact list from file
   for(int i = 0; i < ui->contactList->count(); i++) {
      Conversation* chat = new Conversation(ui->contactList->item(i)->text(), ui);
      chats << chat;
   }
   ui->contactList->setCurrentRow(0);
}

void MainWindow::updateHardware() {
   if(cameras) delete cameras;
   cameras = new VideoHardware();
   if(microphones) delete microphones;
   microphones = new AudioHardware();
   ui->comboCamera->addItems(cameras->getNames());
   ui->comboMicrophone->addItems(microphones->getNames());
}

void MainWindow::setupXmpp() {
   QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);
   server.setDomain(LOCALHOST);
   server.setPasswordChecker(new MyPasswordChecker());
   server.listenForClients(QHostAddress(LOCALHOST));
   server.listenForServers();
   connect(&client, SIGNAL(connected()), this, SLOT(connected()));
   connect(&client, SIGNAL(disconnected()), this, SLOT(disconnected()));
   connect( &client, SIGNAL(messageReceived(const QXmppMessage&))
          , SLOT(messageReceived(const QXmppMessage&)));
   connect( &transferManager, SIGNAL(fileReceived(QXmppTransferJob*))
          , this, SLOT(fileTransferRequest(QXmppTransferJob*)));
   connect( &callManager, SIGNAL(callReceived(QXmppCall*))
          , this, SLOT(callRequest(QXmppCall*)));
   QXmppConfiguration config;
   config.setUser(USERNAME);
   //Uses actual IP address
   config.setHost(LOCALHOST);
   config.setDomain(LOCALHOST);
   config.setPassword("password");
   client.addExtension(&transferManager);
   client.addExtension(&callManager);
   ui->labelStatus->setText("Connecting");
   client.connectToServer(config);
}

Conversation* MainWindow::jidToConversation(QString jid) {
   foreach(Conversation* chat, chats) if(chat->assignedTo(jid)) return chat;
}

void MainWindow::shutdown() {
   foreach(Conversation* chat, chats) delete chat;
   chats.clear();
   if(client.state() == QXmppClient::ConnectedState) {
      connect(&client, SIGNAL(disconnected()), SLOT(close()));
      client.disconnectFromServer();
   } else {
      close();
   }
}

void MainWindow::startCall() {
   if(ui->contactList->currentRow() < 0) return;
   chats[ui->contactList->currentRow()]->startCall();
}

void MainWindow::stopCall() {
   if(ui->contactList->currentRow() < 0) return;
   chats[ui->contactList->currentRow()]->stopCall();
   ui->player->reset();
   ui->comboBoxCodecs->setEnabled(true);
}

void MainWindow::sendFile() {
   if(ui->contactList->currentRow() < 0) return;
   QString filename = QFileDialog::getOpenFileName(this,"Open file","", "Any file (*.*)");
   if(filename.isNull()) return;
   ui->labelStatus->setText("Trying to send file");
   chats[ui->contactList->currentRow()]->sendFile(filename);
}

void MainWindow::sendMessage() {
   if(ui->contactList->currentRow() < 0) return;
   QString text = ui->lineEditChat->text();
   ui->lineEditChat->clear();
   chats[ui->contactList->currentRow()]->sendMessage(text);
}

void MainWindow::codecChanged(const QString &codecName) {
   QList<CodecID> codecs;
   if(codecName == "MJPEG")
      codecs << CODEC_ID_MJPEG;
   else if(codecName == "H.264")
      codecs << CODEC_ID_H264;
   else
      qWarning("Unknown codec requested: %s", codecName.toAscii().data());
   callManager.setCodecs(codecs);
}

void MainWindow::contactChanged(QListWidgetItem* curItem, QListWidgetItem* prevItem) {
   jidToConversation(prevItem->text())->stop();
   jidToConversation(curItem->text())->start();
   /*
   Conversation* chat = jidToConversation(contactName);
   ui->textEditChat->clear();
   foreach(Conversation::Message mes, chat->messages()) {
      if(mes.type == Conversation::RemoteMessage) {
         ui->textEditChat->append(mes.text);
         ui->textEditChat->setAlignment(Qt::AlignRight);
      } else {
         ui->textEditChat->append(mes.text);
         ui->textEditChat->setAlignment(Qt::AlignLeft);
      }
   }
   connect(chat, SIGNAL(canSendFile(bool)), SLOT(canSendFile(bool)));
   connect(chat, SIGNAL(canCall(bool)), SLOT(canCall(bool)));
   connect(chat, SIGNAL(canText(bool)), SLOT(canText(bool)));
   connect(chat, 
   if(chat->canSendFile()) { ui->buttonSendFile->setEnabled(false); }
   else { ui->buttonSendFile->setEnabled(false); }
   if(chat->canCall()) { ui->buttonCall->setEnabled(false); }
   else { ui->buttonCall->setEnabled(false); }
   if(chat->canText()) { ui->lineEditChat->setEnabled(false); }
   else { ui->lineEditChat->setEnabled(false); }
   */
}

//TODO: implement
void MainWindow::setStabilizing(int state) {

}

void MainWindow::connected() {
   ui->buttonCall->setEnabled(true);
   ui->buttonSendFile->setEnabled(true);
   ui->labelStatus->setText("Connected");
}

void MainWindow::disconnected() {
   ui->buttonCall->setEnabled(false);
   ui->buttonSendFile->setEnabled(false);
   ui->labelStatus->setText("Disconnected");
}

//TODO: find out who sent this and reroute to appropriate conversation
void MainWindow::messageReceived(const QXmppMessage& message) {
   qDebug("Message received");
   ui->textEditChat->append(message.body());
   ui->textEditChat->setAlignment(Qt::AlignRight);
}

void MainWindow::fileTransferRequest(QXmppTransferJob* job) {
   QMessageBox msgBox;
   msgBox.setText("Requesting file transfer from user " + job->jid() + ". Accept?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::Yes);
   int ret = msgBox.exec();
   QString filename;
   switch(ret) {
      case QMessageBox::Yes:
         filename = QFileDialog::getSaveFileName(this,"Save file",""
                                                        ,"Any file (*.*)");
         if(filename.isNull()) job->abort();
         ui->labelStatus->setText("Trying to receive file");
         jidToConversation(job->jid())->fileRequest(job, filename);
         break;
      case QMessageBox::No:
         job->abort();
         break;
   }
}

void MainWindow::callRequest(QXmppCall* call) {
   QMessageBox msgBox;
   msgBox.setText("Requesting call from user " + call->jid() + ". Accept?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::Yes);
   int ret = msgBox.exec();
   switch(ret) {
      case QMessageBox::Yes:
         jidToConversation(call->jid())->callRequest(call);
         break;
      case QMessageBox::No:
         call->hangup();
         break;
   }
}
