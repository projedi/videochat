#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "callrequest.h"
#include "callresponse.h"
#include "callscreen.h"
#include <QtConcurrentRun>

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   ui->setupUi(this);
   ui->contactList->setCurrentRow(0);
   connect(ui->buttonExit, SIGNAL(clicked()),SLOT(close()));
   connect(ui->buttonCall, SIGNAL(clicked()), SLOT(startCall()));
   connect(&server, SIGNAL(newConnection()), SLOT(handleCall()));
   server.listen(QHostAddress::Any,8000);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::startCall() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   CallRequest req(contactName);
   QAbstractSocket* socket = req.getSocket();
   if(req.exec() == (int)QDialog::Accepted) {
      cout << "Starting the show" << endl;
      CallScreen cs(req.getRemoteURI(),req.getRemoteURI(),req.getLocalURI(),socket,this);
      cs.exec();
   }
   delete socket;
}

void MainWindow::handleCall() {
   cout << "Someone called me" << endl;
   QAbstractSocket* socket = server.nextPendingConnection();
   try {
      CallResponse resp(socket,this);
      if(resp.exec() == (int)QDialog::Accepted) {
         cout << "Starting the show" << endl;
         CallScreen cs(resp.getRemoteURI(),resp.getRemoteURI()
                      ,resp.getLocalURI(),socket,this);
         cs.exec();
      }
   } catch(...) { cout << "Bogus call" << endl; }
   delete socket;
}
