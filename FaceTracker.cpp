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
	
const char  * WINDOW_NAME  = "Face Tracker";

using namespace std;
using namespace cv;

/*
bool readMainArgument(int argc, char * const argv[], VideoCapture cap[], config_SystemParameter param[]){
	if (argc != 9) {
		cout << "WRONG ARGUMENT NUMBER";
		return false;
	}
	
	if(strcmp(argv[1],"-rightVideo") != 0){
		cout << "WRONG FLAG -rightVideo: "<< argv[1] <<endl;
		return false;
	}
	if(strcmp(argv[3],"-topVideo") != 0){
		cout << "WRONG FLAG -topVideo: "<< argv[3] <<endl;
		return false;
	}
	if(strcmp(argv[5],"-configPathRV") != 0){
		cout << "WRONG FLAG -configPathRV: "<< argv[5] <<endl;
		return false;
	}
	if(strcmp(argv[7],"-configPathTV") != 0){
		cout << "WRONG FLAG -configPathTV: "<< argv[7] <<endl;
		return false;
	}
	
	cout << "MAIN ARGUMENTS"<<endl;
	cout << argv[1]<<": "<< argv[2]<<endl;
	cout << argv[3]<<": "<< argv[4]<<endl;
	cout << argv[5]<<": "<< argv[6]<<endl;
	cout << argv[7]<<": "<< argv[8]<<endl;
	
	if (!cap[0].open(argv[2])) return false;
	if (!cap[1].open(argv[4])) return false;
	
	if (!cap[0].isOpened()) {
		cout << "WRONG PATH -rightVideo:"<< argv[2]<<endl;
		return false;
    }
	if (!cap[0].isOpened()) {
		cout << "WRONG PATH -topVideo:"<< argv[4]<<endl;
		return false;
    }
	
	
	//READ PARAMETER FOR TRACKING
	
	//loading parameter from file
	setSistemConfig(&param[0], argv[6]);
	//deleting old video files
	deleteOldVideo(param[0]);
	//revisando parametros
	if(!checkConfig(&param[0],true)){
		cout << "WRONG PATH -configPathRV:"<< argv[6]<<endl;
		return false;
	}
	
	//loading parameter from file
	setSistemConfig(&param[1], argv[8]);
	//deleting old video files
	deleteOldVideo(param[1]);
	//revisando parametros
	if(!checkConfig(&param[1],true)){
		cout << "WRONG PATH -configPathTV:"<< argv[8]<<endl;
		return false;
	}
	
	cap[0].set(CV_CAP_PROP_POS_MSEC, param[0].startMsec); 
	//cap[1].set(CV_CAP_PROP_POS_MSEC, atoi(argv[10]));
	
	return true;
}
*/

bool readMainArgument(int argc, char * const argv[], VideoCapture cap[], config_SystemParameter param[]){
	if (argc != 9) {
		cout << "WRONG ARGUMENT NUMBER";
		return false;
	}
	
	vector<bool> bool_param(argc,false);
	
	cout << "MAIN ARGUMENTS"<<endl;
	for (int i=1; i<argc; i++) {
		
		if(strcmp(argv[i],"-rightVideo") == 0){
			cout <<argv[i]<<": "<< argv[i+1]<<endl;
			if (!cap[0].open(argv[i+1])){
				cout << "WRONG FLAG -rightVideo: "<< argv[i+1] <<endl;
				return false;
			}
			if (!cap[0].isOpened()) {
				cout << "WRONG PATH -rightVideo:"<< argv[i+1]<<endl;
				return false;
			}
		}
		else if(strcmp(argv[i],"-topVideo") == 0){
			cout <<argv[i]<<": "<< argv[i+1]<<endl;
			if (!cap[1].open(argv[i+1])){
				cout << "WRONG FLAG -configPathRV: "<< argv[i+1] <<endl;
				return false;
			}
			if (!cap[1].isOpened()) {
				cout << "WRONG PATH -topVideo:"<< argv[i+1]<<endl;
				return false;
			}
		}
		else if(strcmp(argv[i],"-configPathRV") == 0){
			cout <<argv[i]<<": "<< argv[i+1]<<endl;
			//loading parameter from file
			setSistemConfig(&param[0], argv[i+1]);
			//deleting old video files
			deleteOldVideo(param[0]);
			//revisando parametros
			if(!checkConfig(&param[0],true)){
				cout << "WRONG PATH -configPathRV:"<< argv[i+1]<<endl;
				return false;
			}
		}
		else if(strcmp(argv[i],"-configPathTV") == 0){
			cout <<argv[i]<<": "<< argv[i+1]<<endl;
			//loading parameter from file
			setSistemConfig(&param[1], argv[i+1]);
			//deleting old video files
			deleteOldVideo(param[1]);
			//revisando parametros
			if(!checkConfig(&param[1],true)){
				cout << "WRONG PATH -configPathTV:"<< argv[i+1]<<endl;
				return false;
			}
		}

	}

	cap[0].set(CV_CAP_PROP_POS_MSEC, param[0].startMsec); 
	//cap[1].set(CV_CAP_PROP_POS_MSEC, atoi(argv[10]));
	
	return true;
}

