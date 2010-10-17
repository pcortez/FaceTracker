/*
 *  GenericParticleFilter.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 16-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "GenericParticleFilter.h"
#include <iostream>

using namespace std;


// *****************************************************************
// Constructor
// *****************************************************************

GenericParticleFilter::GenericParticleFilter(int nParticles, GenericModel *p_TrackingModel, Mat& iniImg, int fps)
{
	this->m_nParticles = nParticles;
    
	mean_configuration = new double[DIMENSION_2D];
	last_configuration = new double[DIMENSION_2D];
	
	sigma = new double[DIMENSION_2D];
	lower_limit = new double[DIMENSION_2D];
	upper_limit = new double[DIMENSION_2D];
	
	int i;
	
	// allocate memory
	s = new double*[nParticles];
	for (i = 0; i < nParticles; i++)
		s[i] = new double[DIMENSION_2D];
	
	s_temp = new double*[nParticles];
	for (i = 0; i < nParticles; i++)
		s_temp[i] = new double[DIMENSION_2D];
	
	c = new double[nParticles];
	pi = new double[nParticles];
	
	temp = new double[DIMENSION_2D];
	
	
	m_TrackingModel = p_TrackingModel;
	
	model.oWidth	= p_TrackingModel->m_first_rect.size.width;
	model.oHeight	= p_TrackingModel->m_first_rect.size.height;
	model.a			= p_TrackingModel->m_first_rect.angle;
	
	//esta hecho asi y no con la imagen del modelo pke puede que la imagen no
	//sea toda
	this->width = iniImg.size().width;
	this->height = iniImg.size().height;
	this->m_fps = fps;
	
	
	InitParticles(p_TrackingModel->m_first_rect, 1);
	
}


// *****************************************************************
// Destructor
// *****************************************************************

GenericParticleFilter::~GenericParticleFilter()
{
	delete [] mean_configuration;
	delete [] last_configuration;
	delete [] sigma;
	delete [] lower_limit;
	delete [] upper_limit;
	
	int i;
	
	for (i = 0; i < m_nParticles; i++)
		delete [] s[i];
	delete [] s;
	
	for (i = 0; i < m_nParticles; i++)
		delete [] s_temp[i];
	delete [] s_temp;
	
	delete [] c;
	delete [] pi;
	
	delete [] temp;
}


// **************************************************************
// GetBestConfiguration
// **************************************************************

void GenericParticleFilter::GetConfiguration(double *pBestConfiguration, double dMeanFactor)
{
	double mean = 0;
	int nConfigurations = 0, i;
	
	for (i = 0; i < DIMENSION_2D; i++)
		pBestConfiguration[i] = 0;
	
	for (i = 0; i < m_nParticles; i++)
		mean += pi[i];
	
	mean /= m_nParticles * dMeanFactor;
	
	for (i = 0; i < m_nParticles; i++)
		if (pi[i] > mean)
		{
			for (int j = 0; j < DIMENSION_2D; j++)
				pBestConfiguration[j] += s[i][j];
			
			nConfigurations++;
		}
	
	if (nConfigurations > 0)
	{
		for (i = 0; i < DIMENSION_2D; i++)
			pBestConfiguration[i] /= nConfigurations;
	}
}


// **************************************************************
// GetBestConfiguration
// **************************************************************

void GenericParticleFilter::GetBestConfiguration(double *pBestConfiguration)
{
	double max = DBL_MIN;
	int best_i, i;
	
	for (i = 0; i < m_nParticles; i++)
		if (pi[i] > max)
		{
			best_i = i;
			max = pi[i];
		}
	
	
	for (i = 0; i < DIMENSION_2D; i++)
		pBestConfiguration[i] = s[best_i][i];
}


// **************************************************************
// GetMeanConfiguration
// **************************************************************

void GenericParticleFilter::GetMeanConfiguration(double *pMeanConfiguration)
{
	for (int i = 0; i < DIMENSION_2D; i++)
		pMeanConfiguration[i] = mean_configuration[i];
}


// **************************************************************
// GetPredictedConfiguration
// **************************************************************

void GenericParticleFilter::GetPredictedConfiguration(double *pPredictedConfiguration)
{
	for (int i = 0; i < DIMENSION_2D; i++)
		pPredictedConfiguration[i] = mean_configuration[i] + 0.8 * (mean_configuration[i] - last_configuration[i]);
}


// **************************************************************
// PickBaseSample
// **************************************************************

int GenericParticleFilter::PickBaseSample()
{
	double choice = uniform_random() * c_total;
	
	int low = 0;
	int high = m_nParticles;
	
	while (high > (low + 1))
	{
		int middle = (high + low) / 2;
		
		if (choice > c[middle])
			low = middle;
		else
			high = middle;
	}
	
	return low;
}


// **************************************************************
// GenericParticleFilter
// **************************************************************

double GenericParticleFilter::ParticleFilter(double *pResultMeanConfiguration, double dSigmaFactor)
{
	// push previous state through process model
	// use dynamic model and add noise
	PredictNewBases(dSigmaFactor);
	
	// apply bayesian measurement weighting
	c_total = 0;
	int i;
	for (i = 0; i < m_nParticles; i++)
	{
		// update model (calculate forward kinematics)
		UpdateModel(i);
		// evaluate likelihood function (compare edges,...)
		pi[i] = CalculateProbability(false);
	}
	
	CalculateFinalProbabilities();
	for (i = 0; i < m_nParticles; i++)
	{
		c[i] = c_total;
		c_total += pi[i];
	}
	
	// normalize
	const double factor = 1 / c_total;
	for (i = 0; i < m_nParticles; i++)
		pi[i] *= factor;
	
	CalculateMean();
	
	//GetMeanConfiguration(pResultMeanConfiguration);
	//GetBestConfiguration(pResultMeanConfiguration);
	GetPredictedConfiguration(pResultMeanConfiguration);
	
	if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS_B){
		Point2f center(pResultMeanConfiguration[0],pResultMeanConfiguration[1]);
		Size2f sizeB(model.oWidth *pResultMeanConfiguration[4],model.oHeight*pResultMeanConfiguration[4]);
		RotatedRect r_aux(center,sizeB,pResultMeanConfiguration[6]);
		GenericFeature* faux = (GenericFeature*) new CovariancePatchDescriptor(r_aux,this->m_TrackingModel->tracker_param);
		faux->computeFeature(this->m_TrackingModel);
	}
	
	return CalculateProbabilityForConfiguration(pResultMeanConfiguration);
}


// **************************************************************
// CalculateMean
// **************************************************************

void GenericParticleFilter::CalculateMean()
{
	int i;
	//double max = -1;
	
	for (i = 0; i < DIMENSION_2D; i++)
	{
		last_configuration[i] = mean_configuration[i];
		mean_configuration[i] = 0;
	}
	
	for (i = 0; i < m_nParticles; i++)
	{
		for (int j = 0; j < DIMENSION_2D; j++)
			mean_configuration[j] += pi[i] * s[i][j];
	}
}

double GenericParticleFilter::CalculateProbabilityForConfiguration(const double *pConfiguration)
{
	// little "trick" for calculating the probability for the mean configuration
	// without introducing a new virtual function by using s[0]
	
	int i;
	
	for (i = 0; i < DIMENSION_2D; i++)
	{
		temp[i] = s[0][i];
		s[0][i] = pConfiguration[i];
	}
	
	UpdateModel(0);
	
	for (i = 0; i < DIMENSION_2D; i++)
		s[0][i] = temp[i];
	
	const double dResult = CalculateProbability(true);
	
	for (i = 0; i < DIMENSION_2D; i++)
		s[0][i] = temp[i];
	
	return dResult;
}

void GenericParticleFilter::UpdateModel(int nParticle)
{
	model.x = int(s[nParticle][0] + 0.5);
	model.y = int(s[nParticle][1] + 0.5);
	model.k = s[nParticle][4];
	model.a = s[nParticle][6];
	
	
	Point2f center(s[nParticle][0],s[nParticle][1]);
	Size2f sizeB(model.oWidth*s[nParticle][4],model.oHeight*s[nParticle][4]);
	RotatedRect boxR(center, sizeB,  s[nParticle][6]);
	
	/*
	 cout << "nParticle: "<<nParticle<<endl;
	 cout << "s[0]: "<<s[nParticle][0]<<endl;
	 cout << "s[1]: "<<s[nParticle][1]<<endl;
	 cout << "s[4]: "<<s[nParticle][4]<<endl;
	 cout << "s[6]: "<<s[nParticle][6]<<endl;
	 cout <<endl;
	*/
	 	
	
	if (this->m_TrackingModel->ModelType == COV_FULL_IMAGE)
		m_feature = (GenericFeature *)new CovarianceFullDescriptor(boxR, this->m_TrackingModel->tracker_param);
	else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS)
		m_feature = (GenericFeature *)new CovariancePatchDescriptor(boxR, this->m_TrackingModel->tracker_param);
	else if(this->m_TrackingModel->ModelType == COV_SUB_WINDOWS_B)
		m_feature = (GenericFeature *)new CovariancePatchDescriptor(boxR, this->m_TrackingModel->tracker_param);

	m_feature->computeFeature(this->m_TrackingModel);
}

