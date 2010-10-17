/*
 *  CovarianceFull.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "CovarianceFull.h"

//
//Descriptor
//
CovarianceFullDescriptor::CovarianceFullDescriptor(RotatedRect& rect, config_SystemParameter *param){
	featureType = COV_FULL_IMAGE;
	this->m_rect = rect;
	this->m_descpVector.resize(5);
	this->tracker_param = param;
}

void CovarianceFullDescriptor::computeFeature(GenericModel *model){
	
	CovarianceFullModel* modelCov = (CovarianceFullModel *)model;
	
	Point2i p1(cvRound(m_rect.center.x-m_rect.size.width/2), cvRound(m_rect.center.y-m_rect.size.height/2));
	Point2i p2(cvRound(m_rect.center.x+m_rect.size.width/2), cvRound(m_rect.center.y+m_rect.size.height/2));
	
	
	p1.x = (p1.x<0)? 0 : p1.x;
	p1.y = (p1.y<0)? 0 : p1.y;
	p2.x = (p2.x>=modelCov->m_last_img.size().width)? modelCov->m_last_img.size().width-1 : p2.x;
	p2.y = (p2.y>=modelCov->m_last_img.size().height)? modelCov->m_last_img.size().height-1 : p2.y;
	
	
	//cout << "x1: "<<p1.x<<" - "<< "y1: "<<p1.y<<endl;
	//cout << "x2: "<<p2.x<<" - "<< "y2: "<<p2.y<<endl<<endl;
	
	Mat cov = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	CovarianceRegion(modelCov->m_P, modelCov->m_Q, cov,p1,p2,tracker_param);
	//printMat(Crxy,MAT_TYPE_DOUBLE);
	
	this->m_descp = mapLogMat(cov,tracker_param);
	
	/*
	if (!m_descpVector.empty()) m_descpVector.clear();
	
	m_descpVector[0] = mapLogMat(cov);
	
	int new_halfWidth = abs(p1.x-p2.x)/2;
	int new_halfHeight = abs(p1.y-p2.y)/2;
	
	//LEFT ZONE
	cov = Mat(NUM_FEATURES,NUM_FEATURES,CV_64F);
	CovarianceRegion(modelCov->m_P, modelCov->m_Q, cov,p1,Point2i(p1.x+new_halfWidth,p2.y));
	m_descpVector.push_back(mapLogMat(cov));
	//RIGHT ZONE
	cov = Mat(NUM_FEATURES,NUM_FEATURES,CV_64F);
	CovarianceRegion(modelCov->m_P, modelCov->m_Q, cov,Point2i(p1.x+new_halfWidth,p1.y),p2);
	m_descpVector.push_back(mapLogMat(cov));
	//TOP ZONE
	cov = Mat(NUM_FEATURES,NUM_FEATURES,CV_64F);
	CovarianceRegion(modelCov->m_P, modelCov->m_Q, cov,p1,Point2i(p2.x,p1.y+new_halfHeight));
	m_descpVector.push_back(mapLogMat(cov));
	//BOTTOM ZONE
	cov = Mat(NUM_FEATURES,NUM_FEATURES,CV_64F);
	CovarianceRegion(modelCov->m_P, modelCov->m_Q, cov,Point2i(p1.x,p1.y+new_halfHeight),p2);
	m_descpVector.push_back(mapLogMat(cov));
	*/
}

//
//Model
//
CovarianceFullModel::CovarianceFullModel(Mat& img, RotatedRect& rect,config_SystemParameter *param){
	
	ModelType = COV_FULL_IMAGE;
	this->tracker_param = param;
	this->m_first_rect = rect;
	this->m_first_img = img;
	updateModel(this->m_first_img);
	
	m_first_descp = new CovarianceFullDescriptor(rect,tracker_param);
	m_first_descp->computeFeature(this);
	
	
}

void CovarianceFullModel::updateModel(Mat& img){
	
	this->m_last_img = img;
	
	vector<Mat> F(tracker_param->numFeature);
	if (tracker_param->isRGB) {
		makeFrgb(img, F,tracker_param);
	}
	else{
		makeFgray(img, F,tracker_param);
	}
	//makeF(img, F,tracker_param);
	//printMat(F,MAT_TYPE_FLOAT);
	
	vector<Mat> P(tracker_param->numFeature);
	vector< vector<Mat> > Q(tracker_param->numFeature, vector<Mat>(tracker_param->numFeature));
	makePQ(F, P,Q,tracker_param);
	//printMat(P,MAT_TYPE_DOUBLE);
	
	this->m_P = P;
	this->m_Q = Q;
	
}

double CovarianceFullModel::distance(GenericFeature *feature1){
	
	CovarianceFullDescriptor *f1 = (CovarianceFullDescriptor *) feature1;

	return regionDist(this->m_first_descp->m_descp,f1->m_descp,tracker_param);
	//return regionDist(this->m_first_descp->m_descpVector,f1->m_descpVector);
}