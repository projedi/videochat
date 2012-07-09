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
   //setupFuture = QtConcurrent::run(this,&CallScreen::setupConnection,contactURI,localURI);
   VideoHardware cameras;
   AudioHardware microphones;
   ui->comboCamera->addItems(cameras.getNames());
   ui->comboMicrophone->addItems(microphones.getNames());
   cout << "Added default cameras/microphones" << endl;
   int camIndex = ui->comboCamera->currentIndex();
   int micIndex = ui->comboMicrophone->currentIndex();

   cout << "Setting up camera" << endl;
   camera = new InputGeneric( cameras.getFormats()[camIndex]
                            , cameras.getFiles()[camIndex]);
   camera->setState(Input::Playing);
   Input::Stream *cameraStream = camera->getStreams()[0];

   cout << "Setting up server to " << remoteURI.toAscii().data() << endl;
   server = new OutputGeneric("mpegts", remoteURI);
   Output::Stream *serverVideoStream = server->addStream(cameraStream->info());
   cameraStream->subscribe(serverVideoStream);

   cout << "Setting up remote to " << localURI.toAscii().data() << endl;
   remote = new InputGeneric("mpegts", localURI);
   remote->setState(Input::Playing);
   Input::Stream *remoteVideoStream = remote->getStreams()[0];

   Output::Stream *playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
   Output::Stream *playerRemoteVideoStream = ui->playerRemote->addStream(remoteVideoStream->info());
   cameraStream->subscribe(playerLocalVideoStream);
   remoteVideoStream->subscribe(playerRemoteVideoStream);
}

CallScreen::~CallScreen() {
    delete ui;
    delete remote;
    delete camera;
    delete microphone;
    delete server;
}

void CallScreen::rejectCall() {
   cout << "Ending call" << endl;
   socket->disconnectFromHost();
   close();
}

void CallScreen::onSocketError(QAbstractSocket::SocketError err) {
   if(err == QAbstractSocket::RemoteHostClosedError) rejectCall();
}
