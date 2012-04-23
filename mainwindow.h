#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include "webcam.h"
#include "converter.h"
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

public slots:
    void update();
    
private slots:
    void on_spinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QTimer* timerDraw;
    struct Webcam* webcam;
    std::list<uchar*> hist;
};

#endif // MAINWINDOW_H
