/*
 *  CovariancePatch.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "CovariancePatch.h"
#include "RandomUtilities.h"

#define PI 3.14159265358979323846
//
//Descriptor
//
CovariancePatchDescriptor::CovariancePatchDescriptor(RotatedRect& rect, config_SystemParameter *param){
	featureType = COV_SUB_WINDOWS;
	this->m_rect = rect;
	this->tracker_param = param;	
}

CovariancePatchDescriptor::~CovariancePatchDescriptor(){
	this->clear();
};

void CovariancePatchDescriptor::computeFeature(GenericModel *model){
	
	if (model->ModelType == COV_SUB_WINDOWS) {
		CovariancePatchModel* modelCov = (CovariancePatchModel *)model;
		calculatePatchCov(modelCov->m_last_img);
	}
	else if (model->ModelType == COV_SUB_WINDOWS_B){
		CovariancePatchModelv2* modelCov = (CovariancePatchModelv2 *)model;
		calculatePatchCov(modelCov->m_last_img);
	}
	else {
		CV_Assert(false);
	}	
	//calculatePatchCov(modelCov->m_last_img, modelCov->TypeNorm);
}

void CovariancePatchDescriptor::clear(){
	
	for (int i=0; i<m_descp.size(); i++) {
		m_descp[i].release();
	}
	m_imgPatch.release();
}

//Private
void CovariancePatchDescriptor::calculatePatchCov(Mat& imgCompleta){
	
	Mat auxPatch;
	
	if (abs(this->m_rect.angle)>1) {
		Mat rot = getRotationMatrix2D( m_rect.center, m_rect.angle, 1);
		warpAffine(imgCompleta, auxPatch, rot, imgCompleta.size());
	}
	else
		auxPatch = imgCompleta;

	//cout << "center.x: "<<m_rect.center.x<<" center.y: "<<m_rect.center.y<<" size.width: "<<m_rect.size.width<<" size.height: "<<m_rect.size.height<<endl;
	
	m_rect.center.x = m_rect.center.x<0 ? 0 : m_rect.center.x;
	m_rect.center.y = m_rect.center.y<0 ? 0 : m_rect.center.y;
	m_rect.center.x = m_rect.center.x>=auxPatch.size().width ? auxPatch.size().width-1 : m_rect.center.x;
	m_rect.center.y = m_rect.center.y>=auxPatch.size().height ? auxPatch.size().width-1 : m_rect.center.y;
	
	int limUpX		= (int)m_rect.center.x+(m_rect.size.width/2);
	int limDownX	= (int)m_rect.center.x-(m_rect.size.width/2);
	int limUpY		= (int)m_rect.center.y+(m_rect.size.height/2);
	int limDownY	= (int)m_rect.center.y-(m_rect.size.height/2);
	
	//cout << "center.x: "<<m_rect.center.x<<" center.y: "<<m_rect.center.y<<" size.width: "<<m_rect.size.width<<" size.height: "<<m_rect.size.height<<endl;
	//cout << "x1: "<<limDownX<<" x2: "<<limUpX<<" y1: "<<limDownY<<" y2: "<<limUpY<<endl;
	
	if (limDownX<0) limDownX=0;
	if (limDownY<0) limDownY=0;
	if (limUpX>=auxPatch.size().width) limUpX=auxPatch.size().width-1;
	if (limUpY>=auxPatch.size().height) limUpY=auxPatch.size().height-1;
	
	if (limUpX<limDownX) {
		int aux = limUpX;
		limUpX = limDownX;
		limDownX = aux;
	}
	
	if (limUpY<limDownY) {
		int aux = limUpY;
		limUpY = limDownY;
		limDownY = aux;
	}
	
	//cout << "x1: "<<limDownX<<" x2: "<<limUpX<<" y1: "<<limDownY<<" y2: "<<limUpY<<endl;
	
	auxPatch = auxPatch(Range(limDownY,limUpY),Range(limDownX,limUpX));
	if (this->tracker_param->normType ==FACE)
		resize(auxPatch, this->m_imgPatch,cv::Size(SIZE_NORM_FACE,SIZE_NORM_FACE));
	else if(this->tracker_param->normType==PEDESTRIAN)
		resize(auxPatch, this->m_imgPatch,cv::Size(SIZE_NORM_PEDESTRIAN_W,SIZE_NORM_PEDESTRIAN_H));
	else
		resize(auxPatch, this->m_imgPatch,cv::Size(SIZE_NORM_FACE,SIZE_NORM_FACE));
	
	
	//imwrite(CreatefileNameString(), imgCompleta);
	//imwrite(CreatefileNameString(), auxPatch);
	//imwrite(CreatefileNameString(), this->m_imgPatch);
	//cvWaitKey();

	CalculateCov();
}

void CovariancePatchDescriptor::CalculateCov(){
	
	vector<Mat> F(tracker_param->numFeature);
	if (tracker_param->isRGB) {
		makeFrgb(this->m_imgPatch, F,tracker_param);
	}
	else{
		makeFgray(this->m_imgPatch, F,tracker_param);
	}
	//makeF(this->m_imgPatch, F,tracker_param);
	//printMat(F,MAT_TYPE_FLOAT);
	vector<Mat> P(tracker_param->numFeature);
	vector< vector<Mat> > Q(tracker_param->numFeature, vector<Mat>(tracker_param->numFeature));
	makePQ(F, P,Q,tracker_param);
	//printMat(P,MAT_TYPE_DOUBLE);
	
	Mat cov = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	CovarianceRegion(P, Q, cov,tracker_param);
	//printMat(Crxy,MAT_TYPE_DOUBLE);
	
	//
	//WE CALCULATE DIFFERENT LOG-MAPS, FOR DIFFERENT IMAGE PATCH'S AREAS
	//
	Mat covAux;
	int halfw,halfh,h,w;
	if (this->tracker_param->normType == FACE){
		halfw = halfh = SIZE_NORM_FACE/2;
		h = w = SIZE_NORM_FACE;
	}
	else{
		halfh = SIZE_NORM_PEDESTRIAN_H/2;
		halfw = SIZE_NORM_PEDESTRIAN_W/2;
		h = SIZE_NORM_PEDESTRIAN_H;
		w = SIZE_NORM_PEDESTRIAN_W;
	}
	
	if (!this->m_descp.empty()) this->m_descp.clear();
	
	this->m_descp.resize(5);
	
	//FULL PATCH
	this->m_descp[0] = mapLogMat(cov,tracker_param);
	
	//LEFT HALF PATCH
	covAux = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	//CovarianceRegion(P, Q, covAux,Point2f(0,0), Point2f(halfw-1,h-1));
	CovarianceRegion(P, Q, covAux,Point2f(0,0), Point2f(halfw-1,halfh-1),tracker_param);
	this->m_descp[1] = mapLogMat(covAux, tracker_param);
	
	//RIGHT HALF PATCH
	covAux = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	//CovarianceRegion(P, Q, covAux,Point2f(halfw,0), Point2f(w-1,h-1));
	CovarianceRegion(P, Q, covAux,Point2f(halfw,0), Point2f(w-1,halfh-1),tracker_param);
	this->m_descp[2] = mapLogMat(covAux,tracker_param);
	
	//BOTTOM HALF PATCH
	covAux = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	//CovarianceRegion(P, Q, covAux,Point2f(0,halfh), Point2f(w-1,h-1));
	CovarianceRegion(P, Q, covAux,Point2f(0,halfh), Point2f(halfw-1,h-1),tracker_param);
	this->m_descp[3] = mapLogMat(covAux,tracker_param);
	
	//TOP HALF PATCH
	covAux = Mat(tracker_param->numFeature,tracker_param->numFeature,CV_64F);
	//CovarianceRegion(P, Q, covAux,Point2f(0,0), Point2f(w-1,halfh-1));
	CovarianceRegion(P, Q, covAux,Point2f(halfw,halfh), Point2f(w-1,h-1),tracker_param);
	this->m_descp[4] = mapLogMat(covAux,tracker_param);
	
	//borrando
	cov.release();
	covAux.release();
	P.clear();
	Q.clear();
	this->m_imgPatch.release();
}

//
//Model
//
CovariancePatchModel::CovariancePatchModel(Mat& img, const RotatedRect& rect, config_SystemParameter *param){
	
	ModelType = COV_SUB_WINDOWS;
	//this->TypeNorm = t_norm;
	this->tracker_param = param;
	
	this->m_first_rect = rect;
	this->m_first_img = this->m_last_img = img;
	
	this->m_first_descp = new CovariancePatchDescriptor(this->m_first_rect,tracker_param);
	this->m_first_descp->computeFeature(this);
	
	this->m_first_descp->m_probability = 0;
	this->m_onlineModel.push_back(*m_first_descp);
	//updateBackgroundModel(m_first_descp);
}

void CovariancePatchModel::updateModel(Mat& img){
	this->m_last_img = img;
}

double CovariancePatchModel::distance(GenericFeature *feature1){
	
	CovariancePatchDescriptor *f1 = (CovariancePatchDescriptor *) feature1;
	//double dist = regionDist(this->m_first_descp->m_descp,f1->m_descp);
	double dist = modelDistance(f1);
	
	//double sigma = 1;
	//f1->m_probability = 1.0/(sqrt(2.0*PI) * sigma)*exp(-0.5 * (dist*dist / (sigma*sigma)));
	f1->m_probability = dist;//modelMeanDistance(f1);//dist;//dist es mejor
	//cout << "f1->m_probability: "<<f1->m_probability<<endl;
	return dist;
}

void CovariancePatchModel::updateOnlineModel(GenericFeature *feature1){
	
	int numModel = 30;//30;//15
	CovariancePatchDescriptor *f1 = (CovariancePatchDescriptor *) feature1;
	if (this->m_onlineModel.size()<numModel) {
		this->m_onlineModel.push_back(*f1);
	}
	else {
		sort (this->m_onlineModel.begin(),this->m_onlineModel.end(), compareFeaturesDesc);
		
		//cout << "p_ini: "<<m_onlineModel[0].m_probability<<endl;
		//cout << "p_end: "<<m_onlineModel[numModel-1].m_probability<<endl;
		//cout << "p_f: "<<f1->m_probability<<endl;
		
		//if (this->m_onlineModel[0].m_probability >= f1->m_probability){
			this->m_onlineModel[0].clear();
			this->m_onlineModel[0] = *f1;
		//}
	}
}


void CovariancePatchModel::updateBackgroundModel(GenericFeature *feature1){
	CovariancePatchDescriptor *f1 = (CovariancePatchDescriptor *) feature1;
	
	//int min_radius = ceil( sqrt( pow(f1->m_rect.size.width/2,2)+pow(f1->m_rect.size.height/2,2) )*2);
	int min_radius = ceil(min(f1->m_rect.size.width,f1->m_rect.size.height));
	int randAngle = 0, x = 0, y = 0;
	int numMemorie = 20;
	
	/*
	//Mat tmp = this->m_last_img.clone();
	m_backgroundModel.clear();
	
	for (int i=0; i<numMemorie; i++) {
		//randAngle = randint((360/numMemorie)*i, (360/numMemorie)*(i+1));
		randAngle = (360/numMemorie)*i;
		
		x = f1->m_rect.center.x + cos(randAngle)*min_radius;
		y = f1->m_rect.center.y + sin(randAngle)*min_radius;
		
		//cout << "x: "<<x<< " y: "<<y<<endl;
		
		RotatedRect auxRect(Point2i(x,y),Size2i(f1->m_rect.size.width, f1->m_rect.size.height),0);
		CovariancePatchDescriptor auxFeature(auxRect);
		auxFeature.computeFeature(this);
		//distance(&auxFeature);
		m_backgroundModel.push_back(auxFeature);
		//drawRotatedRect(tmp, auxRect,CV_RGB(0,0,255),1);
		//imwrite(CreatefileNameString(), auxFeature.m_imgPatch);
	}
	//cvNamedWindow("Particle distribution");
	//imshow( "Particle distribution", tmp);
	//cvWaitKey();
	*/
	
	
	if (m_backgroundModel.empty()) {
		for (int i=0; i<numMemorie; i++) {
			randAngle = randint((360/numMemorie)*i, (360/numMemorie)*(i+1));
			x = f1->m_rect.center.x + cos(randAngle)*min_radius;
			y = f1->m_rect.center.y + sin(randAngle)*min_radius;
			
			RotatedRect auxRect(Point2i(x,y),Size2i(f1->m_rect.size.width, f1->m_rect.size.height),0);
			CovariancePatchDescriptor auxFeature(auxRect,tracker_param);
			auxFeature.computeFeature(this);
			//distance(&auxFeature);
			m_backgroundModel.push_back(auxFeature);
		}
	}
	
	else {
		//sort (this->m_backgroundModel.begin(),this->m_backgroundModel.end(), compareFeaturesAsc);
		for (int i=0; i<numMemorie; i++) {
			randAngle = randint((360/numMemorie)*i, (360/numMemorie)*(i+1));
			x = f1->m_rect.center.x + cos(randAngle)*min_radius;
			y = f1->m_rect.center.y + sin(randAngle)*min_radius;
			
			RotatedRect auxRect(Point2i(x,y),Size2i(f1->m_rect.size.width, f1->m_rect.size.height),0);
			CovariancePatchDescriptor auxFeature(auxRect,tracker_param);
			auxFeature.computeFeature(this);
			//distance(&auxFeature);
			//cout << "p_ini: "<<m_backgroundModel[0].m_probability<<endl;
			//cout << "p_end: "<<m_backgroundModel[numMemorie-1].m_probability<<endl;
			//cout << "auxFeature.m_probability: "<<auxFeature.m_probability<<endl;
			m_backgroundModel.push_back(auxFeature);
			//if (this->m_backgroundModel[0].m_probability < auxFeature.m_probability+5){
			//	m_backgroundModel[0] = auxFeature;
			//	sort (this->m_backgroundModel.begin(),this->m_backgroundModel.end(), compareFeaturesAsc);
			//}
		}
	}
	
	
}

