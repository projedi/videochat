#include "callscreen.h"
#include "ui_callscreen.h"
#include <iostream>
using namespace std;

//TODO: Add microphone into the play
CallScreen::CallScreen( QString contactName, QString remoteURI, QString localURI , QAbstractSocket* socket, QWidget *parent): QDialog(parent)
                                                                 , ui(new Ui::CallScreen) {
   this->socket = socket;
   connect(socket,SIGNAL(error(QAbstractSocket::SocketError))
          ,SLOT(onSocketError(QAbstractSocket::SocketError)));
   ui->setupUi(this);
   connect(ui->buttonEndCall,SIGNAL(clicked()),SLOT(rejectCall()));
   setWindowTitle("Conversation with " + contactName);
   VideoHardware cameras;
   AudioHardware microphones;
   ui->comboCamera->addItems(cameras.getNames());
   ui->comboMicrophone->addItems(microphones.getNames());
   int camIndex = ui->comboCamera->currentIndex();
   int micIndex = ui->comboMicrophone->currentIndex();

   camera = new InputGeneric( cameras.getFormats()[camIndex]
                            , cameras.getFiles()[camIndex]);
   camera->setState(Input::Playing);
   Input::Stream *cameraStream = camera->getStreams()[0];

   microphone = 0;

   server = new OutputGeneric("mpegts", remoteURI);
   Output::Stream *serverVideoStream = server->addStream(cameraStream->info());
   cameraStream->subscribe(serverVideoStream);

   remote = new InputGeneric("mpegts", localURI);
   remote->setState(Input::Playing);
   Input::Stream *remoteVideoStream = remote->getStreams()[0];

   Output::Stream *playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
   Output::Stream *playerRemoteVideoStream = ui->playerRemote->addStream(remoteVideoStream->info());
   cameraStream->subscribe(playerLocalVideoStream);
   remoteVideoStream->subscribe(playerRemoteVideoStream);
}

CallScreen::~CallScreen() {
   camera->setState(Input::Paused);
   remote->setState(Input::Paused);
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
