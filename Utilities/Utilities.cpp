/*
 *  Utilities.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 29-12-09.
 *  Copyright 2009 PUC. All rights reserved.
 *
 */

#include "Utilities.h"



const CFIndex CASCADE_NAME_LEN = 2048;
char  CASCADE_NAME[CASCADE_NAME_LEN] = "./haarcascades/haarcascade_frontalface_alt.xml";
char  CASCADE_NAME_PROFILE[CASCADE_NAME_LEN] = "./haarcascades/haarcascade_profileface.xml";


void setSistemConfig(config_SystemParameter *param, string fileParam){
	
	//system param - Directory
	param->angleVector.clear();
	param->scaleVector.clear();
	
	//READING THE PATCH'S POSITION
	int length;
	char * buffer;
	
	ifstream is;
	is.open (fileParam.c_str(), ios::binary );
	if(!is.is_open()){
		cout << "ERROR OPEN CONFIG VIDEO FILE"<<endl;
		CV_Assert(false);
	}
	
	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);
	// allocate memory:
	buffer = new char [length];
	// read data as a block:
	is.read (buffer,length);
	
	string content(buffer,length);
	stringstream ss(content);
	string centerS,centerS_x,centerS_y,sizeS,sizeS_w,sizeS_h,angleS, 
	       norm,rgb,videoPath,
	       modelMemory,alpha,
	       areaSearch,angleVector,scaleVector,
		   time;
	
	getline(ss, time);
	getline(ss, centerS);
	getline(ss, sizeS);
	getline(ss, angleS);
	getline(ss, norm);
	getline(ss, videoPath);
	getline(ss, rgb);
	getline(ss, modelMemory);
	getline(ss, alpha);
	getline(ss, areaSearch);
	getline(ss, scaleVector);
	getline(ss, angleVector);
	
	//Time range
	param->startTimeOnFrames = isFrame(time.substr(0,time.find(',')));
	param->endTimeOnFrames = isFrame(time.substr(time.find(',')+1));
	
	param->startTime = time2msec(time.substr(0,time.find(',')));
	param->endTime = time2msec(time.substr(time.find(',')+1));
	
	//initialRect
	centerS_x = centerS.substr(0,centerS.find(','));
	centerS_y = centerS.substr(centerS.find(',')+1);
	sizeS_w = sizeS.substr(0,sizeS.find(','));
	sizeS_h = sizeS.substr(sizeS.find(',')+1);
	
	
	Point2f center(atof(centerS_x.c_str()),atof(centerS_y.c_str()));
	cv::Size size(atof(sizeS_w.c_str()),atof(sizeS_h.c_str()));
	
	param->initialRect = RotatedRect(center,size,atof(angleS.c_str()));
	
	//NormType
	if (!norm.compare("FACE")) param->normType = FACE;//*normType = FACE;
	else if(!norm.compare("PEDESTRIAN")) param->normType = PEDESTRIAN;//*normType = PEDESTRIAN;
	else {
		cout << "NOT SUCH NORM: "<<norm<<endl;
		CV_Assert(false);
	}
	
	//RGB
	if (!rgb.compare("1")){ 
		param->isRGB = true;
		param->numFeature = 12;
	}
	else if(!rgb.compare("0")){
		param->isRGB = false;
		param->numFeature = 10;
	}
	else {
		cout << "INVALID RGB VALUE: "<<rgb<<endl;
		CV_Assert(false);
	}
	
	//video path
	param->videoPath = videoPath;
	
	//modelMemory
	param->modelMemory = atof(modelMemory.c_str());
	//alpha
	param->alpha = atof(alpha.c_str());
	//areaSearch
	param->areaSearch = atof(areaSearch.c_str());
	
	//scaleVector
	int index = 0;
	string aux;
	while (index<scaleVector.length()) {
		aux = scaleVector.substr(index,scaleVector.length());
		aux = aux.substr(0,aux.find_first_of(','));
		index += aux.length()+1;
		param->scaleVector.push_back(atof(aux.c_str()));
	}
	//anglesVector
	index = 0;
	while (index<angleVector.length()) {
		aux = angleVector.substr(index,angleVector.length());
		aux = aux.substr(0,aux.find_first_of(','));
		index += aux.length()+1;
		param->angleVector.push_back(atof(aux.c_str()));
	}
	is.close();
	delete[] buffer;
}
bool checkConfig(config_SystemParameter *param, bool print){
	if(param->scaleVector.size()<1){ 
		cout << "BAD SIZE SCALE VECTOR PARAMETER"<<endl;
		return false;
	}
	if(param->angleVector.size()<1){ 
		cout << "BAD SIZE ANGLE VECTOR PARAMETER"<<endl;
		return false;
	}
	if(param->areaSearch<0){ 
		cout << "BAD VALUE AREA SEARCH PARAMETER"<<endl;
		return false;
	}
	if(param->modelMemory<=5){ 
		cout << "BAD VALUE MODEL MEMORY PARAMETER"<<endl;
		return false;
	}
	if(param->alpha<0){ 
		cout << "BAD VALUE ALPHA PARAMETER"<<endl;
		return false;
	}
	if(param->numFeature<=2){ 
		cout << "BAD VALUE NUM FEATURE PARAMETER"<<endl;
		return false;
	}
	if(param->videoPath.empty()){ 
		cout << "BAD VALUE VIDEO PATH PARAMETER"<<endl;
		return false;
	}
	if(param->normType!=FACE && param->normType!=PEDESTRIAN){ 
		cout << "BAD VALUE NORM TYPE PARAMETER"<<endl;
		return false;
	}
	if(param->startTime>=param->endTime && param->endTime<=0 && param->startTime<=0){ 
		cout << "BAD VALUE NORM TYPE PARAMETER"<<endl;
		return false;
	}
	
	
	if(print){
		cout << "SISTEM PARAMETER"<<endl;
		cout << "Start Video msec: "<<param->startTime<<endl;
		cout << "End Video msec: "<<param->endTime<<endl;
		cout << "Video Path: "<<param->videoPath<<endl;
		cout << "Images Color: ";
		if(param->isRGB) cout<<"RGB"<<endl;
		else cout<<"BW"<<endl;
		cout<<"Norm type: ";
		if(param->normType==FACE) cout<<"FACE"<<endl;
		else cout<<"PEDESTRIAN"<<endl;
		cout << "Num Features (F matrix): "<<param->numFeature<<endl;
		
		cout <<endl<<"TRACKER PARAMETER"<<endl;
		cout << "Area search (beta): "<<param->areaSearch<<endl;
		cout << "Scale vector: ";
		for (int i=0; i<param->scaleVector.size(); i++) {
			cout << param->scaleVector[i]<<" ";
		}
		cout << endl<<"Angle vector: ";
		for (int i=0; i<param->angleVector.size(); i++) {
			cout << param->angleVector[i]<<" ";
		}
		cout << endl << "Initial Rect- center.x: "<<param->initialRect.center.x<<" center.y: "<<param->initialRect.center.y<<
			 " size.width "<<param->initialRect.size.width<<" size.height "<<param->initialRect.size.height<<endl;
		
		cout <<endl<<"MODEL PARAMETER"<<endl;
		cout << "Alpha: "<<param->alpha<<endl;
		cout << "Model Memory: "<<param->modelMemory<<endl<<endl;
		
	}
	
	return true;
}

