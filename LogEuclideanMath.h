#ifndef _LOGEUCLIDEANMATH_H
#define _LOGEUCLIDEANMATH_H

#include <OpenCV/OpenCV.h>
#include "OpenCV/cv.h"
#include "OpenCV/highgui.h"
#include "configStruct.h"
#include "saliency.h"

//#define NUM_FEATURES 12//10
//#define NUM_FEATURES_VECTOR 78 //(n*(n-1))/2 

#define MAT_MAP_LOG 1
#define MAT_COV		0

#define MAT_TYPE_UCHAR	0
#define MAT_TYPE_FLOAT	1
#define MAT_TYPE_DOUBLE 2
#define MAT_TYPE_UINT	3
#define MAT_TYPE_INT	4


using namespace cv;

//Calcula el mapa logaritmico de la matriz Crxy
Mat mapLogMat(const Mat& Crxy, config_SystemParameter *param);
Mat mapExpMat(const Mat& Crxy, config_SystemParameter *param);
Mat avgMat(const vector<Mat>& src, int flags, config_SystemParameter *param);
Mat avgLogMat(const vector<Mat>& src, int flags,config_SystemParameter *param);


//distancia entre regiones de covarianza
//los flags dicen si las matrices de input son mapas logaritmicos o matrices de covarianza
//MAT_COV
//MAT_MAP_LOG
double regionDist(Mat& Crxy1, Mat& Crxy2, config_SystemParameter *param, int flags=MAT_MAP_LOG);
double regionDist(const vector<Mat>& Crxy1, const vector<Mat>& Crxy2, config_SystemParameter *param, int flags=MAT_MAP_LOG);

/*PARA LA BUSQUEDA*/
//void makeF(const Mat& img, vector<Mat>& matF, config_SystemParameter *param);
void makeFgray(const Mat& img, vector<Mat>& F, config_SystemParameter *param);
void makeFrgb(const Mat& img, vector<Mat>& F, config_SystemParameter *param);

void makePQ(const vector<Mat>& F, vector<Mat>& P, vector< vector<Mat> >& Q, config_SystemParameter *param);
Mat makePxy(const vector<Mat>& P, int x, int y, config_SystemParameter *param);
Mat makeQxy(const vector< vector<Mat> >& Q, int x, int y, config_SystemParameter *param);

void CovarianceRegion(const vector<Mat>& P, const vector< vector<Mat> >& Q, Mat& Crxy, config_SystemParameter *param);
void CovarianceRegion(const vector<Mat>& P, const vector< vector<Mat> >& Q, Mat& Crxy, const Point2i& pointP1,const Point2i& pointP2, config_SystemParameter *param);


#endif
