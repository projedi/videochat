#include "callscreen.h"
#include "ui_callscreen.h"
#include <iostream>
using namespace std;

//TODO: Add microphone into the play
CallScreen::CallScreen( QString contactName, QString remoteURI, QString localURI 
                      , QAbstractSocket* socket, QWidget *parent): QDialog(parent)
                                                                 , ui(new Ui::CallScreen) {
   this->socket = socket;
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   ui->setupUi(this);
   connect(ui->buttonEndCall,SIGNAL(clicked()),SLOT(rejectCall()));
   setWindowTitle("Conversation with " + contactName);
   QtConcurrent::run(this,&CallScreen::init,localURI,remoteURI);
}

void CallScreen::init(QString localURI, QString remoteURI) {
   VideoHardware cameras;
   AudioHardware microphones;
   ui->comboCamera->addItems(cameras.getNames());
   ui->comboMicrophone->addItems(microphones.getNames());
   int camIndex = ui->comboCamera->currentIndex();
   int micIndex = ui->comboMicrophone->currentIndex();

   Input::Stream *cameraStream = 0;
   Output::Stream *serverVideoStream = 0;
   Input::Stream *remoteVideoStream = 0;
   Output::Stream *playerLocalVideoStream = 0;
   Output::Stream *playerRemoteVideoStream = 0;
   try {
      camera = new InputGeneric( cameras.getFormats()[camIndex]
                               , cameras.getFiles()[camIndex]);
      camera->setState(Input::Playing);
      if(camera->getStreams().count() < 1) logger("Camera doesn't have streams");
      else cameraStream  = camera->getStreams()[0];
   } catch(...) { logger("Can't open camera"); camera = 0; }

   microphone = 0;

   server = new OutputGeneric("mpegts", remoteURI);
   if(cameraStream) {
      serverVideoStream = server->addStream(cameraStream->info());
      cameraStream->subscribe(serverVideoStream);
   }

   try {
      remote = new InputGeneric("mpegts", localURI);
      remote->setState(Input::Playing);
      if(remote->getStreams().count() < 1) logger("Remote doesn't have streams");
      else remoteVideoStream = remote->getStreams()[0];
   } catch(...) { logger("Can't open remote"); remote = 0; }

   if(cameraStream) {
      playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
      cameraStream->subscribe(playerLocalVideoStream);
   }
   if(remoteVideoStream) {
      playerRemoteVideoStream = ui->playerRemote->addStream(remoteVideoStream->info());
      remoteVideoStream->subscribe(playerRemoteVideoStream);
   }
}

CallScreen::~CallScreen() {
   //camera->setState(Input::Paused);
   //remote->setState(Input::Paused);
   logger("Deleting remote");
   delete remote;
   logger("Deleting camera");
   delete camera;
   logger("Deleting ui on the call screen");
   delete ui;
   logger("Deleting server");
   delete server;
   logger("Deleting microphone if exists");
   if(microphone) delete microphone;
}

void CallScreen::rejectCall() {
   socket->disconnectFromHost();
   close();
}

void CallScreen::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) rejectCall();
}
