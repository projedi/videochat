#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include "ffwebcam.h"
#include <list>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();   
    
private slots:
    void on_spinBox_valueChanged(int arg1);
    void updateFrame(QImage frame);

private:
    Ui::MainWindow *ui;
    Webcam* webcam;
};

#endif // MAINWINDOW_H
