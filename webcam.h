#ifndef WEBCAM_H
#define WEBCAM_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

struct Webcam{
    VideoCapture* cap;
    int width;
    int height;
};

Webcam* init();
uchar* getData(Webcam*);

#endif // WEBCAM_H
