/*
 *  ExhaustiveTracking.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 19-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _EXHAUSTIVETRACKING_H
#define _EXHAUSTIVETRACKING_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"

#include "CovarianceFull.h"
#include "CovariancePatch.h"

#include "Utilities.h"

using namespace cv;


class ExhaustiveTracking {
	
public:
	
	ExhaustiveTracking(GenericModel* model, Mat& img, config_SystemParameter *param);
	ExhaustiveTracking(){};
	~ExhaustiveTracking(){};
	
	void update(Mat &img);
	//RotatedRect getNextPosition(int fps);
	double getNextPosition(RotatedRect& final_rect, int fps);
	void updateVariables(GenericModel *_model, Mat& _img, config_SystemParameter *_param);
	void updateVariables(GenericModel *_model, RotatedRect _rect);
	void updateVariables(RotatedRect _rect);
	
private:
	Mat m_img;
	RotatedRect m_rect;
	GenericModel *m_TrackingModel;
	GenericFeature *m_feature;
	config_SystemParameter *tracker_param;
	
};

#endif

