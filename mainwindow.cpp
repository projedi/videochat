#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QFileDialog>
#include "streaming.h"
#include <stabilization.h>
#include <QNetworkInterface>

#define USERNAME "username"

#include <iostream>
#include <fstream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   doExit = false;
   ui->setupUi(this);
   loadContacts();
   ui->contactList->setCurrentRow(0);
   connect(ui->buttonExit, SIGNAL(clicked()),SLOT(shutdown()));
   connect(ui->buttonCall, SIGNAL(clicked()), SLOT(startCall()));
   connect(ui->buttonHangup, SIGNAL(clicked()), SLOT(stopCall()));
   connect(ui->buttonSendFile, SIGNAL(clicked()), SLOT(sendFile()));
   connect(ui->comboBoxCodecs, SIGNAL(currentIndexChanged(const QString&))
          , SLOT(codecChanged(const QString&)));
   connect(ui->lineEditChat, SIGNAL(returnPressed()), SLOT(sendMessage()));
   connect( ui->checkBoxStabilize, SIGNAL(stateChanged(int))
          , ui->player, SLOT(setStabilizing(int)));
   connect(ui->comboCamera, SIGNAL(currentIndexChanged(int)), SLOT(cameraChanged(int)));
   codecChanged(ui->comboBoxCodecs->currentText());
   setupXmpp();
   camera = 0;
   microphone = 0;
   remoteCamera = 0;
   serverVideoStream = 0;
   playerVideoStream = 0;
   playerLocalVideoStream = 0;
   cameraStream = 0;
   cameras = 0;
   microphones = 0;
   updateHardware();
}

void MainWindow::cameraChanged(int camIndex) {
   if(camera) delete camera;
   camera = new InputGeneric( cameras->getFiles()[camIndex]
                            , cameras->getFormats()[camIndex]);
   if(camera->getStreams().count() < 1) {
      qWarning("Camera has no streams");
      cameraStream = 0;
   } else {
      camera->setState(Input::Playing);
      cameraStream = camera->getStreams()[0];
      if(!playerLocalVideoStream)
         playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
      cameraStream->subscribe(playerLocalVideoStream);
      if(serverVideoStream) cameraStream->subscribe(serverVideoStream);
   }
}

MainWindow::~MainWindow() {
   if(camera) { delete camera; camera = 0; }
   delete ui;
}

void MainWindow::loadContacts() {
   fstream contactsfile("contacts.txt",fstream::in);
   char contactName[100];
   while(!contactsfile.eof()) {
      contactsfile.getline(contactName,100);
      ui->contactList->addItem(QString::fromAscii(contactName));
   }
}

void MainWindow::codecChanged(const QString &codecName) {
   qDebug("Codec changed");
   QList<CodecID> codecs;
   if(codecName == "MJPEG")
      codecs << CODEC_ID_JPEG2000;
   else if(codecName == "H.264")
      codecs << CODEC_ID_H264;
   else
      qWarning("Unknown codec requested: %s", codecName.toAscii().data());
   callManager.setCodecs(codecs);
}

void MainWindow::shutdown() {
   if(call) call->hangup();
   if(remoteCamera) stopCall();
   if(client.state() == QXmppClient::ConnectedState) {
      doExit = true;
      client.disconnectFromServer();
   } else {
      close();
   }
}

void MainWindow::connected() {
   ui->buttonCall->setEnabled(true);
   ui->buttonSendFile->setEnabled(true);
   ui->labelStatus->setText("On " + server.domain());
}

void MainWindow::disconnected() {
   ui->buttonCall->setEnabled(false);
   ui->buttonSendFile->setEnabled(false);
   ui->labelStatus->setText("Connecting");
   if(doExit) close();
}

void MainWindow::startCall() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   if(contactName.startsWith("camera")) {
      contactName.replace("camera@","");
      remoteCamera = new InputGeneric("http://"+contactName+"/mjpg/video.mjpg","mjpeg");
      remoteCamera->setState(Input::Playing);
      StreamInfo info;
      info.type = Video;
      info.video.width = 640;
      info.video.height = 480;
      info.video.fps = 30;
      info.video.pixelFormat = PIX_FMT_YUV420P;
      if(playerVideoStream) ui->player->removeStream(playerVideoStream);
      playerVideoStream = ui->player->addStream(info);
      remoteCamera->getStreams()[0]->subscribe(playerVideoStream);
      ui->buttonCall->hide();
      ui->buttonHangup->show();
   } else { 
      call = callManager.call(contactName);
      connect( call, SIGNAL(connected()),this,SLOT(callConnected()));
      connect( call, SIGNAL(finished()), this,SLOT(callFinished()));
      connect( call, SIGNAL(audioModeChanged(QIODevice::OpenMode)), this
             , SLOT(callAudioModeChanged(QIODevice::OpenMode)));
      connect( call, SIGNAL(videoModeChanged(QIODevice::OpenMode)), this
             , SLOT(callVideoModeChanged(QIODevice::OpenMode)));
   }
}

