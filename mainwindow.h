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
   void updateFrame(uint8_t* data, int width, int height);
   void updateRemoteFrame(uint8_t* data, int width, int height);

private:
    Ui::MainWindow *ui;
    //Webcam* webcam;
    FFmpeg* ffmpeg;
};

#endif // MAINWINDOW_H