//private
/*
double CovariancePatchModel::modelDistance(CovariancePatchDescriptor *f1){

	double minDist = 100000;
	double dist = 0;
	//cout << "num: "<<m_onlineModel.size()<<endl;
	for (int i=0; i<m_onlineModel.size(); i++) {
		dist = regionDist(m_onlineModel[i].m_descp,f1->m_descp);
		
		if (dist<minDist) minDist = dist;

		//dist *= m_onlineModel[i].m_probability;
		//minDist += dist;
	}

	return minDist;
	//return minDist/m_onlineModel.size();
	//return dist/m_onlineModel.size();
}
*/

/*
double CovariancePatchModel::modelDistance(CovariancePatchDescriptor *f1){
	
	double minDistModel = 10000, minDistBackground = 10000;
	double dist = 0;
	
	for (int i=0; i<m_onlineModel.size(); i++) {
		dist = regionDist(m_onlineModel[i].m_descp,f1->m_descp);
		
		if (dist<minDistModel) minDistModel = dist;
	}
	
	dist = 0;
	for (int i=0; i<m_backgroundModel.size(); i++) {
		dist = regionDist(m_backgroundModel[i].m_descp,f1->m_descp);
		
		if (dist<minDistBackground) minDistBackground = dist;
	}
	
	//cout << "minDistModel: "<<minDistModel<<" minDistBackground: "<<minDistBackground<<endl;
	
	//if (minDistBackground<0.0000001) {
	//	return 100000;
	//}
	//else {
	//	return minDistModel/minDistBackground;
	//}
	
	
	if (minDistModel<minDistBackground) {
		return minDistModel;
	}
	else {
		return 100000;
	}
	

}
*/

