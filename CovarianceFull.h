/*
 *  CovarianceFull.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _COVARIANCEFULL_H
#define _COVARIANCEFULL_H

#include "Generic.h"
#include "LogEuclideanMath.h"

using namespace cv;

//Prototype
class CovarianceFullDescriptor;
class CovarianceFullModel;

//
//Descriptor
//
class CovarianceFullDescriptor: public GenericFeature {
	
public:
	CovarianceFullDescriptor(RotatedRect& rect, config_SystemParameter *param);
	~CovarianceFullDescriptor(){};
	
	void computeFeature(GenericModel *model);
	
	void clear(){ cout << "NO HICE NADA FULL COV" <<endl; };
	
	
	Mat m_descp;
	vector<Mat> m_descpVector;
private:
	//NADA	
};

//
//Model
//
class CovarianceFullModel: public GenericModel{
	
public:
	CovarianceFullModel(Mat& img, RotatedRect& rect,config_SystemParameter *param);
	~CovarianceFullModel(){};
	
	void updateModel(Mat& img);
	double distance(GenericFeature *feature1);
	void updateOnlineModel(GenericFeature *feature1){ cout << "NO HICE NADA FULL COV" <<endl; };
	
	Mat m_first_img;
	Mat m_last_img;
	
	Mat m_first_logCov;
	//vector<Mat> m_first_descpVector;
	CovarianceFullDescriptor *m_first_descp;
	
	vector<Mat> m_P;
	vector< vector<Mat> > m_Q;
	
private:
	//NADA
};

#endif