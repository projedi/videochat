#include "logging.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QFileDialog>
#include "streaming.h"

#if defined(LINUX)
#define LOCALHOST "192.168.0.102"
#elif defined(WIN32)
#define LOCALHOST "192.168.0.101"
#endif
#define USERNAME "username"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   doExit = false;
   ui->setupUi(this);
   ui->contactList->setCurrentRow(0);
   connect(ui->buttonExit, SIGNAL(clicked()),SLOT(shutdown()));
   connect(ui->buttonCall, SIGNAL(clicked()), SLOT(startCall()));
   connect(ui->buttonSendFile, SIGNAL(clicked()), SLOT(sendFile()));
   ui->labelStatus->setText("Connecting");
   setupXmpp();
   cameras = 0;
   microphones = 0;
   updateHardware();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::shutdown() {
   if(call) call->hangup();
   doExit = true;
   client.disconnectFromServer();
}

void MainWindow::connected() {
   ui->buttonCall->setEnabled(true);
   ui->buttonSendFile->setEnabled(true);
   ui->labelStatus->setText("Connected");
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

   call = callManager.call(contactName);
   connect( call, SIGNAL(connected()),this,SLOT(callConnected()));
   connect( call, SIGNAL(finished()), this,SLOT(callFinished()));
   connect( call, SIGNAL(audioModeChanged(QIODevice::OpenMode)), this
          , SLOT(callAudioModeChanged(QIODevice::OpenMode)));
   connect( call, SIGNAL(videoModeChanged(QIODevice::OpenMode)), this
          , SLOT(callVideoModeChanged(QIODevice::OpenMode)));
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
   //QMessageBox msgBox;
   //msgBox.setText("File transfer finished");
   //msgBox.exec();
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
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   if(call->direction() == QXmppCall::OutgoingDirection) call->startVideo();
}

void MainWindow::callFinished() {
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   if(call->direction() == QXmppCall::OutgoingDirection) call->stopVideo();
}

//TODO: Implement
void MainWindow::callAudioModeChanged(QIODevice::OpenMode) { }

void MainWindow::callVideoModeChanged(QIODevice::OpenMode mode) {
   qDebug() << "Call video mode changed";
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   if(mode & QIODevice::ReadOnly) {
      qDebug() << "Opening device";
      serverVideoStream = new RtpOutputStream(call);
      camera = 0;
      setupCamera(ui->comboCamera->currentIndex());
      QXmppVideoFormat videoFormat;
      videoFormat.setFrameRate(30);
      videoFormat.setFrameSize(QSize(640,480));
      videoFormat.setPixelFormat(QXmppVideoFrame::Format_YUV420P);
      call->videoChannel()->setEncoderFormat(videoFormat);
      StreamInfo info;
      info.type = Video;
      info.video.width = 640;
      info.video.height = 480;
      info.video.fps = 30;
      info.video.pixelFormat = PIX_FMT_YUV420P;
      playerVideoStream = ui->player->addStream(info);
      if(!timer.isActive()) {
         connect(&timer, SIGNAL(timeout()), this, SLOT(readFrames()));
         timer.start();
      }
   } else if(mode == QIODevice::NotOpen) {
      qDebug() << "Closing device";
      //TODO: Close webcam
      delete camera;
      disconnect(&timer, SIGNAL(timeout()), this, SLOT(readFrames()));
      timer.stop();
   } else {
      qDebug() << "Got some oher opennmode" << (int)mode;
   }
}

void MainWindow::readFrames() {
   QXmppVideoFrame qframe;
   QList<QXmppVideoFrame> frames = call->videoChannel()->readFrames();
   if(frames.count() == 0) return;
   foreach(QXmppVideoFrame posFrame, frames) {
      if(posFrame.isValid()) qframe = posFrame;
   }
   AVFrame* frame = avcodec_alloc_frame();
   frame->linesize[0] = qframe.bytesPerLine();
   frame->data[0] = (uchar*) qframe.bits();
   frame->width = qframe.width();
   frame->height = qframe.height();
   frame->format = PIX_FMT_RGB24;
   playerVideoStream->process(frame);
   av_free(frame);
}

void MainWindow::setupXmpp() {
   //QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);
   server.setDomain(LOCALHOST);
   server.setPasswordChecker(new MyPasswordChecker());
   server.listenForClients(QHostAddress(LOCALHOST));
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
   QXmppConfiguration config;
   config.setUser(USERNAME);
   //Uses actual IP address
   config.setHost(LOCALHOST);
   config.setDomain(LOCALHOST);
   config.setPassword("password");
   client.addExtension(&transferManager);
   client.addExtension(&callManager);
   client.connectToServer(config);
}

void MainWindow::updateHardware() {
   if(cameras) delete cameras;
   cameras = new VideoHardware();
   if(microphones) delete microphones;
   microphones = new AudioHardware();
   ui->comboCamera->addItems(cameras->getNames());
   ui->comboMicrophone->addItems(microphones->getNames());
}

void MainWindow::setupCamera(int camIndex) {
   if(camera) delete camera;
   InputStream *cameraStream = 0;
   try {
      camera = new InputGeneric( cameras->getFiles()[camIndex]
                               , cameras->getFormats()[camIndex]);
      camera->setState(Input::Playing);
      if(camera->getStreams().count() < 1) logger("Camera doesn't have streams");
      else cameraStream  = camera->getStreams()[0];
   } catch(...) { logger("Can't open camera"); camera = 0; }
   if(cameraStream && serverVideoStream) {
      cameraStream->subscribe(serverVideoStream);
   }
}

void MainWindow::setupMicrophone(int micIndex) {
   if(microphone) delete microphone;
}