bool readConfigAppCompare(string _path, frameObject *cap, config_SystemParameter *param){
	//READING THE PATCH'S POSITION
	int length;
	char *buffer;
	
	ifstream is;
	is.open (_path.c_str(), ios::binary );
	if(!is.is_open()){
		cout << "ERROR OPEN CONFIG VIDEO FILE: "<<_path<<endl;
		return false;
	}
	
	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);
	// allocate memory:
	buffer = new char [length];
	// read data as a block:
	is.read (buffer,length);
	
	string content(buffer,length);
	stringstream ss(content);
	
	string debug, videoPath, videoConfig;
	
	getline(ss, videoPath);
	getline(ss, videoConfig);
	getline(ss, debug);
	
	if(!cap->iniCap(videoPath)){
		cout << "WRONG PATH video: "<< videoPath <<endl;
		return false;
	}
	cout <<"VIDEO PATH: "<<videoPath<<endl;
	
	//loading parameter from file
	setSistemConfig(param, videoConfig);
	//deleting old video files
	deleteOldVideo(*param);
	//revisando parametros
	if(!checkConfig(param,true)){
		cout << "WRONG PATH videoConfig:"<< videoConfig<<endl;
		return false;
	}
	cout <<"VIDEO CONFIG PATH: "<<videoConfig<<endl;
	
	if (!debug.compare("1")) param->debugMode = true;
	else if(!debug.compare("0")) param->debugMode = false;
	else {
		cout << "INVALID DEBUG VALUE: "<<debug<<endl;
		return false;
	}
	return true;
}
bool readConfigAppQueue(string _path, frameObject cap[], config_SystemParameter param[]){
	//READING THE PATCH'S POSITION
	int length;
	char *buffer;
	
	ifstream is;
	is.open (_path.c_str(), ios::binary );
	if(!is.is_open()){
		cout << "ERROR OPEN CONFIG VIDEO FILE: "<<_path<<endl;
		return false;
	}
	
	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);
	// allocate memory:
	buffer = new char [length];
	// read data as a block:
	is.read (buffer,length);
	
	string content(buffer,length);
	stringstream ss(content);
	
	string debug, videoPath_L, videoConfig_L, videoPath_T, videoConfig_T;
	
	getline(ss, videoPath_L);
	getline(ss, videoPath_T);
	getline(ss, videoConfig_L);
	getline(ss, videoConfig_T);
	getline(ss, debug);
	
	if(!cap[0].iniCap(videoPath_L)){
		cout << "WRONG LATERAL PATH video: "<< videoPath_L <<endl;
		return false;
	}
	cout <<"VIDEO LATERAL PATH: "<<videoPath_L<<endl;
	
	if(!cap[1].iniCap(videoPath_T)){
		cout << "WRONG TOP PATH video: "<< videoPath_T<<endl;
		return false;
	}
	cout <<"VIDEO TOP PATH: "<<videoPath_T<<endl;
	
	
	//loading parameter from file
	setSistemConfig(&param[0], videoConfig_L);
	setSistemConfig(&param[1], videoConfig_T);
	//deleting old video files
	deleteOldVideo(param[0]);
	deleteOldVideo(param[1]);
	//revisando parametros
	if(!checkConfig(&param[0],true)){
		cout << "WRONG LATERAL PATH videoConfig:"<< videoConfig_L<<endl;
		return false;
	}
	cout <<"VIDEO LATERAL CONFIG PATH: "<<videoConfig_L<<endl;
	
	if(!checkConfig(&param[1],true)){
		cout << "WRONG LATERAL PATH videoConfig:"<< videoConfig_T<<endl;
		return false;
	}
	cout <<"VIDEO LATERAL CONFIG PATH: "<<videoConfig_T<<endl;
	
	
	if (!debug.compare("1")){
		param[0].debugMode = true;
		param[1].debugMode = true;
	}
	else if(!debug.compare("0")){
		param[0].debugMode = false;
		param[1].debugMode = false;
	}
	else {
		cout << "INVALID DEBUG VALUE: "<<debug<<endl;
		return false;
	}
	return true;
}


