/*
 *  ExhaustiveTracking.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 19-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "ExhaustiveTracking.h"

ExhaustiveTracking::ExhaustiveTracking(GenericModel* model, Mat& img, config_SystemParameter *param){
	this->m_rect = model->m_first_rect;
	this->m_img = img;	
	this->m_TrackingModel = model;
	this->tracker_param = param;
}

void ExhaustiveTracking::update(Mat &img){
	this->m_img = img;
	m_TrackingModel->updateModel(img);
}

double ExhaustiveTracking::getNextPosition(RotatedRect& final_rect, int fps){

	cv::Rect limits = m_rect.boundingRect();
	Point2i p1(limits.x, limits.y);
	Point2i p2(limits.x+limits.width, limits.y+limits.height);
	
	
	p1.x += ((tracker_param->areaSearch == 0) ? 0 : limits.width/tracker_param->areaSearch);
	p2.x -= ((tracker_param->areaSearch == 0) ? 0 : limits.width/tracker_param->areaSearch);
	p1.y += ((tracker_param->areaSearch == 0) ? 0 : limits.height/tracker_param->areaSearch);
	p2.y -= ((tracker_param->areaSearch == 0) ? 0 : limits.height/tracker_param->areaSearch);
	
	RotatedRect minRect(Point2i(0,0), Size2i(1,1),0);
	
	//OJO depende si es probabilidad
	double minDist = 10000000;
	GenericFeature *minFeature;
	//y
	for (int row=p1.y; row<=p2.y; row++) {
		if (row<0) continue;
		//x
		for (int col=p1.x; col<=p2.x; col++) {
			if (col<0) continue;
			//angle
			for (int angle=0; angle<tracker_param->angleVector.size(); angle++) {
				//scales
				for (int k=0; k<tracker_param->scaleVector.size(); k++) {
					if (col<0 || row<0)	continue;
					
					
					RotatedRect auxRect(Point2i(col,row),
										Size2i(m_TrackingModel->m_first_rect.size.width*tracker_param->scaleVector[k],
											   m_TrackingModel->m_first_rect.size.height*tracker_param->scaleVector[k]),
										tracker_param->angleVector[angle]+m_rect.angle);
					
					//revisar para los angulos
					limits = getBoundingRect(auxRect);
					if (limits.x<0 || limits.y<0 || 
						limits.x+limits.width>=m_img.cols || limits.y+limits.height>=m_img.rows) continue;
					
					
					if (this->m_TrackingModel->ModelType == COV_FULL_IMAGE)
						m_feature = (GenericFeature *)new CovarianceFullDescriptor(auxRect,this->m_TrackingModel->tracker_param);
					else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS)
						m_feature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,this->m_TrackingModel->tracker_param);
					else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS_B)
						m_feature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,this->m_TrackingModel->tracker_param);
					
					m_feature->computeFeature(this->m_TrackingModel);
					double dist = m_TrackingModel->distance(m_feature);
					
					//ojo es al revez para ciertos caso que no son probabilidad <
					if (dist<minDist) {
						minRect = auxRect;
						minDist = dist;
						minFeature = m_feature;
					}
					else {
						m_feature->clear();
						delete m_feature;
					}
				}
			}
			
		}
	}
	m_TrackingModel->updateOnlineModel(minFeature);
	m_rect = final_rect = minRect;
	return minDist;
}

void ExhaustiveTracking::updateVariables(GenericModel* _model, Mat& _img, config_SystemParameter *_param){
	this->m_rect = _model->m_first_rect;
	this->m_img = _img;	
	this->m_TrackingModel = _model;
	this->tracker_param = _param;
}

void ExhaustiveTracking::updateVariables(GenericModel *_model, RotatedRect _rect){
	this->m_rect = _rect;
	this->m_TrackingModel = _model;
}

void ExhaustiveTracking::updateVariables(RotatedRect _rect){
	this->m_rect = _rect;
	this->m_TrackingModel->m_first_rect = _rect;
}
