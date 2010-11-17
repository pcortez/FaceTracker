#include <cassert>
#include <fstream>
#include <iomanip>


#include "Utilities.h"
#include "LogEuclideanMath.h"


//#include "GenericParticleFilter.h"
#include "ExhaustiveTracking.h"
#include "Generic.h"
#include "CondensationTracking.h"
#include "TimeQueue.h"
#include "frameObject.h"
	
const char  * WINDOW_NAME  = "Face Tracker";

using namespace std;
using namespace cv;

const double widthSmall = 240;
const double heightSmall = 180;
const int outTreshold = 3;
Mat OriginalFrame,DrawFrame,miniFrame;
short selection=-1;

bool readMainArgument(int argc, char * const argv[], frameObject cap[], config_SystemParameter param[]){
	if (argc != 4) {
		cout << "WRONG ARGUMENT NUMBER: "<<argc<<endl;
		return false;
	}
	
	string path;
	
	for (int i=0; i<argc; i++) {
		if(strcmp(argv[i],"-queue") == 0){
			cout <<argv[i]<<": "<< "ON"<<endl;
			selection = 0;
			break;
		}
		else if(strcmp(argv[i],"-compare") == 0){
			cout <<argv[i]<<": "<< "ON"<<endl;
			selection = 1;
			break;
		}
	}
	
	if (selection == -1) {
		cout << "WRONG FLAG -compare or - queue";
		return false;
	}
	
	param[0].debugMode = false;
	param[1].debugMode = false;
	
	
	cout << "MAIN ARGUMENTS"<<endl;
	for (int i=1; i<argc; i++) {
		
		if(strcmp(argv[i],"-configPath") == 0){
			if (selection==1) {
				//Compare
				if(!readConfigAppCompare(argv[i+1], &cap[0], &param[0])) return false;
			}
			else{
				//Queue
				if(!readConfigAppQueue(argv[i+1], cap, param)) return false;
			}
		}
	}
	cout << "------------------------------------------------------"<<endl<<endl;

	cap[0].setSeeker(param[0].startTime, param[0].startTimeOnFrames);

	
	return true;
}

int queueTracking(frameObject cap[], config_SystemParameter param[]){
	cvNamedWindow(WINDOW_NAME,CV_WINDOW_AUTOSIZE);
	
	cap[0].grabNextFrame();
	OriginalFrame = cap[0].getNextFrame();
	CovariancePatchModelv2 *TrackingModel_P;
	ExhaustiveTracking *et_P;
	
	//time variable
	int iniFrame = 0;
	int lastFrame = 1;
	bool startTimerQueue = false;
	//out box
	int countRectOut = 0;
	
	VideoWriter video(param[0].videoPath+".avi",CV_FOURCC('D', 'I', 'V', 'X'), cap[0].getFPS(), OriginalFrame.size());
	RotatedRect rectS,rectSmall;
	
	ofstream positionfile;
	string pathPosition = param[0].videoPath.substr(0, param[0].videoPath.find_last_of("/")+1) +"position.txt"; 
	positionfile.open (pathPosition.c_str(), ios::trunc);
	
	for (int i=0; i<2; i++) {
		//ini variable
		countRectOut = 0;
		cap[i].grabNextFrame();
		OriginalFrame = cap[i].getNextFrame();
		DrawFrame = OriginalFrame.clone();
		resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
		cv::Size2f scaleFactor(widthSmall/OriginalFrame.size().width,heightSmall/OriginalFrame.size().height);
		
		if (i==0){
			rectSmall = scaleRect(param[i].initialRect, scaleFactor);
		}
		else {
			param[i].initialRect = searchFace(OriginalFrame,TrackingModel_P,scaleFactor, param[i].debugMode);
			rectSmall = scaleRect(param[i].initialRect, scaleFactor);
			//et_P->updateVariables(rectSmall);
			//et_P->update(miniFrame);
			//et_P->getNextPosition(rectSmall, cap[i].get(CV_CAP_PROP_FPS));
			delete TrackingModel_P;
			delete et_P;
		}
		
		drawRotatedRect(DrawFrame, param[i].initialRect);
		imshow(WINDOW_NAME,DrawFrame);
		if(param[i].debugMode) cvWaitKey();
		
		TrackingModel_P = new CovariancePatchModelv2(miniFrame, rectSmall, &param[i]);
		et_P = new ExhaustiveTracking(TrackingModel_P, miniFrame, &param[i]);
		
		while (cap[i].grabNextFrame() && cvWaitKey(1)!=27) {
			double t = (double)getTickCount();
			
			Mat auxPatch;
			OriginalFrame = cap[i].getNextFrame();
			DrawFrame = OriginalFrame.clone();
			resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
			et_P->update(miniFrame);
			double prob2 = et_P->getNextPosition(rectS, cap[i].getFPS());
			
			drawLimit(DrawFrame);
			
			rectS = scaleRect(rectS, cv::Size2f(1/scaleFactor.width, 1/scaleFactor.height));
			drawRotatedRect(DrawFrame, rectS,CV_RGB(0,255,0),2);
			
			//checking ini time queue
			if (!startTimerQueue) {
				startTimerQueue = checkStartLimit(rectS, DrawFrame);
				iniFrame = (startTimerQueue ? lastFrame : 1);
			}
			else {
				drawTime(DrawFrame, calculateQueueTime(iniFrame, lastFrame, cap[i].getFPS()));
			}
			lastFrame++;
			
			t = ((double)getTickCount() - t)/getTickFrequency();
			cout <<"Frame: "<<setw(3)<<cap[i].getSeeker()<< " Prob: "<<prob2<<" Time elaps: "<<t<<endl;
			if((int)cap[i].getSeeker()%10==1 || lastFrame==2)
				positionfile <<	cap[i].getSeeker()<<"	"<<floor(rectS.center.x)<<"	"<<floor(rectS.center.y)<<endl; 
			
			imshow(WINDOW_NAME,DrawFrame);
			video<<DrawFrame;
			
			//checking out of image
			if (rectS.center.x-rectS.size.width/2 <= 10){
				countRectOut++;
				if (countRectOut>outTreshold) {
					cout << "OBJECT IS OUT OF VIDEO"<<endl;
					break;
				}
			}
			else
				countRectOut = (countRectOut==0?0:countRectOut-1);
		}
		
		if (i<1)
			cap[i+1].setSeeker(cap[i].getSeeker(), true);
		
		OriginalFrame.release();
		miniFrame.release();
		DrawFrame.release();
		
	}
	
	cout << "FINAL QUEUE WAITING TIME:" << calculateQueueTime(iniFrame, lastFrame, cap[0].getFPS())<< " seg"<< endl;
	
	ofstream myfile;
	string path = param[0].videoPath.substr(0, param[0].videoPath.find_last_of("/")+1) +"final_queue_waiting_time.txt"; 
	myfile.open (path.c_str(), ios::trunc);
	myfile << calculateQueueTime(iniFrame, lastFrame, cap[0].getFPS())<< " seg"<<endl;
	myfile <<"outTreshold: "<<outTreshold;
	myfile.close();
	positionfile.close();
	
	cap[0].release();
	cap[1].release();
	cvDestroyWindow(WINDOW_NAME);
	delete TrackingModel_P;
	delete et_P;
	
    return 0;
}