double CovariancePatchModel::modelDistance(CovariancePatchDescriptor *f1){
	
	double dist = 0,totalDist = 0, minDist = 100000;

	for (int nfeatures = 0; nfeatures<f1->m_descp.size(); nfeatures++){
		dist = 0;
		minDist = 100000;
		for (int nModel = 0; nModel<m_onlineModel.size(); nModel++) {
			dist = regionDist(m_onlineModel[nModel].m_descp[nfeatures],f1->m_descp[nfeatures],tracker_param);
			if (dist<minDist) minDist = dist;
		}
		totalDist += minDist;
	}	
	return totalDist;
}

double CovariancePatchModel::modelMeanDistance(CovariancePatchDescriptor *f1){
	/*
	double dist = 0;
	
	for (int nModel = 0; nModel<m_onlineModel.size(); nModel++) {
		dist += regionDist(m_onlineModel[nModel].m_descp[0],f1->m_descp[0]);
	}
	
	return dist/m_onlineModel.size();
	 */
	
	vector<Mat> modelcovMat;
	
	for (int nModel = 0; nModel<m_onlineModel.size(); nModel++) {
		modelcovMat.push_back(m_onlineModel[nModel].m_descp[0]);
	}
	double det1 = determinant(mapExpMat(f1->m_descp[0],tracker_param));
	double det2 = determinant(avgMat(modelcovMat,MAT_MAP_LOG, tracker_param));
	//cout << "det1: "<<det1<<" det2: "<<det2<<endl;
	return det2/det1;
	
}

