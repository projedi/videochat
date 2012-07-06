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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_buttonCall_clicked() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contact = ui->contactList->selectedItems()[0]->text();
   CallRequest req(contact,this);
   int result = req.exec();
   if(result == (int)QDialog::Accepted) {
      CallScreen call(contact,req.getRemoteURI(),req.getLocalURI(),this);
      call.exec();
   }
}

void MainWindow::on_buttonExit_clicked() {
    this->close();
}

void MainWindow::handleCall() {
   QAbstractSocket* socket = server.nextPendingConnection();
   char buffer[10];
   int len;
   len = socket->read(buffer,9);
   buffer[len] = 0;
   QString init(buffer);
   if(init != "VIDEOCHAT") {
      socket->close();
      return;
   }
   CallResponse resp(socket,this);
   int result = resp.exec();
   if(result == (int)QDialog::Accepted) {
      CallScreen call(resp.getRemoteURI(),resp.getRemoteURI(),resp.getLocalURI(),this);
      call.exec();
   }
}
