#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    //TODO: Wrong place I think
    qRegisterMetaType<QAVFrame>("QAVFrame");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onLocalFrame(QPixmap frame){
   ui->labelWebcam->setPixmap(frame);
}

void MainWindow::on_buttonCall_clicked() {
   local = new ALSAV4L2("/dev/video0", "hw:0"); 
   server = new Server("udp://localhost:8080");
   localToServer.ffConnect(local,server);
   localPlayer = new Player(ui->labelWebcam->width(),ui->labelWebcam->height());
   localToPlayer.ffConnect(local,localPlayer);
   this->connect(localPlayer,SIGNAL(onNewFrame(QPixmap)),SLOT(onLocalFrame(QPixmap)));
}

void MainWindow::on_buttonSendFile_clicked() {

}
