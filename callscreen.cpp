#include "callscreen.h"
#include "ui_callscreen.h"

//TODO: Add microphone into the play
CallScreen::CallScreen( QString contactName, QString contactURI, QString localURI
                      , QWidget *parent): QDialog(parent), ui(new Ui::CallScreen) {
   ui->setupUi(this);
   cout << "Setting up call with " << contactName << endl;
   this->setWindowTitle("Conversation with " + contactName);
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
   cout << "Setting up server to " << contactURI.toAscii().data() << endl;
   server = new OutputGeneric("mpegts", contactURI);
   cout << "Setting up remote to " << localURI.toAscii().data() << endl;
   remote = new InputGeneric("mpegts", localURI);

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
