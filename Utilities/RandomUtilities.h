/*
 *  RandomUtilities.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 18-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _RANDOMUTILITIES_H
#define _RANDOMUTILITIES_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"

static CvRNG rng_state = cvRNG(time(0));

void randinitalize( const int init );
int randint( const int min=0, const int max=5);
float randfloat();
float randfloat(const float min, const float max ) ;
float randgaus(const float mean, const float std);

#endif
