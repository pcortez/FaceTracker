/*
 *  TimeQueue.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 14-09-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "TimeQueue.h"
#include <iomanip>

double calculateQueueTime(int _iniFrame, int _currentFrame, int _fps){
	CV_Assert(_currentFrame>=_iniFrame);
	return (1/(double)_fps)*(_currentFrame-_iniFrame);
}

void drawLimit(Mat& _img){
	
	//line(_img, Point2i(0, topLimitStart), Point2i(_img.size().width-rightLimitStart, topLimitStart), CV_RGB(255,0,0), 2);
	line(_img, Point2i(_img.size().width-rightLimitStart, 0), 
			   Point2i(_img.size().width-rightLimitStart, _img.size().height), 
		 CV_RGB(0,0,255), 2);
	
	line(_img, Point2i(10, 0),Point2i(10, _img.size().height),CV_RGB(255,0,0), 2);
	//line(_img, Point2i(10, _img.size().height-10),Point2i(_img.size().width-10,_img.size().height-10),
	//	 CV_RGB(100,50,50), 2);
}

bool checkStartLimit(RotatedRect _objectRect, const Mat& _img){
	if (_objectRect.center.x <= _img.size().width-rightLimitStart && 
		_objectRect.center.y >= topLimitStart) {
		cout << "START QUEUE WAITING TIME"<<endl;
		return true;
	}
	else {
		return false;
	}

}

void drawTime(Mat& _img, double _seconds){
	int minute = _seconds/60;
	double sec = _seconds-minute*60;
	int baseline=0;
	string title = "QUEUE WAITING TIME";
	std::stringstream out;
	out << minute <<":"<<sec;
	
	
	cv::Size textSize = getTextSize(out.str(), FONT_HERSHEY_SIMPLEX,	0.5, 1, &baseline);
	cv::Size titleSize = getTextSize(title, FONT_HERSHEY_SIMPLEX,	0.5, 1, &baseline);
	
	int maxWidth = max(textSize.width,titleSize.width);
	
	rectangle(_img, Point2i(_img.size().width-maxWidth,_img.size().height-50), Point2i(_img.size().width, _img.size().height)
			  ,Scalar::all(255),CV_FILLED);
	putText(_img, title, Point2i(_img.size().width-maxWidth,_img.size().height-35), 
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar::all(0), 1, 8);
	putText(_img, out.str(), Point2i(_img.size().width-maxWidth,_img.size().height-10), 
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar::all(0), 1, 8);
	
}