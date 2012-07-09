#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "callrequest.h"
#include "callresponse.h"
#include "callscreen.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   ui->setupUi(this);
   connect(&server,SIGNAL(newConnection()),SLOT(handleCall()));
   server.listen(QHostAddress::Any,8000);
   ui->contactList->setCurrentRow(0);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_buttonExit_clicked() { close(); }

void MainWindow::on_buttonCall_clicked() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   CallRequest req(contactName,this);
   if(req.exec() == (int)QDialog::Accepted) {
      cout << "Starting the show" << endl;
      CallScreen call(req.getRemoteURI(),req.getRemoteURI(),req.getLocalURI(),this);
      call.exec();
   }
}

void MainWindow::handleCall() {
   cout << "Someone called me" << endl;
   QAbstractSocket* socket = server.nextPendingConnection();
   CallResponse resp(socket,this);
   if(resp.exec() == (int)QDialog::Accepted) {
      cout << "Starting the show";
      CallScreen call(resp.getRemoteURI(),resp.getRemoteURI(),resp.getLocalURI(),this);
      call.exec();
   }
}
