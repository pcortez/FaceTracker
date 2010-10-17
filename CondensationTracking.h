/*
 *  CondensationTracking.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 27-04-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _CONDENSATIONTRACKING_H
#define _CONDENSATIONTRACKING_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"

#include "CovarianceFull.h"
#include "CovariancePatch.h"
#include "RandomUtilities.h"
static const double Condense_PI = 3.14159265358979323846;
using namespace cv;


class CondensationTracking {
	
public:
	CondensationTracking(GenericModel* model, Mat& img, int numSampes);
	~CondensationTracking();
	
	void update(Mat &img);
	//RotatedRect getNextPosition(int fps);
	double getNextPosition(RotatedRect& final_rect, int fps);
	
private:
	
	double getAvg();
	double getVar();
	void setNewEstadisticSample(double value);
	
	
	Mat m_img;
	RotatedRect m_rect;
	GenericModel *m_TrackingModel;
	GenericFeature *m_feature;
	CvConDensation *m_cond;
	double m_promAcumulado;
	double m_iteration;
	double m_prom;
	vector<double> m_estadisticsSamples;
	
};

#endif