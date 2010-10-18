/*
 *  GenericParticleFilter.h
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 16-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */
#ifndef _GENERICPARTICLEFILTER_H
#define _GENERICPARTICLEFILTER_H


// *****************************************************************
// defines
// *****************************************************************

#define DIMENSION_2D	8
#include <OpenCV/OpenCV.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include "CovarianceDescriptor.h"
//#include "TrackingModel.h"
#include "Utilities.h"

#include "Generic.h"
#include "CovarianceFull.h"
#include "CovariancePatch.h"

// *****************************************************************
// ParticleFilter
// *****************************************************************

static const double Condense_PI = 3.14159265358979323846;

class GenericParticleFilter
{
public:
	
	Mat m_imgSearch;
	
	struct Square2D
	{
		int x, y;
		//scale
		double k;
		//rotation angle
		double a;
		// width/height original de la region
		int oWidth;
		int oHeight;
	};
	
	// constructor
	GenericParticleFilter(int nParticles, GenericModel *model, Mat& iniImg, int fps);
	
	// destructor
	~GenericParticleFilter();
	
	
	// public methods
	double ParticleFilter(double *pResultMeanConfiguration, double dSigmaFactor = 1);
	double CalculateProbabilityForConfiguration(const double *pConfiguration);
	void GetConfiguration(double *pBestConfiguration, double dMeanFactor);
	void GetBestConfiguration(double *pBestConfiguration);
	void GetMeanConfiguration(double *pMeanConfiguration);
	void GetPredictedConfiguration(double *pPredictedConfiguration);
	
	void InitParticles(RotatedRect& box, int scale);
	void SetImage(Mat& img);
	
	// member access
	//vector<Mat> m_in;
	//Mat m_descpO;
	//vector<Mat> m_out;
	//Mat m_descpS;
	
	GenericModel* m_TrackingModel;
	
	Square2D model;
	
	static double uniform_random()
	{
		return rand() / double(RAND_MAX);
	}
	
	static double gaussian_random()
	{
		static int next_gaussian = 0;
		static double saved_gaussian_value;
		
		double fac, rsq, v1, v2;
		
		if (next_gaussian == 0)
		{
			do
			{
				v1 = 2.0 * uniform_random() - 1.0;
				v2 = 2.0 * uniform_random() - 1.0;
				rsq = v1 * v1 + v2 * v2;
			}
			while (rsq >= 1.0 || rsq == 0.0);
			
			fac = sqrt(-2.0 * log(rsq) / rsq);
			saved_gaussian_value = v1 * fac;
			next_gaussian = 1;
			
			return v2 * fac;
		}
		else
		{
			next_gaussian = 0;
			return saved_gaussian_value;
		}
	}
	
	
protected:
	// protected methods
	int PickBaseSample();
	void CalculateMean();
	
	// virtual methods (framework methods to be implemented: design pattern "framwork" with "template methods")
	void UpdateModel(int nParticle);
	void PredictNewBases(double dSigmaFactor);
	double CalculateProbability(bool bSeparateCall = true);
	void CalculateFinalProbabilities() { }
	
	
	// protected attributes
	double *mean_configuration;
	double *last_configuration;
	
	double *sigma;
	double *lower_limit;
	double *upper_limit;
	
	// particle related attributes
	int m_nParticles;
	double c_total;
	double **s;
	double **s_temp;
	double *c;
	double *pi;
	double *temp;
	
	
private:
	
	int width, height;
	int m_typeNorm;
	int m_fps;
	
	GenericFeature *m_feature;
	
	//CvHaarClassifierCascade* cascadeFront;
	//CvHaarClassifierCascade* cascadeProfile;
    //CvMemStorage* storageFront;
	//CvMemStorage* storageProfile;
};



#endif
