/*
 *  Generic.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _GENERIC_H
#define _GENERIC_H


#include "configStruct.h"

using namespace cv;

//enum Feature { HAAR, COV_SUB_WINDOWS, COV_FULL_IMAGE, COV_MIL, COV_SUB_WINDOWS_B};
//enum Norm_SubWindows {FACE, PEDESTRIAN};

//Prototype
class GenericModel;
class GenericFeature;


class GenericFeature {
	
public:
	GenericFeature(){};
	//~GenericFeature();
	
	virtual void computeFeature(GenericModel *model) = 0;
	virtual double distance(GenericFeature *feature1, GenericFeature *feature2){cout<<"NO HICE NADA"<<endl; return 10000;};
	virtual void clear() = 0;
	
	Feature featureType;
	RotatedRect m_rect;
	config_SystemParameter *tracker_param;
	
private:
	//NADA
	
};

class GenericModel {
	
public:
	
	GenericModel(){};
	//~GenericModel();
	
	virtual void updateModel(Mat& src) = 0;
	virtual double distance(GenericFeature* feature1) = 0;
	virtual void updateOnlineModel(GenericFeature *feature1) = 0;
	
	Feature ModelType;
	RotatedRect m_last_rect;
	RotatedRect m_first_rect;
	
	config_SystemParameter *tracker_param;
	
private:
	//NADA
	
};
#endif
