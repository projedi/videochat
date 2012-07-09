#pragma once

#include <QDialog>
#include <QtConcurrentRun>
#include <QFuture>
#include "ffmpeg.h"

namespace Ui { class CallScreen; }

class CallScreen : public QDialog {
Q_OBJECT
public:
   explicit CallScreen( QString contactName, QString contactURI, QString localURI
                      , QWidget *parent = 0);
   ~CallScreen();
private slots:
   void on_buttonEndCall_clicked();
protected:
   void showEvent(QShowEvent*);
private:
   QFuture<void> setupFuture;
   void setupConnection(QString remoteURI, QString localURI);
   Ui::CallScreen *ui;
   Input *remote, *camera, *microphone;
   Output *server;
};
