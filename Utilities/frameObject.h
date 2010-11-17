/*
 *  frameObject.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-11-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */
#ifndef _FRAMEOBJECT_H
#define _FRAMEOBJECT_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"
#include <dirent.h>

using namespace cv;

class frameObject{
public:
	bool isVideo;
	
	frameObject();
	frameObject(string _path);
	
	~frameObject();
	void iniCap(string _path);
	
	Mat getNextFrame();
	void setSeeker(double value, bool isFrame);
	int getSeeker();
	double getFPS();

private:
	
	VideoCapture video;
	vector<string> filesPath;
	int indexFrame;
};


#endif
