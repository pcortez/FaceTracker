/*
 *  CovariancePatch.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#ifndef _COVARIANCEPATCH_H
#define _COVARIANCEPATCH_H

#include "Generic.h"
#include "LogEuclideanMath.h"
#include "Utilities.h"

#define SIZE_NORM_FACE 20//20
#define SIZE_NORM_PEDESTRIAN_W 20
#define SIZE_NORM_PEDESTRIAN_H 40

using namespace cv;

//Prototype
class ModelFeature;
class CovariancePatchDescriptor;
class CovariancePatchModel;
class CovariancePatchModelv2;


//
//Descriptor
//
class CovariancePatchDescriptor: public GenericFeature {
	
public:
	CovariancePatchDescriptor(RotatedRect& rect, config_SystemParameter *param);
	//CovariancePatchDescriptor(){};
	~CovariancePatchDescriptor();
	
	void computeFeature(GenericModel *model);
	void clear();
	
	//Mat m_descp;
	vector<Mat> m_descp;
	Mat m_imgPatch;
	
	double m_probability;
	
private:
	void calculatePatchCov(Mat& imgCompleta);
	void CalculateCov();
};

//
//Model
//
class CovariancePatchModel: public GenericModel{
	
public:
	CovariancePatchModel(Mat& img, const RotatedRect& rect,config_SystemParameter *param);
	CovariancePatchModel(){};
	
	void updateModel(Mat& img);
	double distance(GenericFeature *feature1);
	void updateOnlineModel(GenericFeature *feature1);
	void updateBackgroundModel(GenericFeature *feature1);
	
	
	Mat m_first_img;
	Mat m_last_img;
	
	
	CovariancePatchDescriptor *m_first_descp;
	//vector<Mat> m_first_logCov;
	vector<CovariancePatchDescriptor> m_onlineModel;
	
	vector<CovariancePatchDescriptor> m_backgroundModel;
	
	//Norm_SubWindows TypeNorm;
	
private:
	double modelDistance(CovariancePatchDescriptor *f1);
	double modelMeanDistance(CovariancePatchDescriptor *f1);
	static bool compareFeaturesDesc(const CovariancePatchDescriptor &f1, const CovariancePatchDescriptor &f2){
		return f1.m_probability > f2.m_probability;
	}
	static bool compareFeaturesAsc(const CovariancePatchDescriptor &f1, const CovariancePatchDescriptor &f2){
		return f1.m_probability < f2.m_probability;
	}
};


//
//Model 2
//
class ModelFeature {
public:
	
	ModelFeature(){m_index=0; m_probability=-1;};
	ModelFeature(int indx, Mat& cov, double prob, Point2i pos);
	~ModelFeature();
	void clear();
	
	Mat m_covMatrix;
	int m_index;
	double m_probability;
	Point2f m_pos;
	
private:
	//NADA
};

class CovariancePatchModelv2: public GenericModel{
	
public:
	CovariancePatchModelv2(Mat& img, const RotatedRect& rect, config_SystemParameter *param);
	CovariancePatchModelv2(){};
	
	void updateModel(Mat& img);
	double distance(GenericFeature *feature1);
	void updateOnlineModel(GenericFeature *feature1);
	
	Mat m_first_img;
	Mat m_last_img;
	
	
	CovariancePatchDescriptor *m_first_descp;
	vector<vector<ModelFeature> > m_onlineModel;
	
	//Norm_SubWindows TypeNorm;
	
private:
	double modelDistance(CovariancePatchDescriptor *f1);
	double updateModelDistance(int indx, Mat& covMat);
	//double updateModelDistance(int indx, CovariancePatchDescriptor *f1);
	
	static bool compareFeaturesDesc(const ModelFeature &f1, const ModelFeature &f2){
		return f1.m_probability > f2.m_probability;
	}
	static bool compareFeaturesAsc(const ModelFeature &f1, const ModelFeature &f2){
		return f1.m_probability < f2.m_probability;
	}
};



#endif