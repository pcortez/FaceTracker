/*
 *  TimeQueue.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 14-09-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _TIMEQUEUE_H
#define _TIMEQUEUE_H

#define topLimitStart 20
#define rightLimitStart 190

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"

using namespace cv;

double calculateQueueTime(int _iniFrame, int _currentFrame, int _fps);
void drawLimit(Mat& _img);
bool checkStartLimit(RotatedRect _objectRect, const Mat& _img);
void drawTime(Mat& _img, double _seconds);


#endif