void GenericParticleFilter::PredictNewBases(double dSigmaFactor)
{
	int nNewIndex = 0;
	double dt = 1/this->m_fps;
	
	Mat tmp = this->m_imgSearch.clone();
	
	for (nNewIndex = 0; nNewIndex < m_nParticles; nNewIndex++)
	{
		int nOldIndex = PickBaseSample();
		
		const double xx = s[nOldIndex][0] + dt*s[nOldIndex][2] + dSigmaFactor * sigma[0] * gaussian_random();
		const double yy = s[nOldIndex][1] + dt*s[nOldIndex][3] + dSigmaFactor * sigma[1] * gaussian_random();
		const double vx = s[nOldIndex][2] + dSigmaFactor * sigma[2] * gaussian_random();
		const double vy = s[nOldIndex][3] + dSigmaFactor * sigma[3] * gaussian_random();
		const double ss = s[nOldIndex][4] + dt*s[nOldIndex][5] + dSigmaFactor * sigma[4] * gaussian_random();
		const double vs = s[nOldIndex][5] + dSigmaFactor * sigma[5] * gaussian_random();
		const double aa = 0;//s[nOldIndex][6] + dt*s[nOldIndex][7] + dSigmaFactor * sigma[6] * gaussian_random();
		const double va = 0;//s[nOldIndex][7] + dSigmaFactor * sigma[7] * gaussian_random();
		
		/*
		 cout << "xx: "<<xx<<endl;
		 cout << "yy: "<<yy<<endl;
		 cout << "ss: "<<ss<<endl;
		 cout << "aa: "<<aa<<endl;
		 cout << endl;
		 */
		int x = int(xx + 0.5);
		int y = int(yy + 0.5);
		
		
		if (x < lower_limit[0] || x > upper_limit[0])
			s_temp[nNewIndex][0] = s_temp[nOldIndex][0];
		else
			s_temp[nNewIndex][0] = xx;
		
		if (y < lower_limit[1] || y > upper_limit[1])
			s_temp[nNewIndex][1] = s_temp[nOldIndex][1];
		else
			s_temp[nNewIndex][1] = yy;
		
		if (vx < lower_limit[2] || vx > upper_limit[2])
			s_temp[nNewIndex][2] = s_temp[nOldIndex][2];
		else
			s_temp[nNewIndex][2] = vx;
		
		if (vy < lower_limit[3] || vy > upper_limit[3])
			s_temp[nNewIndex][3] = s_temp[nOldIndex][3];
		else
			s_temp[nNewIndex][3] = vy;
		
		if (ss < lower_limit[4] || ss > upper_limit[4])
			s_temp[nNewIndex][4] = s_temp[nOldIndex][4];
		else
			s_temp[nNewIndex][4] = ss;
		
		if (vs < lower_limit[5] || vs > upper_limit[5])
			s_temp[nNewIndex][5] = s_temp[nOldIndex][5];
		else
			s_temp[nNewIndex][5] = vs;
		
		if (aa < lower_limit[6] || aa > upper_limit[6])
			s_temp[nNewIndex][6] = s_temp[nOldIndex][6];
		else
			s_temp[nNewIndex][6] = aa;
		
		if (va < lower_limit[7] || va > upper_limit[7])
			s_temp[nNewIndex][7] = s_temp[nOldIndex][7];
		else
			s_temp[nNewIndex][7] = va;
		
		
		Point2f center(s_temp[nNewIndex][0], s_temp[nNewIndex][1]);
		Size2f sizeB(model.oWidth*s_temp[nNewIndex][4],model.oHeight*s_temp[nNewIndex][4]);
		circle(tmp, center,2,cvScalar(0,0,255));
		//drawRotatedRect(tmp, RotatedRect(center,sizeB,s_temp[nNewIndex][6]));
		
		/*
		 cout << "xx2: "<<s_temp[nNewIndex][0]<<endl;
		 cout << "yy2: "<<s_temp[nNewIndex][1]<<endl;
		 cout << "ss2: "<<s_temp[nNewIndex][4]<<endl;
		 cout << "aa2: "<<s_temp[nNewIndex][6]<<endl;
		 cout << endl;
		 */
	}
	
	// switch old/new
	double **temp = s_temp;
	s_temp = s;
	s = temp;
	
	
	cvNamedWindow("Particle distribution");
	imshow( "Particle distribution", tmp);
	//cvWaitKey();
	//cvDestroyWindow("Particle distribution");
	
}

