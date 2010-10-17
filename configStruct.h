/*
 *  configStruct.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 25-08-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _CONFIGPARAM_H
#define _CONFIGPARAM_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"


enum Feature { HAAR, COV_SUB_WINDOWS, COV_FULL_IMAGE, COV_MIL, COV_SUB_WINDOWS_B};
enum Norm_SubWindows {FACE, PEDESTRIAN};

struct config_SystemParameter {
	//tracking param
	vector<double> angleVector;
	vector<double> scaleVector;
	float areaSearch; //en el caso que sea 0 revisar pke va dividir por 0
	
	//model param
	int modelMemory; //casi siempre es 30
	double alpha; //casi siempre es 0.05;
	
	//sistem param
	int numFeature;
	cv::RotatedRect initialRect;
	Norm_SubWindows normType;
	bool isRGB;
	string videoPath;//incluey el nombre del video pero no el .formato
	double startMsec;
	double endMsec;
};

struct CaptionDrawData {
	vector<string> texts;
	vector<CvScalar> colorLine;
	vector< vector<cv::RotatedRect> > coord;
};


#endif