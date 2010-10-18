#include "LogEuclideanMath.h"


Mat mapLogMat(const Mat& Crxy, config_SystemParameter *param){
	CV_Assert( Crxy.rows == param->numFeature && Crxy.cols == param->numFeature);
	
	SVD decomp(Crxy);
	cv::log(decomp.w,decomp.w);
	Mat W = decomp.w.diag(decomp.w);
	Mat logA = decomp.u*W*decomp.vt;
	
	return logA;	
}
Mat mapExpMat(const Mat& Crxy, config_SystemParameter *param){
	CV_Assert( Crxy.rows == param->numFeature && Crxy.cols == param->numFeature);
	
	SVD decomp(Crxy);
	cv::exp(decomp.w,decomp.w);
	Mat W = decomp.w.diag(decomp.w);
	Mat expA = decomp.u*W*decomp.vt;
	
	return expA;	
}
Mat avgMat(const vector<Mat>& src, int flags, config_SystemParameter *param){
	
	return mapExpMat(avgLogMat(src,flags,param),param);
}
Mat avgLogMat(const vector<Mat>& src, int flags, config_SystemParameter *param){
	CV_Assert(src.size()>=1);
	
	Mat avgMat(src[0].rows,src[0].cols,src[0].type());
	avgMat = avgMat-avgMat;
	
	for (int i=0; i<src.size(); i++) {
		avgMat = avgMat + (flags==MAT_MAP_LOG ? src[i]: mapLogMat(src[i],param) );
	}
	
	return avgMat/src.size();
}

//----------------------------------------------------------------

double regionDist(Mat& Crxy1, Mat& Crxy2, config_SystemParameter *param, int flags){
	CV_Assert(Crxy1.rows == param->numFeature && Crxy1.cols == param->numFeature && 
			  Crxy2.rows == param->numFeature && Crxy2.cols == param->numFeature);	
	
	double dist;
	
	if (flags==MAT_COV)	dist = norm(mapLogMat(Crxy1,param), mapLogMat(Crxy2,param), NORM_L2);
	else if(flags==MAT_MAP_LOG)	dist = norm(Crxy1, Crxy2, NORM_L2);
	else dist = norm(mapLogMat(Crxy1,param), mapLogMat(Crxy2,param), NORM_L2);
	
	return  dist;
	
}
double regionDist(const vector<Mat>& Crxy1, const vector<Mat>& Crxy2, config_SystemParameter *param, int flags){
	CV_Assert(Crxy1[0].rows == param->numFeature && Crxy1[0].cols == param->numFeature && 
			  Crxy2[0].rows == param->numFeature && Crxy2[0].cols == param->numFeature);	
	CV_Assert(Crxy1.size()==Crxy2.size());
	
	double dist=0;//,distJ,distI;
	double minDist = 1e+37;
	
	for (int i=0; i<Crxy1.size(); i++) {			
		if (flags==MAT_COV)	dist = norm(mapLogMat(Crxy1[i],param), mapLogMat(Crxy2[i],param), NORM_L2);
		else if(flags==MAT_MAP_LOG) dist = norm(Crxy1[i], Crxy2[i], NORM_L2);
		else CV_Assert(false);
			
		if (minDist>dist) minDist = dist;
		//minDist += dist;
	}

	/*
	for (int j=0; j<Crxy1.size(); j++) {
		dist = 0;
		for (int i= 0; i<Crxy1.size(); i++) {
			
			if (flags==MAT_COV)	distI = norm(mapLogMat(Crxy1[i]	), mapLogMat(Crxy2[i]), NORM_L2);
			else if(flags==MAT_MAP_LOG) distI = norm(Crxy1[i], Crxy2[i], NORM_L2);
			else CV_Assert(false);
			
			if (flags==MAT_COV)	distJ = norm(mapLogMat(Crxy1[j]	), mapLogMat(Crxy2[j]), NORM_L2);
			else if(flags==MAT_MAP_LOG) distJ = norm(Crxy1[j], Crxy2[j], NORM_L2);
			else CV_Assert(false);
			
			dist += distI-distJ;
			
		}
		if (minDist>dist) minDist = dist;
	}
	 */
	
	return  minDist;
}

//----------------------------------------------------------------

/**************************************************/
/*** NO ES MATEMATICA LOG-E                        */
/**************************************************/