int compareTracking(frameObject cap[], config_SystemParameter param[]){
	cvNamedWindow(WINDOW_NAME,CV_WINDOW_AUTOSIZE);
	
	cap[0].grabNextFrame();
	OriginalFrame = cap[0].getNextFrame();
	CovariancePatchModelv2 *TrackingModel_P;
	ExhaustiveTracking *et_P;
	
	//out box
	int countRectOut = 0;
	
	VideoWriter video(param[0].videoPath+".avi",CV_FOURCC('D', 'I', 'V', 'X'), cap[0].getFPS(), OriginalFrame.size());
	RotatedRect rectS,rectSmall;
	
	ofstream positionfile;
	string pathPosition = param[0].videoPath.substr(0, param[0].videoPath.find_last_of("/")+1) +"position.txt"; 
	positionfile.open (pathPosition.c_str(), ios::trunc);
	
	//caption
	vector<CvScalar> colorLine;
	vector<string> texts;
	texts.push_back("ONBNN");
	colorLine.push_back(CV_RGB(0,255,0));
	
	//ini variable
	countRectOut = 0;
	OriginalFrame = cap[0].getNextFrame();
	DrawFrame = OriginalFrame.clone();
	resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
	cv::Size2f scaleFactor(widthSmall/OriginalFrame.size().width,heightSmall/OriginalFrame.size().height);
	
	rectSmall = scaleRect(param[0].initialRect, scaleFactor);
	drawRotatedRect(DrawFrame, param[0].initialRect);
	
	imshow(WINDOW_NAME,DrawFrame);
	if(param[0].debugMode) cvWaitKey();
		
	TrackingModel_P = new CovariancePatchModelv2(miniFrame, rectSmall, &param[0]);
	et_P = new ExhaustiveTracking(TrackingModel_P, miniFrame, &param[0]);
	
		
	while (cap[0].grabNextFrame() && cvWaitKey(1)!=27) {
		double t = (double)getTickCount();
			
		Mat auxPatch;
		OriginalFrame = cap[0].getNextFrame();
		DrawFrame = OriginalFrame.clone();
		resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
		et_P->update(miniFrame);
		double prob2 = et_P->getNextPosition(rectS, cap[0].getFPS());
			
		rectS = scaleRect(rectS, cv::Size2f(1/scaleFactor.width, 1/scaleFactor.height));
		drawRotatedRect(DrawFrame, rectS,CV_RGB(0,255,0),2);
		drawLegend(texts, colorLine, DrawFrame);
			
		t = ((double)getTickCount() - t)/getTickFrequency();
		cout <<"Frame: "<<setw(3)<<cap[0].getSeeker()<< " Prob: "<<prob2<<" Time elaps: "<<t<<endl;
		if((int)cap[0].getSeeker()%10==1 /*|| lastFrame==2*/)
			positionfile <<	cap[0].getSeeker()<<"	"<<floor(rectS.center.x)<<"	"<<floor(rectS.center.y)<<endl; 
			
		imshow(WINDOW_NAME,DrawFrame);
		video<<DrawFrame;
			
		//checking out of image
		if (rectS.center.x-rectS.size.width/2 <= 10){
			countRectOut++;
			if (countRectOut>outTreshold) {
				cout << "OBJECT IS OUT OF VIDEO"<<endl;
				break;
			}
		}
		else
			countRectOut = (countRectOut==0?0:countRectOut-1);
	}
	
	OriginalFrame.release();
	miniFrame.release();
	DrawFrame.release();
		
		
	positionfile.close();
	cvDestroyWindow(WINDOW_NAME);
	delete TrackingModel_P, &cap[0], et_P;

    return 0;
}


int main (int argc, char * const argv[]){
	frameObject cap[2];
	config_SystemParameter param[2];	
    if (!readMainArgument(argc, argv, cap, param))
		return 0;
	
	if (selection==0) queueTracking(cap, param);
	else if(selection==1) compareTracking(cap, param);
	else return 0;

    return 0;
}