double GenericParticleFilter::CalculateProbability(bool bSeparateCall)
{
	
	double dist = m_TrackingModel->distance(m_feature);

	double sigma = 1;//1 original	
	if (this->m_TrackingModel->ModelType == COV_FULL_IMAGE) sigma = 1;
	else if (this->m_TrackingModel->ModelType == COV_SUB_WINDOWS) sigma = 1;
	else if (this->m_TrackingModel->ModelType == HAAR){
		double aux = 1.0/(sqrt(2.0*Condense_PI) * sigma)*exp(-0.5 * (dist*dist / (sigma*sigma)));
		cout << "dist: "<<dist<<" aux:"<<aux<<endl;
		return aux;
	}
	
	//double aux = 1.0/(sqrt(2.0*Condense_PI) * sigma)*exp(-0.5 * (dist*dist / (sigma*sigma)));
	//cout << "dist: "<<dist<<" aux:"<<aux<<endl;
	
	return 1.0/(sqrt(2.0*Condense_PI) * sigma)*exp(-0.5 * (dist*dist / (sigma*sigma)));
	
}

void GenericParticleFilter::SetImage(Mat& img){
	this->m_imgSearch = img;
	this->width = img.size().width;
	this->height = img.size().height;
	
	m_TrackingModel->updateModel(img);
}

