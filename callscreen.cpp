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
   cameras = 0;
   microphones = 0;

   updateHardware();
   int camIndex = ui->comboCamera->currentIndex();
   int micIndex = ui->comboMicrophone->currentIndex();

   serverVideoStream = 0;
   playerLocalVideoStream = 0;
   playerRemoteVideoStream = 0;
   camera = 0;
   microphone = 0;
   remote = 0;
   server = 0;
   server = new OutputGeneric("mjpeg", remoteURI);
   QtConcurrent::run(this,&CallScreen::setupCamera,camIndex);
   QtConcurrent::run(this,&CallScreen::setupMicrophone,micIndex);
   QtConcurrent::run(this,&CallScreen::setupRemote,localURI);
}

void CallScreen::updateHardware() {
   if(cameras) delete cameras;
   cameras = new VideoHardware();
   if(microphones) delete microphones;
   microphones = new AudioHardware();
   ui->comboCamera->addItems(cameras->getNames());
   ui->comboMicrophone->addItems(microphones->getNames());
}

void CallScreen::setupCamera(int camIndex) {
   if(serverVideoStream) {
      server->removeStream(serverVideoStream);
      serverVideoStream = 0;
   }
   if(playerLocalVideoStream) {
      server->removeStream(playerLocalVideoStream);
      playerLocalVideoStream = 0;
   }
   if(camera) delete camera;
   InputStream *cameraStream = 0;
   try {
      camera = new InputGeneric( cameras->getFiles()[camIndex]
                               , cameras->getFormats()[camIndex]);
      camera->setState(Input::Playing);
      if(camera->getStreams().count() < 1) logger("Camera doesn't have streams");
      else cameraStream  = camera->getStreams()[0];
   } catch(...) { logger("Can't open camera"); camera = 0; }
   if(server && cameraStream) {
      serverVideoStream = server->addStream(cameraStream->info());
      cameraStream->subscribe(serverVideoStream);
   }
   if(ui->playerLocal && cameraStream) {
      playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
      cameraStream->subscribe(playerLocalVideoStream);
   }
}

void CallScreen::setupMicrophone(int micIndex) {
   if(microphone) delete microphone;
}

void CallScreen::setupRemote(QString localURI) {
   if(playerRemoteVideoStream) {
      server->removeStream(playerRemoteVideoStream);
      playerRemoteVideoStream = 0;
   }
   if(remote) delete remote;
   InputStream *remoteVideoStream = 0;
   try {
      remote = new InputGeneric(localURI, "mjpeg");
      logger("Constructed remote");
      remote->setState(Input::Playing);
      logger("Set state to play");
      QList<InputStream*> streams = remote->getStreams();
      if(streams.count() < 1) logger("Remote doesn't have streams");
      else {
         for(int i = 0; i < streams.count(); i++) {
            if(streams[i]->info().type == Video) {
               remoteVideoStream = streams[i];
               break;
            }
         }
         if(!remoteVideoStream) logger("Remote doesn't have video stream");
      }
   } catch(...) { logger("Can't open remote"); remote = 0; }
   if(ui->playerRemote && remoteVideoStream) {
      playerRemoteVideoStream = ui->playerRemote->addStream(remoteVideoStream->info());
      remoteVideoStream->subscribe(playerRemoteVideoStream);
   }
}

CallScreen::~CallScreen() {
   //camera->setState(Input::Paused);
   //remote->setState(Input::Paused);
   logger("Deleting remote");
   if(remote) delete remote;
   logger("Deleting camera");
   if(camera) delete camera;
   logger("Deleting ui on the call screen");
   delete ui;
   logger("Deleting server");
   if(server) delete server;
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
