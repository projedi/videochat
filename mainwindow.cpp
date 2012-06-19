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
   cout << "Call button clicked" << endl;
   local = new ALSAV4L2("/dev/video0", "hw:0"); 
   cout << "Local created" << endl;
   localPlayer = new Player(ui->labelWebcam->width(),ui->labelWebcam->height());
   cout << "Local player created" << endl;
   localToPlayer.ffConnect(local,localPlayer);
   cout << "local and localPlayer connected" << endl;
   this->connect(localPlayer,SIGNAL(onNewFrame(QPixmap)),SLOT(onLocalFrame(QPixmap)));
   cout << "MainWindow connected to player" << endl;
}

void MainWindow::on_buttonSendFile_clicked() {

}