void deleteOldVideo(config_SystemParameter param){
	//borrando archivo video.avi para no crear archivo
	//muy grandes
	string videoFile = param.videoPath+".avi";
	if( remove( videoFile.c_str() ) == 0 )
		cout << "Archivo video.avi existia, fue eliminado"<<endl;
}

vector<RotatedRect> getComparison(string dir){
	
	vector<RotatedRect> rect;
	
	DIR *dp;
	//struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL) {
		cout << "Error" << endl;
		CV_Assert(false);
	}
	
	//READING THE PATCH'S POSITION
	int length;
	char * buffer;
	
	ifstream is;
	string iniFile = dir+ "/comparisonCoord.txt";
	
	is.open (iniFile.c_str(), ios::binary );
	
	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);
	// allocate memory:
	buffer = new char [length];
	// read data as a block:
	is.read (buffer,length);
	
	string content(buffer,length);
	stringstream ss(content);
	string sline;
	RotatedRect auxRect(Point2i(0,0), cv::Size(1,1), 0);
	
	while (getline(ss, sline)) {
		
		auxRect.center.x = atof(sline.substr(0,sline.find(',')).c_str());
		sline = sline.substr(sline.find(',')+1);
		auxRect.center.y = atof(sline.substr(0,sline.find(',')).c_str());
		sline = sline.substr(sline.find(',')+1);
		auxRect.size.width = atof(sline.substr(0,sline.find(',')).c_str());
		sline = sline.substr(sline.find(',')+1);
		auxRect.size.height = atof(sline.substr(0,sline.find(',')).c_str());

		auxRect.center.x += auxRect.size.width/2;
		auxRect.center.y += auxRect.size.height/2;
		
		rect.push_back(auxRect);
	}
	is.close();
	delete[] buffer;
	

	
	return rect;
}

