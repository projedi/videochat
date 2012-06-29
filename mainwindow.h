#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include "ffmpeg.h"
#include <list>

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
   Q_OBJECT
public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();   
//signals:
//   void newServerFrame(AVFrame* frame);
private slots:
   void onLocalFrame(QPixmap frame);
   //void on_pushButtonStartReceive_clicked();
   //void on_pushButtonStartSend_clicked();
   void on_buttonCall_clicked();
   void on_buttonSendFile_clicked();

private:
   Ui::MainWindow *ui;
   ALSAV4L2* local;
   Server* server;
   //Client* client;
   Player* localPlayer;
   //Player* clientPlayer;
   FFConnector localToServer;
   FFConnector localToPlayer;
   //FFConnector clientToPlayer;
};

#endif // MAINWINDOW_H