/*
double CovariancePatchModel::modelDistance(CovariancePatchDescriptor *f1){
	
	double distBackground = 0,totalDistBackground = 0, minDistBackground = 10000;
	double distModel = 0, totalDistModel = 0, minDistModel = 10000;
	
	for (int nfeatures = 0; nfeatures<f1->m_descp.size(); nfeatures++){
		distModel = distBackground = 0;
		minDistModel = minDistBackground = 100000;
		
		for (int nModel = 0; nModel<m_onlineModel.size(); nModel++) {
			distModel = regionDist(m_onlineModel[nModel].m_descp[nfeatures],f1->m_descp[nfeatures]);
			if (distModel<minDistModel) minDistModel = distModel;
		}
		totalDistModel += minDistModel;
		
		for (int nBackground = 0; nBackground<m_backgroundModel.size(); nBackground++) {
			distBackground = regionDist(m_backgroundModel[nBackground].m_descp[nfeatures],f1->m_descp[nfeatures]);
			if (distBackground<minDistBackground) minDistBackground = distBackground;
		}
		
		totalDistBackground += minDistBackground;
	}
	
	
	if (totalDistBackground<0.0001) {
		return 1000;
	}
	else {
		return totalDistModel/totalDistBackground;
	}
}
*/


//
//Model 2
//
CovariancePatchModelv2::CovariancePatchModelv2(Mat& img, const RotatedRect& rect, config_SystemParameter *param){
	
	ModelType = COV_SUB_WINDOWS_B;
	//this->TypeNorm = t_norm;
	this->tracker_param = param;
	
	this->m_first_rect = rect;
	this->m_first_img = this->m_last_img = img;
	this->m_first_descp = new CovariancePatchDescriptor(this->m_first_rect,this->tracker_param);
	this->m_first_descp->computeFeature(this);
	
	this->m_first_descp->m_probability = 0;
	
	this->m_onlineModel.resize(5);
	
	this->m_onlineModel[0].push_back(ModelFeature(0,m_first_descp->m_descp[0],0, 
												  Point2f(m_first_descp->m_rect.center.x, m_first_descp->m_rect.center.y)));
	
	Point2f aux = getAreaCenter(m_first_descp->m_rect, 1);
	this->m_onlineModel[1].push_back(ModelFeature(1,m_first_descp->m_descp[1],0,aux));
	aux = getAreaCenter(m_first_descp->m_rect, 2);
	this->m_onlineModel[2].push_back(ModelFeature(2,m_first_descp->m_descp[2],0,aux));
	aux = getAreaCenter(m_first_descp->m_rect, 3);
	this->m_onlineModel[3].push_back(ModelFeature(3,m_first_descp->m_descp[3],0,aux));
	aux = getAreaCenter(m_first_descp->m_rect, 4);
	this->m_onlineModel[4].push_back(ModelFeature(4,m_first_descp->m_descp[4],0,aux));
}

