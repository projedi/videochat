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

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_buttonCall_clicked() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contact = ui->contactList->selectedItems()[0]->text();
   CallRequest req(contact,this);
   int result = req.exec();
   if(result == (int)QDialog::Accepted) {
      cout << "Starting the show" << endl;
      CallScreen call(contact,req.getRemoteURI(),req.getLocalURI(),this);
      call.exec();
   }
}

void MainWindow::on_buttonExit_clicked() {
    this->close();
}

void MainWindow::handleCall() {
    cout << "Someone called me" << endl;
   QAbstractSocket* socket = server.nextPendingConnection();
   char buffer[100];
   int len;
   if(!socket->waitForReadyRead()) return;
   len = socket->read(buffer,99);
   cout << "it is " << len << " long" << endl;
   buffer[len] = 0;
   QString init(buffer);
   cout << "This is what i read: " << buffer << endl;
   if(init != "VIDEOCHAT") {
      socket->close();
      return;
   }
   CallResponse resp(socket,this);
   int result = resp.exec();
   if(result == (int)QDialog::Accepted) {
       cout << "Starting the show";
      CallScreen call(resp.getRemoteURI(),resp.getRemoteURI(),resp.getLocalURI(),this);
      call.exec();
   }
}