void MainWindow::stopCall() {
   if(remoteCamera) {
      delete remoteCamera;
      remoteCamera = 0;
      ui->buttonHangup->hide();
      ui->buttonCall->show();
   } else {
      call->hangup();
   }
   ui->player->reset();
   ui->comboBoxCodecs->setEnabled(true);
   ui->lineEditBitrate->setEnabled(true);
   ui->spinBoxKeyFrame->setEnabled(true);
   ui->spinBoxFPS->setEnabled(true);
   ui->comboBoxSize->setEnabled(true);
}

void MainWindow::sendFile() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   QString filename = QFileDialog::getOpenFileName(this,"Open file","", "Any file (*.*)");
   if(filename.isNull()) return;
   ui->labelStatus->setText("Trying to send file");
   QXmppTransferJob* job = transferManager.sendFile(contactName,filename);
   connect( job, SIGNAL(error(QXmppTransferJob::Error))
          , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
   connect( job, SIGNAL(progress(qint64,qint64))
          , this, SLOT(fileTransferProgress(qint64,qint64)));
}

void MainWindow::sendMessage() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString text = ui->lineEditChat->text();
   ui->lineEditChat->clear();
   QString contactName = ui->contactList->selectedItems()[0]->text();
   ui->textEditChat->append(text);
   ui->textEditChat->setAlignment(Qt::AlignLeft);
   //TODO: Don't send message to self
   client.sendMessage(contactName, text);
}

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
         connect( job, SIGNAL(error(QXmppTransferJob::Error))
                , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
         connect( job, SIGNAL(progress(qint64,qint64))
                , this, SLOT(fileTransferProgress(qint64,qint64)));
         job->accept(filename);
         break;
      case QMessageBox::No:
         job->abort();
         break;
   }
}

void MainWindow::fileTransferStarted(QXmppTransferJob* job) {
   ui->labelStatus->hide();
   ui->progressBarStatus->show();
   ui->progressBarStatus->setMinimum(0);
   ui->progressBarStatus->setMaximum(job->fileInfo().size());
   ui->progressBarStatus->setValue(0);
}

void MainWindow::fileTransferFinished(QXmppTransferJob* job) {
   ui->progressBarStatus->hide();
   ui->labelStatus->show();
   ui->labelStatus->setText("Connected");
}

void MainWindow::fileTransferProgress(qint64 done, qint64 total) {
   ui->progressBarStatus->setValue(done);
}

void MainWindow::fileTransferError(QXmppTransferJob::Error error) { }