void CovariancePatchModelv2::updateModel(Mat& img){
	this->m_last_img = img;
}

double CovariancePatchModelv2::distance(GenericFeature *feature1){
	
	CovariancePatchDescriptor *f1 = (CovariancePatchDescriptor *) feature1;
	double dist = modelDistance(f1);
	f1->m_probability = dist;
	return dist;
}

void CovariancePatchModelv2::updateOnlineModel(GenericFeature *feature1){
	
	int memory = 30;
	CovariancePatchDescriptor *f1 = (CovariancePatchDescriptor *) feature1;

	int minIndx = 0;
	double minValue = 100000, dst = 0;
	
	for (int numFeature = 1; numFeature<f1->m_descp.size(); numFeature++) {
		dst = updateModelDistance(numFeature, f1->m_descp[numFeature]);
		//dst = updateModelDistance(numFeature, f1);
		//cout << "Feature["<<numFeature<<"] = "<<dst<<endl;
		if (dst<minValue) {
			minValue = dst;
			minIndx = numFeature;
		}
	}
	
	//cout << "Feature add: "<<minIndx<<endl<<endl;
	
	Point2f aux = getAreaCenter(f1->m_rect, minIndx);
	
	if (this->m_onlineModel[minIndx].size()>memory) {
		sort (this->m_onlineModel[minIndx].begin(),this->m_onlineModel[minIndx].end(), compareFeaturesDesc);
		
		this->m_onlineModel[minIndx][0].clear();
		this->m_onlineModel[minIndx][0].m_index = minIndx;
		this->m_onlineModel[minIndx][0].m_covMatrix = f1->m_descp[minIndx];
		this->m_onlineModel[minIndx][0].m_probability = minValue;
		this->m_onlineModel[minIndx][0].m_pos.x = aux.x;
		this->m_onlineModel[minIndx][0].m_pos.y = aux.y;
	}
	else {
		this->m_onlineModel[minIndx].push_back(ModelFeature(minIndx,f1->m_descp[minIndx],minValue,aux));
	}
	
	/*
	double dst = 0;
	
	for (int numFeature=1; numFeature<f1->m_descp.size(); numFeature++) {
		dst = updateModelDistance(numFeature, f1->m_descp[numFeature]);
		Point2i aux((numFeature == 1 || numFeature == 3) ? 
					f1->m_rect.center.x-f1->m_rect.size.width/4 : f1->m_rect.center.x+f1->m_rect.size.width/4,
					(numFeature == 1 || numFeature == 2) ? 
					f1->m_rect.center.y-f1->m_rect.size.height/4 : f1->m_rect.center.y+f1->m_rect.size.height/4);
		
		if (m_onlineModel[numFeature].size()>memory) {
			sort (this->m_onlineModel[numFeature].begin(),this->m_onlineModel[numFeature].end(), compareFeaturesDesc);
			//if (this->m_onlineModel[numFeature][0].m_probability>dst) {
				
				this->m_onlineModel[numFeature][0].clear();
				this->m_onlineModel[numFeature][0].m_index = numFeature;
				this->m_onlineModel[numFeature][0].m_covMatrix = f1->m_descp[numFeature];
				this->m_onlineModel[numFeature][0].m_probability = dst;
				
			this->m_onlineModel[numFeature][0].m_pos.x = aux.x;
				this->m_onlineModel[numFeature][0].m_pos.y = aux.y;
				
		
			//}
		}
		else {
			
			this->m_onlineModel[numFeature].push_back(ModelFeature(numFeature,f1->m_descp[numFeature],numFeature,aux));
		}
	}
	 */
	
}

