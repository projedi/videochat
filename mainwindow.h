#pragma once

#include <QMainWindow>
#include <QTcpServer>

namespace Ui { class MainWindow; }

class MainWindow: public QMainWindow {
   Q_OBJECT
public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();
private slots:
   void startCall();
   void handleCall();
private:
   Ui::MainWindow *ui;
   QTcpServer server;
};
