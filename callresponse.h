#pragma once

#include <QDialog>
#include <QTcpSocket>
#include <QFuture>
#include <QtConcurrentRun>

namespace Ui { class CallResponse; }

class CallResponse: public QDialog {
Q_OBJECT
public:
   explicit CallResponse(QAbstractSocket* socket, QWidget *parent = 0);
   ~CallResponse();
   QString getLocalURI();
   QString getRemoteURI();
private slots:
   void on_buttonAccept_clicked();
   void on_buttonDecline_clicked();
protected:
   void showEvent(QShowEvent*);
private:
   void checkValidity();
   QFuture<void> validityFuture;
   QAbstractSocket* socket;
   Ui::CallResponse *ui;
   QString localURI;
   QString remoteURI;
};
