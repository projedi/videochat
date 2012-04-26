#include "cvwebcam.h"

Webcam::Webcam(int deviceID){
    VideoCapture* cap = new VideoCapture(deviceID);
    if(!cap->isOpened()) throw "Can't open camera";
    this->cap = cap;
    this->width = cap->get(CV_CAP_PROP_FRAME_WIDTH);
    this->height = cap->get(CV_CAP_PROP_FRAME_HEIGHT);
}

uchar* Webcam::getData() {
    Mat frame;
    this->cap->read(frame);
    cvtColor(frame,frame,CV_BGR2RGB);
    return frame.data;
}
