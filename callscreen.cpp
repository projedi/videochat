#include "callscreen.h"
#include "ui_callscreen.h"

//TODO: Add microphone into the play
CallScreen::CallScreen( QString contactName, QString contactURI, QString localURI
                      , QWidget *parent): QDialog(parent), ui(new Ui::CallScreen) {
   ui->setupUi(this);
   this->setWindowTitle("Conversation with " + contactName);
   VideoHardware cameras;
   AudioHardware microphones;
   ui->comboCamera->addItems(cameras.getNames());
   ui->comboMicrophone->addItems(microphones.getNames());
   int camIndex = ui->comboCamera->currentIndex();
   int micIndex = ui->comboMicrophone->currentIndex();
   camera = new InputGeneric( cameras.getFormats()[camIndex]
                            , cameras.getFiles()[camIndex]);
   remote = new InputGeneric("mpegts", localURI);
   server = new OutputGeneric("mpegts", contactURI);

   Input::Stream *cameraStream, *remoteVideoStream;
   Output::Stream *serverVideoStream, *playerLocalVideoStream, *playerRemoteVideoStream;
   cameraStream = camera->getStreams()[0];
   remoteVideoStream = remote->getStreams()[0];
   serverVideoStream = server->addStream(cameraStream->info());
   playerLocalVideoStream = ui->playerLocal->addStream(cameraStream->info());
   playerRemoteVideoStream = ui->playerRemote->addStream(remoteVideoStream->info());
   cameraStream->subscribe(serverVideoStream);
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

void CallScreen::on_buttonEndCall_clicked() {
    this->close();
}