int main (int argc, char * const argv[]){
	
	Mat OriginalFrame,DrawFrame,miniFrame;
    VideoCapture cap[2];
	config_SystemParameter param[2];	
	
    if (!readMainArgument(argc, argv, cap, param))
		return 0;
		
	const double widthSmall = 240;
	const double heightSmall = 180;
	const int outTreshold = 1;
	
    cvNamedWindow(WINDOW_NAME,CV_WINDOW_AUTOSIZE);
	
    cap[0] >> OriginalFrame;	
	CovariancePatchModelv2 *TrackingModel_P;
	ExhaustiveTracking *et_P;

	//time variable
	int iniFrame = 0;
	int lastFrame = 1;
	bool startTimerQueue = false;
	//out box
	int countRectOut = 0;
	
	VideoWriter video(param[0].videoPath+".avi",CV_FOURCC('D', 'I', 'V', 'X'), cap[0].get(CV_CAP_PROP_FPS) , OriginalFrame.size());
	RotatedRect rectS,rectSmall;
	
	for (int i=0; i<2; i++) {
		//ini variable
		countRectOut = 0;
		cap[i] >> OriginalFrame;
		DrawFrame = OriginalFrame.clone();
		resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
		cv::Size2f scaleFactor(widthSmall/OriginalFrame.size().width,heightSmall/OriginalFrame.size().height);
		
		if (i==0){
			rectSmall = scaleRect(param[i].initialRect, scaleFactor);
		}
		else {
			param[i].initialRect = searchFace(OriginalFrame,TrackingModel_P,scaleFactor, false);
			rectSmall = scaleRect(param[i].initialRect, scaleFactor);
			//et_P->updateVariables(rectSmall);
			//et_P->update(miniFrame);
			//et_P->getNextPosition(rectSmall, cap[i].get(CV_CAP_PROP_FPS));
			delete TrackingModel_P;
			delete et_P;
		}
		
		drawRotatedRect(DrawFrame, param[i].initialRect);
		imshow(WINDOW_NAME,DrawFrame);
		//cvWaitKey();
		
		TrackingModel_P = new CovariancePatchModelv2(miniFrame, rectSmall, &param[i]);
		et_P = new ExhaustiveTracking(TrackingModel_P, miniFrame, &param[i]);
		
		while (cap[i].get(CV_CAP_PROP_POS_MSEC)<=param[i].endMsec && cvWaitKey(1)!=27) {
			double t = (double)getTickCount();

			Mat auxPatch;
			cap[i] >> OriginalFrame;
			DrawFrame = OriginalFrame.clone();
			resize(OriginalFrame, miniFrame,cv::Size2i(widthSmall,heightSmall));
			et_P->update(miniFrame);
			double prob2 = et_P->getNextPosition(rectS, cap[i].get(CV_CAP_PROP_FPS));
			
			drawLimit(DrawFrame);
			
			rectS = scaleRect(rectS, cv::Size2f(1/scaleFactor.width, 1/scaleFactor.height));
			drawRotatedRect(DrawFrame, rectS,CV_RGB(0,255,0),2);
			
			//checking ini time queue
			if (!startTimerQueue) {
				startTimerQueue = checkStartLimit(rectS, DrawFrame);
				iniFrame = (startTimerQueue ? lastFrame : 1);
			}
			else {
				drawTime(DrawFrame, calculateQueueTime(iniFrame, lastFrame, cap[i].get(CV_CAP_PROP_FPS)));
			}
			lastFrame++;
			
			t = ((double)getTickCount() - t)/getTickFrequency();
			cout <<"Frame: "<<setw(3)<<cap[i].get(CV_CAP_PROP_POS_FRAMES)<< " Prob: "<<prob2<<" Time elaps: "<<t<<endl;
			
			imshow(WINDOW_NAME,DrawFrame);
			video<<DrawFrame;
			
			//checking out of image
			if (/*rectS.center.x+rectS.size.width/2 >= DrawFrame.size().width-10 || */rectS.center.x-rectS.size.width/2 <= 10 /*||
				rectS.center.y+rectS.size.height/2 >= DrawFrame.size().height-10*/) {
				countRectOut++;
				
				if (countRectOut>outTreshold) {
					cout << "OBJECT IS OUT OF VIDEO"<<endl;
					break;
				}
			}
			else
				countRectOut = (countRectOut==0?0:countRectOut-1);
		}
		
		if (i<1) {
			cap[i+1].set(CV_CAP_PROP_POS_FRAMES,cap[i].get(CV_CAP_PROP_POS_FRAMES));
		}
		
		OriginalFrame.release();
		miniFrame.release();
		DrawFrame.release();
		
		
	}
	
	cout << "FINAL QUEUE WAITING TIME:" << calculateQueueTime(iniFrame, lastFrame,cap[0].get(CV_CAP_PROP_FPS))<< " seg"<< endl;
	
	ofstream myfile;
	string path = param[0].videoPath.substr(0, param[0].videoPath.find_last_of("/")+1) +"final_queue_waiting_time.txt"; 
	myfile.open (path.c_str(), ios::trunc);
	myfile << calculateQueueTime(iniFrame, lastFrame,cap[0].get(CV_CAP_PROP_FPS))<< " seg"<<endl;
	myfile <<"outTreshold: "<<outTreshold;
	myfile.close();
	
	
	cap[0].release();
	cap[1].release();
	cvDestroyWindow(WINDOW_NAME);
	delete TrackingModel_P;
	delete et_P;
	
    return 0;
}
