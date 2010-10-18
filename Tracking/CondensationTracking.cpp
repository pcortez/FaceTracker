/*
 *  CondensationTracking.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 27-04-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "CondensationTracking.h"

CondensationTracking::CondensationTracking(GenericModel* model, Mat& img, int numSampes){
	this->m_promAcumulado = 0;
	this->m_iteration = 0;
	this->m_rect = model->m_first_rect;
	this->m_img = img;	
	this->m_TrackingModel = model;
	this->m_cond = cvCreateConDensation(8, 4, numSampes);
	
	this->m_cond->State[0] = model->m_first_rect.center.x;
	this->m_cond->State[1] = model->m_first_rect.center.y;
	this->m_cond->State[2] = model->m_first_rect.angle;
	this->m_cond->State[3] = 1;
	
	CvMat* lowerBound;
	CvMat* upperBound;
	lowerBound = cvCreateMat(8, 1, CV_32F);
	upperBound = cvCreateMat(8, 1, CV_32F);
	
	cvmSet( lowerBound, 0, 0, model->m_first_rect.center.x-model->m_first_rect.size.width/2.5);//1); 
	cvmSet( upperBound, 0, 0, model->m_first_rect.center.x+model->m_first_rect.size.width/2.5);//(float) img.cols-1);
	
	cvmSet( lowerBound, 1, 0, model->m_first_rect.center.y-model->m_first_rect.size.height/2.5);//1.0 ); 
	cvmSet( upperBound, 1, 0, model->m_first_rect.center.y+model->m_first_rect.size.height/2.5);//(float)img.rows-1);
	
	cvmSet( lowerBound, 2, 0, -5.0 ); 
	cvmSet( upperBound, 2, 0, 5.0 );
	
	cvmSet( lowerBound, 3, 0, 0.9);//0.7 ); 
	cvmSet( upperBound, 3, 0, 1.1);//2.0 );
	
	cvmSet( lowerBound, 4, 0, -0.001);//-25.5); 
	cvmSet( upperBound, 4, 0, 0.001);//25.5);
	
	cvmSet( lowerBound, 5, 0, -0.001);//-25.5 ); 
	cvmSet( upperBound, 5, 0, 0.001);//25.5  );
	
	cvmSet( lowerBound, 6, 0, -0.001);//-25 ); 
	cvmSet( upperBound, 6, 0, 0.001);//25  );
	
	cvmSet( lowerBound, 7, 0, -0.001);//-1.0 ); 
	cvmSet( upperBound, 7, 0,  0.001);//1.0 );
	
	cvConDensInitSampleSet(this->m_cond, lowerBound, upperBound);
	
	for(int i=0; i < numSampes; i++){
		this->m_cond->flSamples[i][0] = randint(cvRound(model->m_first_rect.center.x-model->m_first_rect.size.width/2.5), 
												cvRound(model->m_first_rect.center.x+model->m_first_rect.size.width/2.5));//model->m_first_rect.center.x;
		this->m_cond->flSamples[i][1] = randint(cvRound(model->m_first_rect.center.y-model->m_first_rect.size.height/2.5), 
												cvRound(model->m_first_rect.center.y+model->m_first_rect.size.height/2.5));//model->m_first_rect.center.y;
		this->m_cond->flSamples[i][2] = model->m_first_rect.angle;//randfloat(model->m_first_rect.angle-1,model->m_first_rect.angle+1);
		this->m_cond->flSamples[i][3] = 1;//randfloat(0.9,1.1);//1;
		
		this->m_cond->flSamples[i][4] = randfloat(-0.00,0.00);//(-20, 20);
		this->m_cond->flSamples[i][5] = randfloat(-0.00,0.00);//(-20, 20);
		this->m_cond->flSamples[i][6] = randfloat(-0.00,0.00);//(-10, 10);
		this->m_cond->flSamples[i][7] = randfloat(-0.00,0.00);//(-15.1, 15.1);
	}
	/*
	const float F[] = {1,0,0,0,0,0,0,0,//x 0
					   0,1,0,0,0,0,0,0,//y 1
					   0,0,1,0,0,0,0,0,//angulo 2
					   0,0,0,1,0,0,0,0,//escala 3
					   1,0,0,0,1,0,0,0,//x punto 4
					   0,1,0,0,0,1,0,0,//y punto 5
					   0,0,1,0,0,0,1,0,//angulo punto 6
					   0,0,0,1,0,0,0,1//escala punto  7	 
	};
	*/
	const float F[] = {1,0,0,0,1,0,0,0,//x 0 - x +dx*t
					   0,1,0,0,0,1,0,0,//y 1 - y+dy*t
					   0,0,1,0,0,0,1,0,//a 2 - a+da*t
					   0,0,0,1,0,0,0,1,//s 3 - s+ds*t
					   0,0,0,0,1,0,0,0,//dx 4 - dx = dx
					   0,0,0,0,0,1,0,0,//dy 5 - dy = dy
					   0,0,0,0,0,0,1,0,//da 6 - da = da
					   0,0,0,0,0,0,0,1 //ds 7 - ds = ds	 
	};
	
	memcpy( m_cond->DynamMatr, F, sizeof(F));
	
}

