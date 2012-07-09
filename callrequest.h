#pragma once

#include <QDialog>
#include <QTcpSocket>
#include <QtConcurrentRun>

namespace Ui { class CallRequest; }

class CallRequest : public QDialog {
Q_OBJECT
public:
   explicit CallRequest(QString contactName, QWidget *parent = 0);
   ~CallRequest();
   QString getLocalURI();
   QString getRemoteURI();
private slots:
   void discuss();
private:
   void discussWorker();
   QAbstractSocket* socket;
   QString localURI;
   QString remoteURI;
   Ui::CallRequest *ui;
};
