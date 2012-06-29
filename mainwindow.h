#pragma once

#include <QMainWindow>
#include "ffmpeg.h"

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
   Q_OBJECT
public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();   
private slots:
   void on_buttonCall_clicked();
   void on_buttonSendFile_clicked();
private:
   Ui::MainWindow *ui;
   ALSAV4L2* local;
   Server* server;
   Client* client;
};