void MainWindow::callReceived(QXmppCall* call) {
   QMessageBox msgBox;
   msgBox.setText("Requesting call from user " + call->jid() + ". Accept?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::Yes);
   int ret = msgBox.exec();
   switch(ret) {
      case QMessageBox::Yes:
         this->call = call;
         connect( call, SIGNAL(connected()),this,SLOT(callConnected()));
         connect( call, SIGNAL(finished()), this,SLOT(callFinished()));
         connect( call, SIGNAL(audioModeChanged(QIODevice::OpenMode)), this
                , SLOT(callAudioModeChanged(QIODevice::OpenMode)));
         connect( call, SIGNAL(videoModeChanged(QIODevice::OpenMode)), this
                , SLOT(callVideoModeChanged(QIODevice::OpenMode)));
         call->accept();
         break;
      case QMessageBox::No:
         call->hangup();
         break;
   }
}

void MainWindow::callConnected() {
   ui->comboBoxCodecs->setEnabled(false);
   ui->lineEditBitrate->setEnabled(false);
   ui->spinBoxKeyFrame->setEnabled(false);
   ui->spinBoxFPS->setEnabled(false);
   ui->comboBoxSize->setEnabled(false);
   ui->buttonCall->hide();
   ui->buttonHangup->show();
   ui->buttonCall->setEnabled(false);
   ui->contactList->setEnabled(false);
   if(call->direction() == QXmppCall::OutgoingDirection) call->startVideo();
}

void MainWindow::callFinished() {
   if(call->direction() == QXmppCall::OutgoingDirection) call->stopVideo();
   call = 0;
   ui->player->reset();
   ui->comboBoxCodecs->setEnabled(true);
   ui->lineEditBitrate->setEnabled(true);
   ui->spinBoxKeyFrame->setEnabled(true);
   ui->spinBoxFPS->setEnabled(true);
   ui->comboBoxSize->setEnabled(true);
   ui->buttonCall->setEnabled(true);
   ui->buttonHangup->hide();
   ui->buttonCall->show();
   ui->contactList->setEnabled(true);
}

//TODO: Implement
void MainWindow::callAudioModeChanged(QIODevice::OpenMode) { }

void MainWindow::callVideoModeChanged(QIODevice::OpenMode mode) {
   if(mode & QIODevice::ReadOnly) {
      qDebug() << "Opening device";
      if(serverVideoStream) delete serverVideoStream;
      serverVideoStream = new RtpOutputStream(call);
      if(!cameraStream) call->hangup();
      cameraStream->subscribe(serverVideoStream);
      QXmppVideoFormat videoFormat;
      videoFormat.setFrameRate(ui->spinBoxFPS->value());
      if(ui->comboBoxSize->currentText() == "640x480")
         videoFormat.setFrameSize(QSize(640,480));
      else if(ui->comboBoxSize->currentText() == "320x240")
         videoFormat.setFrameSize(QSize(320,240));
      videoFormat.setPixelFormat(PIX_FMT_YUV420P);
      videoFormat.setGopSize(ui->spinBoxKeyFrame->value());
      videoFormat.setBitrate(ui->lineEditBitrate->text().toInt());
      call->videoChannel()->setEncoderFormat(videoFormat);
      StreamInfo info;
      info.type = Video;
      QXmppVideoFormat incomingFormat = call->videoChannel()->decoderFormat();
      info.video.width = incomingFormat.frameWidth();
      info.video.height = incomingFormat.frameHeight();
      info.video.fps = incomingFormat.frameRate();
      info.video.pixelFormat = incomingFormat.pixelFormat();
      if(playerVideoStream) ui->player->removeStream(playerVideoStream);
      playerVideoStream = ui->player->addStream(info);
      if(!timer.isActive()) {
         connect(&timer, SIGNAL(timeout()), this, SLOT(readFrames()));
         timer.start();
      }
   } else if(mode == QIODevice::NotOpen) {
      qDebug() << "Closing device";
      if(serverVideoStream) {
         if(cameraStream) cameraStream->unsubscribe(serverVideoStream);
         delete serverVideoStream;
         serverVideoStream = 0;
      }
      disconnect(&timer, SIGNAL(timeout()), this, SLOT(readFrames()));
      timer.stop();
   } else {
      qDebug() << "Got some oher opennmode" << (int)mode;
   }
}

void MainWindow::readFrames() {
   AVFrame* frame = 0;
   QList<AVFrame*> frames = call->videoChannel()->readFrames();
   if(frames.count() == 0) return;
   foreach(AVFrame* posFrame, frames) {
      if(frame) av_free(frame);
      frame = posFrame;
   }
   playerVideoStream->process(frame);
   av_free(frame);
}

void MainWindow::setupXmpp() {
   /*
   QString localhost;
   QList<QNetworkInterface> lsnet = QNetworkInterface::allInterfaces();
   foreach(QNetworkInterface inet, lsnet) {
      if(!(inet.flags() & QNetworkInterface::IsLoopBack) && (inet.flags() & QNetworkInterface::IsRunning)) {
         if(inet.addressEntries().count() < 1) continue;
         qDebug("There are %d addresses to interface", inet.addressEntries().count());
         foreach(QNetworkAddressEntry addr, inet.addressEntries()) {
            qDebug() << addr.ip();
            if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol)
               localhost = addr.ip().toString();
         }
      }
   }
   */
   fstream configfile("config.txt",fstream::in);
   char localhostStr[100];
   configfile.getline(localhostStr,100);
   QString localhost = QString::fromAscii(localhostStr);
   //QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);
   server.setDomain(localhost);
   server.setPasswordChecker(new MyPasswordChecker());
   server.listenForClients(QHostAddress::LocalHost);
   server.listenForServers();
   transferManager.setSupportedMethods(QXmppTransferJob::SocksMethod);
   connect( &transferManager, SIGNAL(fileReceived(QXmppTransferJob*))
          , this, SLOT(fileTransferRequest(QXmppTransferJob*)));
   connect( &transferManager, SIGNAL(jobStarted(QXmppTransferJob*))
          , this, SLOT(fileTransferStarted(QXmppTransferJob*)));
   connect( &transferManager, SIGNAL(jobFinished(QXmppTransferJob*))
          , this, SLOT(fileTransferFinished(QXmppTransferJob*)));
   connect( &callManager, SIGNAL(callReceived(QXmppCall*))
          , this, SLOT(callReceived(QXmppCall*)));
   connect( &client, SIGNAL(connected()), this, SLOT(connected()));
   connect( &client, SIGNAL(disconnected()), this, SLOT(disconnected()));
   connect( &client, SIGNAL(messageReceived(const QXmppMessage&))
          , SLOT(messageReceived(const QXmppMessage&)));
   QXmppConfiguration config;
   config.setUser(USERNAME);
   //Uses actual IP address
   config.setHost("127.0.0.1");
   config.setDomain(localhost);
   config.setPassword("password");
   client.addExtension(&transferManager);
   client.addExtension(&callManager);
   client.connectToServer(config);
   ui->labelStatus->setText("Connecting");
   call = 0;
}

void MainWindow::updateHardware() {
   if(cameras) delete cameras;
   cameras = new VideoHardware();
   if(microphones) delete microphones;
   microphones = new AudioHardware();
   ui->comboCamera->addItems(cameras->getNames());
   ui->comboMicrophone->addItems(microphones->getNames());
}