void CondensationTracking::update(Mat &img){
	this->m_img = img;
	m_TrackingModel->updateModel(img);
}

double CondensationTracking::getNextPosition(RotatedRect& final_rect, int fps){
	
	cv::Rect limits, bounding = getBoundingRect(m_rect);
	Point2i p1(bounding.x, bounding.y);
	Point2i p2(bounding.x+bounding.width, bounding.y+bounding.height);
	
	//p1.x += bounding.width/2.5;
	//p2.x -= bounding.width/2.5;
	//p1.y += bounding.height/2.5;
	//p2.y -= bounding.height/2.5;
	
	Mat tmp = this->m_img.clone();
	
	for(int i = 0; i < m_cond->SamplesNum; i++){

		RotatedRect auxRect(Point2i(m_cond->flSamples[i][0],m_cond->flSamples[i][1]),
							Size2i(m_TrackingModel->m_first_rect.size.width*m_cond->flSamples[i][3],
								   m_TrackingModel->m_first_rect.size.height*m_cond->flSamples[i][3]),
							m_cond->flSamples[i][2]);
		
		//cout << "dx: "<<m_cond->flSamples[i][4]<<" dy: "<<m_cond->flSamples[i][5]<<
		//" da: "<<m_cond->flSamples[i][6]<<" ds: "<<m_cond->flSamples[i][7]<<endl;
		
		circle(tmp, auxRect.center,1,CV_RGB(255,0,0),-1);		
		limits = getBoundingRect(auxRect);
		

		if (limits.x<0 || limits.y<0 || limits.x+limits.width>=m_img.cols || limits.y+limits.height>=m_img.rows || 
			auxRect.size.width<4 || auxRect.size.height<4 || 
			auxRect.size.width>m_rect.size.width*1.5 || auxRect.size.height>m_rect.size.height*1.5 ||
			m_cond->flSamples[i][0]<p1.x || m_cond->flSamples[i][0]>p2.x || 
			m_cond->flSamples[i][1]<p1.y || m_cond->flSamples[i][1]>p2.y){
			//abs(m_cond->flSamples[i][4])>0.01 || abs(m_cond->flSamples[i][5])>0.01 || abs(m_cond->flSamples[i][6])>0.01 || abs(m_cond->flSamples[i][7])>0.01){
			m_cond->flConfidence[i]=0.0;
			continue;
		}
		
		
		if (this->m_TrackingModel->ModelType == COV_FULL_IMAGE)
			m_feature = (GenericFeature *)new CovarianceFullDescriptor(auxRect,this->m_TrackingModel->tracker_param);
		else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS)
			m_feature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,this->m_TrackingModel->tracker_param);
		else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS_B)
			m_feature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,this->m_TrackingModel->tracker_param);
		
		m_feature->computeFeature(this->m_TrackingModel);
		double dist = m_TrackingModel->distance(m_feature);
		m_feature->clear();
		
		
		//m_cond->flConfidence[i] = exp(-0.5*(((getAvg()-dist)*(getAvg()-dist))/getVar()));
		//m_cond->flConfidence[i] = exp(-abs(dist-getAvg()));
		m_cond->flConfidence[i] = exp(-dist);
		//m_cond->flConfidence[i] = exp((-0.5*dist*dist)/getVar());
		//m_cond->flConfidence[i] = ((dist<0.001) ? 1 : 1/dist );
		//m_cond->flConfidence[i] = 1.0/(sqrt(2.0*Condense_PI) * getVar())*exp(-0.5 * (dist*dist /getVar()));
		//cout << "m_cond->flConfidence[i]: "<<m_cond->flConfidence[i]<<" dist: "<<dist<<endl;
		
	}
	
	cvConDensUpdateByTime(m_cond);
	cout << "x: "<<m_cond->State[0]<<" y: "<<m_cond->State[1]<<
			" angulo: "<<m_cond->State[2]<<" escala: "<<m_cond->State[3]<<endl;
	
	if (m_cond->State[0] != m_cond->State[0] || m_cond->State[1] != m_cond->State[1] ||
		m_cond->State[2] != m_cond->State[2] || m_cond->State[3] != m_cond->State[3]) {
		
		final_rect = m_rect;
		cout << "Probability negative, restart trackier "<<endl;
		return -1;
	}
	
	RotatedRect minRect(Point2i(cvRound(m_cond->State[0]),cvRound(m_cond->State[1])),
						Size2i(m_TrackingModel->m_first_rect.size.width*m_cond->State[3],
							   m_TrackingModel->m_first_rect.size.height*m_cond->State[3]),
						(abs(cvRound(m_cond->State[2]))<=1 || this->m_TrackingModel->ModelType == COV_FULL_IMAGE ? 0:m_cond->State[2]));
	
	circle(tmp, minRect.center,2,CV_RGB(255,255,255),-1);
	drawRotatedRect(tmp, minRect,CV_RGB(255,255,0),1);
	limits = getBoundingRect(minRect);
	rectangle(tmp, limits.tl(),limits.br(), CV_RGB(0,255,255), 1);
	
	cvNamedWindow("Particle distribution");
	imshow( "Particle distribution", tmp);
	//cvWaitKey();
	
	if (this->m_TrackingModel->ModelType == COV_FULL_IMAGE)
		m_feature = (GenericFeature *)new CovarianceFullDescriptor(minRect,this->m_TrackingModel->tracker_param);
	else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS)
		m_feature = (GenericFeature *)new CovariancePatchDescriptor(minRect,this->m_TrackingModel->tracker_param);
	else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS_B)
		m_feature = (GenericFeature *)new CovariancePatchDescriptor(minRect,this->m_TrackingModel->tracker_param);
	
	m_feature->computeFeature(this->m_TrackingModel);
	double minDist = m_TrackingModel->distance(m_feature);
	//calculando varianza y promedio 
	setNewEstadisticSample(minDist);
	
	m_TrackingModel->updateOnlineModel(m_feature);
	m_rect = final_rect = minRect;
	
	
	//return minDist;
	return exp(-minDist);
	//return exp((-0.5*minDist*minDist)/getVar());
	//return exp(-0.5*(((getAvg()-minDist)*(getAvg()-minDist))/getVar()));
	//return 1.0/(sqrt(2.0*Condense_PI) * getVar())*exp(-0.5 * (minDist*minDist /getVar()));
}

CondensationTracking::~CondensationTracking(){
	cvReleaseConDensation(&m_cond);
	m_img.release();
}


//
//Private
//

double CondensationTracking::getAvg(){
	return (m_iteration<=0 ? 1: m_prom);
}
double CondensationTracking::getVar(){
	if (m_estadisticsSamples.size()<=1) return 1;
	
	double auxValue = 0;
	
	for (int i=0; i<m_estadisticsSamples.size(); i++)
		auxValue += pow(m_estadisticsSamples[i]-m_prom, 2);
	
	return auxValue/m_iteration;
}
void CondensationTracking::setNewEstadisticSample(double value){
	m_promAcumulado += value;
	m_iteration +=1;
	m_estadisticsSamples.push_back(value);
	m_prom = m_promAcumulado/m_iteration;
	//m_prom = value;
}