void GenericParticleFilter::InitParticles(RotatedRect& box, int scale)
{
	int i;
	
	// init particle related attributes
	for (i = 0; i < m_nParticles; i++)
	{
		// particle positions
		s[i][0] = box.center.x;
		s[i][1] = box.center.y;
		s[i][2] = uniform_random();
		s[i][3] = uniform_random();
		s[i][4] = scale;
		s[i][5] = uniform_random();
		s[i][6] = box.angle;
		s[i][7] = uniform_random();
		
		s_temp[i][0] = box.center.x;
		s_temp[i][1] = box.center.y;
		s_temp[i][2] = uniform_random();
		s_temp[i][3] = uniform_random();
		s_temp[i][4] = scale;
		s_temp[i][5] = uniform_random();
		s_temp[i][6] = box.angle;
		s_temp[i][7] = uniform_random();
		// probability for each particle
		
		pi[i] = 1.0 / m_nParticles;
		
	}
	
	// initialize configurations
	for (i = 0; i < DIMENSION_2D; i++)
	{
		mean_configuration[i] = s[0][i];
		last_configuration[i] = s[0][i];
	}
	
	c_total = 1.0;
	
	/*
	 // limits for positions
	 lower_limit[0] = 0;	
	 upper_limit[0] = width;
	 
	 lower_limit[1] = 0;	
	 upper_limit[1] = height;
	 
	 
	 lower_limit[2] = -50;	
	 upper_limit[2] =  50;
	 
	 lower_limit[3] = -50;	
	 upper_limit[3] =  50;
	 
	 lower_limit[4] = 0.5;
	 upper_limit[4] = 1.5;
	 
	 lower_limit[5] = -5;	
	 upper_limit[5] =  5;
	 
	 lower_limit[6] = -30;	
	 upper_limit[6] =  30;
	 
	 lower_limit[7] = -15;	
	 upper_limit[7] =  15;
	 
	 // maximum offset for next configuration
	 sigma[0] = 20;
	 sigma[1] = 20;
	 sigma[2] = 30;
	 sigma[3] = 30;
	 sigma[4] = 0.5;
	 sigma[5] = 0.4;
	 sigma[6] = 10;
	 sigma[7] = 10;
	 */
	
	// limits for positions
	lower_limit[0] = 0;	
	upper_limit[0] = width;
	
	lower_limit[1] = 0;	
	upper_limit[1] = height;
	
	
	lower_limit[2] = -20;	
	upper_limit[2] =  20;
	
	lower_limit[3] = -20;	
	upper_limit[3] =  20;
	
	lower_limit[4] = 0.5;
	upper_limit[4] = 1.5;//2.5;
	
	lower_limit[5] = -5;	
	upper_limit[5] =  5;
	
	lower_limit[6] = -30;	
	upper_limit[6] =  30;
	
	lower_limit[7] = -15;	
	upper_limit[7] =  15;
	
	// maximum offset for next configuration
	sigma[0] = 8;
	sigma[1] = 8;
	sigma[2] = 8;//10;
	sigma[3] = 8;//10;
	sigma[4] = 0.5;//0.25;
	sigma[5] = 0.4;
	sigma[6] = 7;//10;
	sigma[7] = 7;//10;
}