void drawLegend(vector<string> texts, vector<CvScalar> colorLine, Mat& img){
	
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	int thickness = 1;
	int baseline=0;
	int prevTextHeight = 0;
	
	int maxWidth = 0, totalHeight = 0, firstBaseLine = 0;
	//encontrando max y min para el rectangulo
	for (int i=0; i<texts.size(); i++) {
		baseline=0;
		cv::Size textSize = getTextSize(texts[i], fontFace,	fontScale, thickness, &baseline);
		totalHeight += textSize.height;
		if (maxWidth<textSize.width) maxWidth = textSize.width; 
		if (i==0) firstBaseLine = textSize.height+thickness;
	}
	
	maxWidth += 40;
	
	Point2i RectOrg(img.cols - maxWidth, img.rows - totalHeight);
	
	rectangle(img, Point2i(RectOrg.x, RectOrg.y-firstBaseLine), Point2i(RectOrg.x+maxWidth, RectOrg.y+totalHeight) , Scalar::all(255),CV_FILLED);
	
	for (int i=0; i<texts.size(); i++) {
		baseline=0;
		cv::Size textSize = getTextSize(texts[i], fontFace,	fontScale, thickness, &baseline);
		baseline += thickness;
		Point2i textOrg(RectOrg.x+40, RectOrg.y+prevTextHeight);
		line(img, textOrg + Point2i(-35, thickness-6), textOrg + Point2i(-10, thickness-6), colorLine[i],3);
		putText(img, texts[i], textOrg, fontFace, fontScale, Scalar::all(0), thickness, 8);
		prevTextHeight += textSize.height+3;
	}
	
}

RotatedRect doRect(double *conf,const double scale, const RotatedRect rect){
	
	Point2f center(conf[0]*scale,conf[1]*scale);
	Size2f sizeB(rect.size.width*conf[4]*scale,rect.size.height*conf[4]*scale);
	
	return RotatedRect(center,sizeB,conf[6]);
}

void printMat(const Mat& src, int flag){
	
	int cols = src.cols;
	int rows = src.rows;
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j <cols; j++){
			if (flag==MAT_TYPE_DOUBLE)
				cout << (double)src.at<double>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_FLOAT)
				cout << (float)src.at<float>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_UCHAR)
				cout << (int)src.at<uchar>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_UINT)
				cout << (uint)src.at<uint>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_INT)
				cout << (int)src.at<int>(i, j)<<" _ ";
			else
				cout << (double)src.at<double>(i, j)<<" _ ";
			
		}
		cout << endl;
	}
	cout << endl;
}

void printMat(const Mat& src, Point2i p1, Point2i p2, int flag){
	
	//int cols = src.cols;
	//int rows = src.rows;
	for(int i = p1.y; i <= p2.y/*rows*/; i++)
	{
		for(int j = p1.x; j <= p2.x/*cols*/; j++){
			if (flag==MAT_TYPE_DOUBLE)
				cout << (double)src.at<double>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_FLOAT)
				cout << (float)src.at<float>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_UCHAR)
				cout << (int)src.at<uchar>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_UINT)
				cout << (uint)src.at<uint>(i, j)<<" _ ";
			else if(flag==MAT_TYPE_INT)
				cout << (int)src.at<int>(i, j)<<" _ ";
			else
				cout << (double)src.at<double>(i, j)<<" _ ";
			
		}
		cout << endl;
	}
	cout << endl;
}

