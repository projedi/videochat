#include "conversation.h"

Conversation::Conversation(QString jid) {
   this->jid = jid;
   call = 0;
   remoteCamera = 0;
}

Conversation::~Conversation() { }

void Conversation::start() {

}

void Conversation::stop() {

}

void Conversation::startCall() {
/*
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
   */
}

void Conversation::stopCall() {
   /*
   if(remoteCamera) {
      delete remoteCamera;
      remoteCamera = 0;
      ui->buttonHangup->hide();
      ui->buttonCall->show();
   } else {
      call->hangup();
   }
   */
}

void Conversation::sendFile(QString filename) {
   /*
   QString contactName = ui->contactList->selectedItems()[0]->text();
   QXmppTransferJob* job = transferManager.sendFile(contactName,filename);
   connect( job, SIGNAL(error(QXmppTransferJob::Error))
          , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
   connect( job, SIGNAL(progress(qint64,qint64))
          , this, SLOT(fileTransferProgress(qint64,qint64)));
          */
}

void Conversation::sendMessage(QString text) {
   /*
   QString contactName = ui->contactList->selectedItems()[0]->text();
   ui->textEditChat->append(text);
   ui->textEditChat->setAlignment(Qt::AlignLeft);
   //TODO: Don't send message to self
   client.sendMessage(contactName, text);
   */
}

void Conversation::fileRequest(QXmppTransferJob* job, QString filename) {
         /*
         connect( job, SIGNAL(error(QXmppTransferJob::Error))
                , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
         connect( job, SIGNAL(progress(qint64,qint64))
                , this, SLOT(fileTransferProgress(qint64,qint64)));
         job->accept(filename);
         */
}

void Conversation::callRequest(QXmppCall* call) {
         /*
         this->call = call;
         connect( call, SIGNAL(connected()),this,SLOT(callConnected()));
         connect( call, SIGNAL(finished()), this,SLOT(callFinished()));
         connect( call, SIGNAL(audioModeChanged(QIODevice::OpenMode)), this
                , SLOT(callAudioModeChanged(QIODevice::OpenMode)));
         connect( call, SIGNAL(videoModeChanged(QIODevice::OpenMode)), this
                , SLOT(callVideoModeChanged(QIODevice::OpenMode)));
         call->accept();
         */
}

bool Conversation::assignedTo(QString jid) {

}

/*
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


void MainWindow::callConnected() {
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   ui->comboBoxCodecs->setEnabled(false);
   ui->buttonCall->hide();
   ui->buttonHangup->show();
   ui->buttonCall->setEnabled(false);
   if(call->direction() == QXmppCall::OutgoingDirection) call->startVideo();
}

void MainWindow::callFinished() {
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   if(call->direction() == QXmppCall::OutgoingDirection) call->stopVideo();
   call = 0;
   ui->player->reset();
   ui->comboBoxCodecs->setEnabled(true);
   ui->buttonCall->setEnabled(true);
   ui->buttonHangup->hide();
   ui->buttonCall->show();
}

//TODO: Implement
void MainWindow::callAudioModeChanged(QIODevice::OpenMode) { }

void MainWindow::callVideoModeChanged(QIODevice::OpenMode mode) {
   //QXmppCall* call = static_cast<QXmppCall*>(sender());
   if(mode & QIODevice::ReadOnly) {
      qDebug() << "Opening device";
      serverVideoStream = new RtpOutputStream(call);
      InputStream *cameraStream = 0;
      int camIndex = ui->comboCamera->currentIndex();
      camera = new InputGeneric( cameras->getFiles()[camIndex]
                               , cameras->getFormats()[camIndex]);
      if(camera->getStreams().count() < 1) {
         qWarning("Camera has no streams");
         //TODO: Or maybe just stop sending video?
         call->hangup();
         return;
      }
      camera->setState(Input::Playing);
      cameraStream  = camera->getStreams()[0];
      cameraStream->subscribe(serverVideoStream);
      QXmppVideoFormat videoFormat;
      videoFormat.setFrameRate(30);
      videoFormat.setFrameSize(QSize(640,480));
      videoFormat.setPixelFormat(PIX_FMT_YUV420P);
      call->videoChannel()->setEncoderFormat(videoFormat);
      StreamInfo info;
      info.type = Video;
      info.video.width = 640;
      info.video.height = 480;
      info.video.fps = 30;
      info.video.pixelFormat = PIX_FMT_YUV420P;
      if(playerVideoStream) ui->player->removeStream(playerVideoStream);
      playerVideoStream = ui->player->addStream(info);
      if(!timer.isActive()) {
         connect(&timer, SIGNAL(timeout()), this, SLOT(readFrames()));
         timer.start();
      }
   } else if(mode == QIODevice::NotOpen) {
      qDebug() << "Closing device";
      delete camera;
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

*/
