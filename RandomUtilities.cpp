/*
 *  RandomUtilities.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 18-03-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "RandomUtilities.h"

void randinitalize( const int init ){
	rng_state = cvRNG(init);
}

int	randint( const int min, const int max ){
	return cvRandInt( &rng_state )%(max-min+1) + min;
}

float randfloat( ){
	return (float)cvRandReal( &rng_state );
}

float randfloat(const float min, const float max ){
	return (float)cvRandReal( &rng_state )*(max+min) + min;
}

float randgaus(const float mean, const float sigma){
	double x, y, r2;
	
	do{
		x = -1 + 2 * randfloat();
		y = -1 + 2 * randfloat();
		r2 = x * x + y * y;
	}
	while (r2 > 1.0 || r2 == 0);
	
	return (float) (sigma * y * sqrt (-2.0 * log (r2) / r2)) + mean;
}