void printMat(const vector<Mat>& src, int flag){
	
	int cols = src[0].cols;
	int rows = src[0].rows;
	
	for (int k=0; k<src.size(); k++) {
		for(int i = 0; i < rows; i++)
		{
			for(int j = 0; j < cols; j++){
				if (flag==MAT_TYPE_DOUBLE)
					cout << (double)src[k].at<double>(i, j)<<" _ ";
				else if(flag==MAT_TYPE_FLOAT)
					cout << (float)src[k].at<float>(i, j)<<" _ ";
				else if(flag==MAT_TYPE_UCHAR)
					cout << (int)src[k].at<uchar>(i, j)<<" _ ";
				else
					cout << (double)src[k].at<double>(i, j)<<" _ ";
			}
			cout << endl;
		}
		cout << endl;
	}
	cout << endl;
	
}

void drawRotatedRect(Mat& img, const RotatedRect& rect, CvScalar color, int thickness){
	
	if (abs(rect.angle)<1) {
		rectangle(img, Point2i(rect.center.x-rect.size.width/2,rect.center.y-rect.size.height/2), 
				  Point2i(rect.center.x+rect.size.width/2,rect.center.y+rect.size.height/2), color, thickness);
	}
	else {
		
		RotatedRect box(rect.center,rect.size,-rect.angle+90);
		CvPoint2D32f boxPoints[4];
		
		cvBoxPoints(CvBox2D(box), boxPoints);
		line(img,Point2f((int)boxPoints[0].x, (int)boxPoints[0].y), Point2f((int)boxPoints[1].x, (int)boxPoints[1].y),color,thickness);
		line(img,Point2f((int)boxPoints[1].x, (int)boxPoints[1].y), Point2f((int)boxPoints[2].x, (int)boxPoints[2].y),color,thickness);
		line(img,Point2f((int)boxPoints[2].x, (int)boxPoints[2].y), Point2f((int)boxPoints[3].x, (int)boxPoints[3].y),color,thickness);
		line(img,Point2f((int)boxPoints[3].x, (int)boxPoints[3].y), Point2f((int)boxPoints[0].x, (int)boxPoints[0].y),color,thickness);
	}

	
}

RotatedRect scaleRect(const RotatedRect rect, const double scale){
	return RotatedRect(Point2f(rect.center.x*scale,rect.center.y*scale), cv::Size(rect.size.width*scale,rect.size.height*scale), rect.angle);
}

RotatedRect scaleRect(const RotatedRect rect, const cv::Size2f scale){
	return RotatedRect(Point2f(rect.center.x*scale.width,rect.center.y*scale.height), 
					   cv::Size(rect.size.width*scale.width,rect.size.height*scale.height), 
					   rect.angle);
}

string CreatefileNameString(){
	std::stringstream out;
	out << rand();
	return out.str()+".jpeg";
}

//el indx dice que zona es, indx = 1...4
Point2f getAreaCenter(RotatedRect& rect, int indx){
	
	Point2f aux(0,0);
	if (abs(rect.angle)<1) {
		aux.x = ((indx == 1 || indx == 3) ? rect.center.x-rect.size.width/4 : rect.center.x+rect.size.width/4);
		aux.y = ((indx == 1 || indx == 2) ? rect.center.y-rect.size.height/4 : rect.center.y+rect.size.height/4);
	}
	else {
		CvPoint2D32f boxPoints[4];
		RotatedRect box2(rect.center,cv::Size(rect.size.width/2,rect.size.height/2),-rect.angle+90);
		cvBoxPoints(CvBox2D(box2), boxPoints);
		if (indx == 1) {
			aux.x = (double)boxPoints[1].x;
			aux.y = (double)boxPoints[1].y;
		}
		else if(indx == 2){
			aux.x = (double)boxPoints[2].x;
			aux.y = (double)boxPoints[2].y;
		}
		else if(indx == 3){
			aux.x = (double)boxPoints[0].x;
			aux.y = (double)boxPoints[0].y;
		}
		else if(indx == 4){
			aux.x = (double)boxPoints[3].x;
			aux.y = (double)boxPoints[3].y;
		}
		else
			CV_Assert(false);
	}
	
	return aux;
}
cv::Rect getBoundingRect(RotatedRect rect){
	rect.angle = rect.angle+90;
    return rect.boundingRect();
}