//Private

double CovariancePatchModelv2::modelDistance(CovariancePatchDescriptor *f1){
	
	double dist = 0,totalDist = 0, minDist = 100000;// alpha = 0.05;//0.05
	Point2f aux(0,0);
	for (int nfeatures = 1; nfeatures<f1->m_descp.size(); nfeatures++){
		dist = 0;
		minDist = 100000;
		for (int nModel = 0; nModel<m_onlineModel[nfeatures].size(); nModel++) {
			dist = regionDist(m_onlineModel[nfeatures][nModel].m_covMatrix,f1->m_descp[nfeatures],tracker_param);
			aux = getAreaCenter(f1->m_rect, nfeatures);
			
			dist += tracker_param->alpha*sqrt(pow((double)(m_onlineModel[nfeatures][nModel].m_pos.x-aux.x),2)+
							   pow((double)(m_onlineModel[nfeatures][nModel].m_pos.y-aux.y),2));
			
			
			if (dist<minDist) minDist = dist;
		}
		totalDist += minDist;
	}	
	return totalDist;
}

double CovariancePatchModelv2::updateModelDistance(int indx, Mat& covMat){
	
	double dist = 0;//, minDist = 100000;
	
	for (int numSample=0; numSample<m_onlineModel[indx].size(); numSample++) {
		dist += regionDist(m_onlineModel[indx][numSample].m_covMatrix,covMat, tracker_param,MAT_MAP_LOG);
		//if (dist<minDist) minDist = dist;
	}
	
	return dist/m_onlineModel[indx].size();
	//return minDist;
}
/*
double CovariancePatchModelv2::updateModelDistance(int indx, CovariancePatchDescriptor* f1){
	
	double dist = 0 , alpha = 0.09;//, minDist = 100000;
	
	for (int numSample=0; numSample<m_onlineModel[indx].size(); numSample++) {
		dist += regionDist(m_onlineModel[indx][numSample].m_covMatrix,f1->m_descp[indx],tracker_param);
		dist += alpha*sqrt(pow(m_onlineModel[indx][numSample].m_pos.x- 
								((indx == 1 || indx == 3) ? 
								 f1->m_rect.center.x-f1->m_rect.size.width/4 : f1->m_rect.center.x+f1->m_rect.size.width/4),2)+
							pow(m_onlineModel[indx][numSample].m_pos.y- 
								((indx == 1 || indx == 2) ? 
								 f1->m_rect.center.y-f1->m_rect.size.height/4 : f1->m_rect.center.y+f1->m_rect.size.height/4),2));
	}
	return dist/m_onlineModel[indx].size();
}
*/

//
//
//

ModelFeature::ModelFeature(int indx, Mat& cov, double prob, Point2i pos){
	m_index = indx;
	m_probability = prob;
	m_covMatrix = cov;
	m_pos = pos;
}
ModelFeature::~ModelFeature(){
	m_covMatrix.release();
}

void ModelFeature::clear(){
	m_covMatrix.release();
}


