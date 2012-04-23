#include "webcam.h"

Webcam* init(){
    VideoCapture* cap = new VideoCapture(0);
    if(!cap->isOpened()) return 0;
    Webcam* web = new Webcam;
    web->cap = cap;
    web->width = cap->get(CV_CAP_PROP_FRAME_WIDTH);
    web->height = cap->get(CV_CAP_PROP_FRAME_HEIGHT);

    return web;
}

uchar* getData(Webcam *cam) {
    Mat frame;
    (*cam->cap) >> frame;
    cvtColor(frame,frame,CV_BGR2RGB);
    return frame.data;
}