//imagenes de 3 canales pero blanco y negro
//acuerdate de cambiar la cantidad de caracteristicas a 10
void makeFgray(const Mat& img, vector<Mat>& F, config_SystemParameter *param){
	CV_Assert(F.size() == param->numFeature);
	CV_Assert(img.channels() == 3);
	//por el momento, deberia ser borrado este assert
	CV_Assert(param->numFeature == 10);
	
	Mat greyImg,greyAux;
	cvtColor(img, greyImg, CV_RGB2GRAY);
	equalizeHist(greyImg, greyAux );	
	
	//INICIALIZANDO VECTOR CON VALORES FLOATS
	for (int i=0; i<param->numFeature; i++) F[i]  = Mat(img.rows,img.cols,CV_32F);
	
	//CREANDO MATRICES X E Y
	for(int i = 0; i < F[0].rows; i++)
	{
		float* MatF0i = F[0].ptr<float>(i);
		float* MatF1i = F[1].ptr<float>(i);
		
		for(int j = 0; j < F[0].cols; j++){
			MatF0i[j] = (float)j;
			MatF1i[j] = (float)i;
		}
	}
	
	//MATRICES RGB PERO CON FORMATO FLOAT
	greyAux.convertTo(F[2], CV_32F);
	
	//PRIMERA DERIVADA
	cv::Ptr<FilterEngine> Fc = createDerivFilter(F[2].type(), CV_32F, 1, 0, 3);
	Fc->apply(F[2],F[3], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[3] = abs(F[3]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 1, 3);
	Fc->apply(F[2],F[4], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[4] = abs(F[4]);
	
	//MAGNITUD DE LA DERIVADA
	magnitude(F[3], F[4], F[5]);
	
	//SEGUNDA DERIVADA
	Fc = createDerivFilter(F[2].type(), CV_32F, 2, 0, 3);
	Fc->apply(F[2],F[6], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[6] = abs(F[6]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 2, 3);
	Fc->apply(F[2],F[7], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[7] = abs(F[7]);
	
	//PHASE DE LA PRIMERA DERIVADA EN X E Y
	phase(F[3], F[4], F[8]);
	
	//SILENCY
	
	IplImage srcImg, *dstImg;
	
	srcImg = IplImage(greyAux);
	dstImg = cvCreateImage(cvSize(srcImg.width, srcImg.height), 8, 1);
	Saliency *saliency = new Saliency;
	
	saliency->calcIntensityChannel(&srcImg, dstImg);
	
	F[9] = Mat(dstImg);
	Mat aux(img.rows,img.cols,CV_32F);
	F[9].convertTo(aux,F[7].type());
	F[9] = aux;	
	
	greyAux.release();
	greyImg.release();
	delete saliency;
}

//imagenes de color
//acuerdate de cambiar la cantidad de caracteristicas a 12
void makeFrgb(const Mat& img, vector<Mat>& F, config_SystemParameter *param){
	CV_Assert(F.size() == param->numFeature);
	CV_Assert(img.channels() == 3);
	//por el momento, deberia ser borrado este assert
	CV_Assert(param->numFeature == 12);
	
	Mat greyImg,greyAux;
	cvtColor(img, greyImg, CV_RGB2GRAY);
	equalizeHist(greyImg, greyAux );
	greyAux.convertTo(greyImg, CV_32F);
	//greyAux.release();
	
	
	vector<Mat> Mrgb;
	split(img, Mrgb);
	
	//INICIALIZANDO VECTOR CON VALORES FLOATS
	for (int i=0; i<param->numFeature; i++) {
		F[i]  = Mat(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	}
	
	//CREANDO MATRICES X E Y
	for(int i = 0; i < F[0].rows; i++)
	{
		float* MatF0i = F[0].ptr<float>(i);
		float* MatF1i = F[1].ptr<float>(i);
		
		for(int j = 0; j < F[0].cols; j++){
			MatF0i[j] = (float)j;
			MatF1i[j] = (float)i;
		}
	}
	
	//MATRICES RGB PERO CON FORMATO FLOAT
	
	greyAux.release();
	Mrgb[0].convertTo(F[2], CV_32F);
	equalizeHist(Mrgb[1], greyAux );
	greyAux.convertTo(F[3], CV_32F);
	greyAux.release();
	equalizeHist(Mrgb[2], greyAux );
	greyAux.convertTo(F[4], CV_32F);
	
	
	//PRIMERA DERIVADA
	cv::Ptr<FilterEngine> Fc = createDerivFilter(F[2].type(), CV_32F, 1, 0, 3);
	Fc->apply(greyImg,F[5], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[5] = abs(F[5]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 1, 3);
	Fc->apply(greyImg,F[6], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[6] = abs(F[6]);
	
	//MAGNITUD DE LA DERIVADA
	magnitude(F[5], F[6], F[7]);
	
	//SEGUNDA DERIVADA
	Fc = createDerivFilter(F[2].type(), CV_32F, 2, 0, 3);
	Fc->apply(greyImg,F[8], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[8] = abs(F[8]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 2, 3);
	Fc->apply(greyImg,F[9], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[9] = abs(F[9]);
	
	//PHASE DE LA PRIMERA DERIVADA EN X E Y
	phase(F[5], F[6], F[10]);
	
	//SILENCY
	
	IplImage srcImg, *dstImg;
	
	srcImg = IplImage(greyAux);
	dstImg = cvCreateImage(cvSize(srcImg.width, srcImg.height), 8, 1);
	Saliency *saliency = new Saliency;
	
	saliency->calcIntensityChannel(&srcImg, dstImg);
	
	F[11] = Mat(dstImg);
	Mat aux(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	F[11].convertTo(aux,F[9].type());
	F[11] = aux;
	
	greyAux.release();
	greyImg.release();
	delete saliency;
	
}
/*
void makeF(const Mat& img, vector<Mat>& F, config_SystemParameter *param){
	CV_Assert(F.size() == param->numFeature);
	CV_Assert(img.channels() == 3);
	//por el momento, deberia ser borrado este assert
	CV_Assert(param->numFeature == 12);
	
	Mat greyImg,greyAux;
	cvtColor(img, greyImg, CV_RGB2GRAY);
	equalizeHist(greyImg, greyAux );
	greyAux.convertTo(greyImg, CV_32F);
	//greyAux.release();
	
	
	vector<Mat> Mrgb;
	split(img, Mrgb);
	
	//INICIALIZANDO VECTOR CON VALORES FLOATS
	for (int i=0; i<param->numFeature; i++) {
		F[i]  = Mat(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	}
	
	//CREANDO MATRICES X E Y
	for(int i = 0; i < F[0].rows; i++)
	{
		float* MatF0i = F[0].ptr<float>(i);
		float* MatF1i = F[1].ptr<float>(i);
		
		for(int j = 0; j < F[0].cols; j++){
			MatF0i[j] = (float)j;
			MatF1i[j] = (float)i;
		}
	}
	
	//MATRICES RGB PERO CON FORMATO FLOAT
	
	greyAux.release();
	//equalizeHist(Mrgb[0], greyAux );
	Mrgb[0].convertTo(F[2], CV_32F);
	//greyAux.convertTo(F[2], CV_32F);
	equalizeHist(Mrgb[1], greyAux );
	//Mrgb[1].convertTo(F[3], CV_32F);
	greyAux.convertTo(F[3], CV_32F);
	greyAux.release();
	equalizeHist(Mrgb[2], greyAux );
	//Mrgb[2].convertTo(F[4], CV_32F);
	greyAux.convertTo(F[4], CV_32F);
	
	//Mrgb[0].convertTo(F[2], CV_32F);
	//Mrgb[1].convertTo(F[3], CV_32F);
	//Mrgb[2].convertTo(F[4], CV_32F);
	
	//printMat(Mrgb[0],MAT_TYPE_UCHAR);
	//cout << endl;
	//printMat(F[2],MAT_TYPE_FLOAT);
	
	//PRIMERA DERIVADA
	cv::Ptr<FilterEngine> Fc = createDerivFilter(F[2].type(), CV_32F, 1, 0, 3);
	Fc->apply(greyImg,F[5], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[5] = abs(F[5]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 1, 3);
	Fc->apply(greyImg,F[6], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[6] = abs(F[6]);
	
	//MAGNITUD DE LA DERIVADA
	magnitude(F[5], F[6], F[7]);
	
	//SEGUNDA DERIVADA
	Fc = createDerivFilter(F[2].type(), CV_32F, 2, 0, 3);
	Fc->apply(greyImg,F[8], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[8] = abs(F[8]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 2, 3);
	Fc->apply(greyImg,F[9], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[9] = abs(F[9]);
	
	//PHASE DE LA PRIMERA DERIVADA EN X E Y
	phase(F[5], F[6], F[10]);
	
	//SILENCY
	
	IplImage srcImg, *dstImg;
	
	srcImg = IplImage(greyAux);
	dstImg = cvCreateImage(cvSize(srcImg.width, srcImg.height), 8, 1);
	Saliency *saliency = new Saliency;
	
	saliency->calcIntensityChannel(&srcImg, dstImg);
	
	
	F[11] = Mat(dstImg);
	Mat aux(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	//Mat aux2(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	F[11].convertTo(aux,F[9].type());
	F[11] = aux;
	
	greyAux.release();
	greyImg.release();
	
}
*/
/*
void makeF(const Mat& img, vector<Mat>& F){
	CV_Assert(F.size() == NUM_FEATURES);
	CV_Assert(img.channels() == 3);
	
	vector<Mat> Mrgb;
	split(img, Mrgb);
	
	//INICIALIZANDO VECTOR CON VALORES FLOATS
	for (int i=0; i<NUM_FEATURES; i++) {
		F[i]  = Mat(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	}
	
	//CREANDO MATRICES X E Y
	for(int i = 0; i < F[0].rows; i++)
	{
		float* MatF0i = F[0].ptr<float>(i);
		float* MatF1i = F[1].ptr<float>(i);
		
		for(int j = 0; j < F[0].cols; j++){
			MatF0i[j] = (float)j;
			MatF1i[j] = (float)i;
		}
	}
	
	//MATRICES RGB PERO CON FORMATO FLOAT
	Mrgb[0].convertTo(F[2], CV_32F);
	Mrgb[1].convertTo(F[3], CV_32F);
	Mrgb[2].convertTo(F[4], CV_32F);
	
	//printMat(Mrgb[0],MAT_TYPE_UCHAR);
	//cout << endl;
	//printMat(F[2],MAT_TYPE_FLOAT);
	
	//PRIMERA DERIVADA
	cv::Ptr<FilterEngine> Fc = createDerivFilter(F[2].type(), CV_32F, 1, 0, 3);
	Fc->apply(F[3],F[5], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[5] = abs(F[5]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 1, 3);
	Fc->apply(F[3],F[6], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[6] = abs(F[6]);
	
	//MAGNITUD DE LA DERIVADA
	magnitude(F[5], F[6], F[7]);
	
	//SEGUNDA DERIVADA
	Fc = createDerivFilter(F[2].type(), CV_32F, 2, 0, 3);
	Fc->apply(F[3],F[8], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[8] = abs(F[8]);
	
	Fc = createDerivFilter(F[2].type(), CV_32F, 0, 2, 3);
	Fc->apply(F[3],F[9], cv::Rect(0,0,-1,-1),cv::Point(0,0));
	F[9] = abs(F[9]);
	
	//PHASE DE LA PRIMERA DERIVADA EN X E Y
	phase(F[5], F[6], F[10]);
	
	//SILENCY
	
	IplImage srcImg, *dstImg;
	
	srcImg = IplImage(img);
	dstImg = cvCreateImage(cvSize(srcImg.width, srcImg.height), 8, 1);
	Saliency *saliency = new Saliency;
	
	saliency->calcIntensityChannel(&srcImg, dstImg);
	
	
	F[11] = Mat(dstImg);
	Mat aux(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	//Mat aux2(Mrgb[2].rows,Mrgb[2].cols,CV_32F);
	F[11].convertTo(aux,F[9].type());
	F[11] = aux;
	
	//printMat(F[11],MAT_TYPE_FLOAT);
	//cout << endl;
	//printMat(F[11],MAT_TYPE_DOUBLE);
	//cvNamedWindow("test",CV_WINDOW_AUTOSIZE);
	//imshow("test",F[11]);
	//cvWaitKey ();
	
}
*/

//--------------------------------------------------------------

void makePQ(const vector<Mat>& F, vector<Mat>& P, vector< vector<Mat> >& Q, config_SystemParameter *param){
	CV_Assert(F.size() == param->numFeature);
	CV_Assert(P.size() == param->numFeature);
	CV_Assert(Q.size() == param->numFeature && Q[0].size() == param->numFeature);
	
	for (int i=0; i<param->numFeature; i++){
		P[i] = Mat(F[0].rows+1, F[0].cols+1, CV_32F);
		integral(F[i],P[i]);
		P[i] = P[i](Range(1, P[0].rows), Range(1,P[0].cols));
			
		for (int j=0; j<param->numFeature; j++){
			Q[i][j] = Mat(F[0].rows+1, F[0].cols+1, CV_32F);			
			integral(F[i].mul(F[j]), Q[i][j]);
			Q[i][j] = Q[i][j](Range(1, Q[0][0].rows), Range(1,Q[0][0].cols));
		}
	}
}
Mat makePxy(const vector<Mat>& P, int x, int y, config_SystemParameter *param){
	CV_Assert( x>=0 && y>=0 && x<P[0].cols && y<P[0].rows);
	
	Mat Pxy(param->numFeature,1,CV_64F);
	
	for(int i = 0; i <param->numFeature; i++){
		double* Pxyi = Pxy.ptr<double>(i);
		Pxyi[0] = P[i].at<double>(y,x);
	}
	return Pxy;
}
Mat makeQxy(const vector< vector<Mat> >& Q, int x, int y, config_SystemParameter *param){
	CV_Assert( x>=0 && y>=0 && x<Q[0][0].cols && y<Q[0][0].rows);
	
	Mat Qxy(param->numFeature, param->numFeature,CV_64F);
	
	for(int i = 0; i < param->numFeature; i++){
		double* Qxyi = Qxy.ptr<double>(i);
		for (int j=0; j<param->numFeature; j++)	Qxyi[j] = Q[i][j].at<double>(y, x);	
	}
	
	return Qxy;
}

//---------------------------------------------------------------

void CovarianceRegion(const vector<Mat>& P, const vector< vector<Mat> >& Q, Mat& Crxy, const Point2i& pointP1,const Point2i& pointP2, config_SystemParameter *param){
	CV_Assert( Crxy.rows == param->numFeature && Crxy.cols == param->numFeature );
	CV_Assert( pointP1.x<pointP2.x && pointP1.y<pointP2.y && pointP1.x!=pointP2.x && pointP1.y!=pointP2.y);
	CV_Assert( (pointP2.x-pointP1.x)>=1 && (pointP2.y-pointP1.y)>=1);
	
	double n = (pointP2.x-pointP1.x)*(pointP2.y-pointP1.y);
	
	Mat AuxQ(param->numFeature,param->numFeature,P[0].type());
	Mat AuxP(param->numFeature,param->numFeature,P[0].type());
	
	AuxQ = makeQxy(Q, pointP2.x, pointP2.y,param)+makeQxy(Q, pointP1.x, pointP1.y,param)-
		   makeQxy(Q, pointP2.x, pointP1.y,param)-makeQxy(Q, pointP1.x, pointP2.y,param);
	AuxP = makePxy(P, pointP2.x, pointP2.y,param)+makePxy(P, pointP1.x, pointP1.y,param)-
		   makePxy(P, pointP1.x, pointP2.y,param)-makePxy(P, pointP2.x, pointP1.y,param);
	
	Crxy = (1/n)*(AuxQ-((1/n)*AuxP*AuxP.t()));
	
	AuxP.release();
	AuxQ.release();
	
}
void CovarianceRegion(const vector<Mat>& P, const vector< vector<Mat> >& Q, Mat& Crxy, config_SystemParameter *param){
	CV_Assert( Crxy.rows == param->numFeature && Crxy.cols == param->numFeature );

	
	double n = (P[0].rows)*(P[0].cols);
	
	Mat AuxQ(param->numFeature,param->numFeature,P[0].type());
	Mat AuxP(param->numFeature,param->numFeature,P[0].type());
	
	AuxQ = makeQxy(Q, P[0].cols-1, P[0].rows-1,param)+makeQxy(Q, 0, 0,param)-
		   makeQxy(Q, P[0].cols-1, 0,param)-makeQxy(Q, 0, P[0].rows-1,param);
	AuxP = makePxy(P, P[0].cols-1, P[0].rows-1,param)+makePxy(P, 0, 0,param)-
		   makePxy(P, 0, P[0].rows-1,param)-makePxy(P, P[0].cols-1, 0,param);
	
	Crxy = (1/n)*(AuxQ-((1/n)*AuxP*AuxP.t()));
	
	AuxP.release();
	AuxQ.release();
}
