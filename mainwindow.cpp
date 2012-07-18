#include "logging.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "callrequest.h"
#include "callresponse.h"
#include "callscreen.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QFileDialog>

#if defined(LINUX)
#define LOCALHOST "192.168.0.102"
#elif defined(WIN32)
#define LOCALHOST "192.168.0.101"
#endif
#define USERNAME "username"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   setupXmpp();
   ui->setupUi(this);
   ui->contactList->setCurrentRow(0);
   connect(ui->buttonExit, SIGNAL(clicked()),SLOT(close()));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::connected() {
   ui->buttonCall->setEnabled(true);
   ui->buttonSendFile->setEnabled(true);
   connect(ui->buttonCall, SIGNAL(clicked()), SLOT(startCall()));
   connect(ui->buttonSendFile, SIGNAL(clicked()), SLOT(sendFile()));
}

void MainWindow::startCall() {
   /*
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   CallRequest req(contactName);
   QAbstractSocket* socket = req.getSocket();
   if(req.exec() == (int)QDialog::Accepted) {
      CallScreen cs(req.getRemoteURI(),req.getRemoteURI(),req.getLocalURI(),socket,this);
      cs.exec();
   }
   delete socket;
   */
}

void MainWindow::sendFile() {
   if(ui->contactList->selectedItems().count() < 1) return;
   QString contactName = ui->contactList->selectedItems()[0]->text();
   QString filename = QFileDialog::getOpenFileName(this,"Open file","", "Any file (*.*)");
   if(filename.isNull()) return;
   QXmppTransferJob* job = transferManager.sendFile(contactName,filename);
   connect( job, SIGNAL(error(QXmppTransferJob::Error))
          , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
   connect( job, SIGNAL(progress(qint64,qint64))
          , this, SLOT(fileTransferProgress(qint64,qint64)));
}

void MainWindow::fileTransferRequest(QXmppTransferJob* job) {
   QMessageBox msgBox;
   msgBox.setText("Requesting file transfer from user " + job->jid() + ". Accept?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::Yes);
   int ret = msgBox.exec();
   QString filename;
   switch(ret) {
      case QMessageBox::Yes:
         filename = QFileDialog::getSaveFileName(this,"Save file","~"
                                                        ,"Any file (*.*)");
         connect( job, SIGNAL(error(QXmppTransferJob::Error))
                , this, SLOT(fileTransferError(QXmppTransferJob::Error)));
         connect( job, SIGNAL(progress(qint64,qint64))
                , this, SLOT(fileTransferProgress(qint64,qint64)));
         job->accept(filename);
         break;
      case QMessageBox::No:
         job->abort();
         break;
   }
}

void MainWindow::fileTransferStarted(QXmppTransferJob* job) { }
void MainWindow::fileTransferFinished(QXmppTransferJob* job) { }
void MainWindow::fileTransferProgress(qint64 done, qint64 total) { }
void MainWindow::fileTransferError(QXmppTransferJob::Error error) { }

void MainWindow::setupXmpp() {
   QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);
   server.setDomain(LOCALHOST);
   server.setPasswordChecker(new MyPasswordChecker());
   server.listenForClients(QHostAddress(LOCALHOST));
   server.listenForServers();
   transferManager.setSupportedMethods(QXmppTransferJob::SocksMethod);
   connect( &transferManager, SIGNAL(fileReceived(QXmppTransferJob*))
          , this, SLOT(fileTransferRequest(QXmppTransferJob*)));
   connect( &transferManager, SIGNAL(jobStarted(QXmppTransferJob*))
          , this, SLOT(fileTransferStarted(QXmppTransferJob*)));
   connect( &transferManager, SIGNAL(jobFinished(QXmppTransferJob*))
          , this, SLOT(fileTransferFinished(QXmppTransferJob*)));
   connect( &client, SIGNAL(connected()), this, SLOT(connected()));
   QXmppConfiguration config;
   config.setUser(USERNAME);
   //Uses actual IP address
   config.setHost(LOCALHOST);
   config.setDomain(LOCALHOST);
   config.setPassword("password");
   client.addExtension(&transferManager);
   client.connectToServer(config);
}