bool searchFace(const Mat& src, RotatedRect rect){
	
	
	CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*) cvLoad (CASCADE_NAME, 0, 0, 0);
    CvMemStorage* storage = cvCreateMemStorage(0);
    assert (storage);
	if (! cascade)
        abort ();
	
	CvHaarClassifierCascade* cascadeProfile = (CvHaarClassifierCascade*) cvLoad (CASCADE_NAME_PROFILE, 0, 0, 0);
    CvMemStorage* storageProfile = cvCreateMemStorage(0);
    assert (storageProfile);
	if (! cascadeProfile)
        abort ();
	
	IplImage *gray_image = cvCreateImage(src.size(), IPL_DEPTH_8U, 1);
	IplImage aux = IplImage(src);
	
	rect.size.width *= 1.5;
	rect.size.height *= 1.5;
	
	cvCvtColor (&aux, gray_image, CV_BGR2GRAY);
	cvEqualizeHist( gray_image, gray_image );
	cvSetImageROI(gray_image, getBoundingRect(rect));
	
	CvSeq* faces = cvHaarDetectObjects (gray_image, cascade, storage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, cvSize (10, 10));
	CvSeq* facesProfiles = cvHaarDetectObjects (gray_image, cascadeProfile, storageProfile, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, cvSize (10, 10));
	
	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* r = (CvRect*) cvGetSeqElem (faces, i);
		
		CvPoint center;
		int radius;
		center.x = cvRound((r->width*0.5 + r->x));
		center.y = cvRound((r->y + r->height*0.5));
		radius = cvRound((r->width + r->height)*0.25);
		cvCircle (gray_image, center, radius, CV_RGB(0,255,0), 3, 8, 0 );

	}
	
	for (int i = 0; i < (facesProfiles ? facesProfiles->total : 0); i++){
		CvRect* r = (CvRect*) cvGetSeqElem (facesProfiles, i);
		CvPoint center;
		int radius;
		center.x = cvRound((r->width*0.5 + r->x));
		center.y = cvRound((r->y + r->height*0.5));
		radius = cvRound((r->width + r->height)*0.25);
		cvCircle (gray_image, center, radius, CV_RGB(0,255,0), 3, 2, 0 );
	}
	
	
	cvNamedWindow("ROI");
	imshow( "ROI", gray_image);
	//cvWaitKey();
	
	cvResetImageROI(gray_image);
	cvReleaseImage(&gray_image);
	
	cvClearMemStorage(storage);
	
	if (faces->total>0 || facesProfiles->total>0)return true;
	else return false;
}

