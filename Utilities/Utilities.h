/*
 *  Utilities.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 29-12-09.
 *  Copyright 2009 PUC. All rights reserved.
 *
 */

#ifndef _UTILITIES_H
#define _UTILITIES_H


#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "LogEuclideanMath.h"
#include "configStruct.h"

//arreglar las referencias
#include "CovariancePatch.h"
#include "Generic.h"


void setSistemConfig(config_SystemParameter *param, string fileParam);
bool checkConfig(config_SystemParameter *param, bool print);

void deleteOldVideo(config_SystemParameter param);

vector<RotatedRect> getComparison(string dir);

RotatedRect doRect(double *conf, const double scale, const RotatedRect rect);

void printMat(const Mat& src, int flag);

void printMat(const Mat& src, Point2i p1, Point2i p2, int flag);

void printMat(const vector<Mat>& src, int flag);

void drawRotatedRect(Mat& img, const RotatedRect& rect, CvScalar color = cvScalar(0,255,0), int thickness = 2);

void drawLegend(vector<string> texts, vector<CvScalar> colorLine, Mat& img);

RotatedRect scaleRect(const RotatedRect rect, const double scale);
RotatedRect scaleRect(const RotatedRect rect, const cv::Size2f scale);

string CreatefileNameString();

inline float sigmoid(float x){
	return 1.0f/(1.0f+exp(-x));
};
Point2f getAreaCenter(RotatedRect& rect, int indx);

cv::Rect getBoundingRect(RotatedRect rect);

bool searchFace(const Mat& src, RotatedRect rect);
RotatedRect searchFace(Mat& src, GenericModel *model, cv::Size2f scaleFactor, bool draw);

//se podria dejer como un int o unsigned int, pero la funcion opencv acepta doubles
//como son milisegundos me aseguro tener un max value grande con el long
double time2msec(const char _time[]);
#endif