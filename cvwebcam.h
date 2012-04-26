#ifndef CVWEBCAM_H
#define CVWEBCAM_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class Webcam {
public:
    Webcam(int deviceID);
    uchar* getData();
    int width;
    int height;
private:
    VideoCapture* cap;
};


#endif // CVWEBCAM_H