RotatedRect searchFace(Mat& src, GenericModel *model, cv::Size2f scaleFactor, bool draw){
	
	GenericFeature *minFeature;
	Mat auxImg, auxImg2;
	resize(src, auxImg,cv::Size2i(scaleFactor.width*src.size().width, scaleFactor.height*src.size().height));
	auxImg2 = auxImg.clone();
	
	CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*) cvLoad (CASCADE_NAME, 0, 0, 0);
    CvMemStorage* storage = cvCreateMemStorage(0);
    assert (storage);
	if (! cascade)
        abort ();
	
	CvHaarClassifierCascade* cascadeProfile = (CvHaarClassifierCascade*) cvLoad (CASCADE_NAME_PROFILE, 0, 0, 0);
    CvMemStorage* storageProfile = cvCreateMemStorage(0);
    assert (storageProfile);
	if (! cascadeProfile)
        abort ();
	
	IplImage *gray_image = cvCreateImage(src.size(), IPL_DEPTH_8U, 1);
	IplImage aux = IplImage(src);
	
	cvCvtColor (&aux, gray_image, CV_BGR2GRAY);
	cvEqualizeHist( gray_image, gray_image );
	
	CvSeq* faces = cvHaarDetectObjects (gray_image, cascade, storage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, cvSize (25, 25));
	CvSeq* facesProfiles = cvHaarDetectObjects (gray_image, cascadeProfile, storageProfile, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, cvSize (25, 25));
	
	double minValue = 10000.0;
	RotatedRect minRect;
	
	model->updateModel(auxImg);
	if (draw) cvNamedWindow("ROI");
	
	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* r = (CvRect*) cvGetSeqElem (faces, i);
		RotatedRect auxRect(Point2i(r->x+r->width/2,r->y+r->height/2),Size2i(r->width,r->height),0);
		auxRect = scaleRect(auxRect, cv::Size2f(scaleFactor.width, scaleFactor.height));
		if (draw) drawRotatedRect(auxImg2, auxRect,CV_RGB(100,50,50) , 2);
		
		
		if(model->ModelType == COV_FULL_IMAGE){
			//minFeature = (GenericFeature *)new CovarianceFullDescriptor(auxRect,model->tracker_param);
			CV_Assert(false);
		}
		else if(model->ModelType == COV_SUB_WINDOWS)
			minFeature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,model->tracker_param);
		else if(model->ModelType == COV_SUB_WINDOWS_B)
			minFeature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,model->tracker_param);
		
		minFeature->computeFeature(model);
		double dist = model->distance(minFeature);
		
		if (dist<minValue) {
			minValue = dist;
			minRect = auxRect;
		}
		
		minFeature->clear();
		delete minFeature;
		if (draw){
			cout << "dist: "<<dist<<endl;
			imshow( "ROI", auxImg2);
			cvWaitKey();
		}
		
	}
	
	for (int i = 0; i < (facesProfiles ? facesProfiles->total : 0); i++){
		CvRect* r = (CvRect*) cvGetSeqElem (facesProfiles, i);
		RotatedRect auxRect(Point2i(r->x+r->width/2,r->y+r->height/2),Size2i(r->width,r->height),0);
		auxRect = scaleRect(auxRect, cv::Size2f(scaleFactor.width, scaleFactor.height));
		if (draw) drawRotatedRect(auxImg2, auxRect,CV_RGB(0,0,0) , 2);
		
		if(model->ModelType == COV_FULL_IMAGE){
			//minFeature = (GenericFeature *)new CovarianceFullDescriptor(auxRect,model->tracker_param);
			CV_Assert(false);
		}
		else if(model->ModelType == COV_SUB_WINDOWS)
			minFeature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,model->tracker_param);
		else if(model->ModelType == COV_SUB_WINDOWS_B)
			minFeature = (GenericFeature *)new CovariancePatchDescriptor(auxRect,model->tracker_param);
		
		minFeature->computeFeature(model);
		double dist = model->distance(minFeature);
		
		
		if (dist<minValue) {
			minValue = dist;
			minRect = auxRect;
		}
		
		minFeature->clear();
		delete minFeature;
		if (draw){
			cout << "dist: "<<dist<<endl;
			imshow( "ROI", auxImg2);
			cvWaitKey();
		}	
	}	
	
	
	if (draw){
		drawRotatedRect(auxImg2, minRect,CV_RGB(255,0,0) , 3);	
		imshow( "ROI", auxImg2);
		cvWaitKey();
		cvDestroyWindow("ROI");
	}
	auxImg2.release();
	auxImg.release();
	
	cvReleaseImage(&gray_image);
	
	cvClearMemStorage(storage);
	cvClearMemStorage(storageProfile);
	
	return scaleRect(minRect, cv::Size2f(1/scaleFactor.width, 1/scaleFactor.height));	
}

bool isFrame(string timeString){
	if (timeString.find(":")==string::npos)
		return true;
	else
		return false;
}

double time2msec(string timeString){
	if (isFrame(timeString))
		return atoi(timeString.c_str());
	
	string hrsSting = "0",minSting = "0",secSting = "0";
	
	if(timeString.find_first_of(':')==timeString.find_last_of(':')){
		minSting = timeString.substr(0,timeString.find_first_of(':'));
		secSting = timeString.substr(timeString.find_last_of(':')+1);
	}
	else {
		hrsSting = timeString.substr(0,timeString.find_first_of(':'));
		minSting = timeString.substr(timeString.find_first_of(':')+1,
									 timeString.find_last_of(':')-timeString.find_first_of(':')-1);
		secSting = timeString.substr(timeString.find_last_of(':')+1);
	}
	
	return atoi(hrsSting.c_str())*3600000+atoi(minSting.c_str())*60000+atof(secSting.c_str())*1000;
}
