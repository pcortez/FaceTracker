/*
 *  frameObject.cpp
 *  FaceTracker
 *
 *  Created by Pedro Cortez Cargill on 17-11-10.
 *  Copyright 2010 PUC. All rights reserved.
 *
 */

#include "frameObject.h"

frameObject::frameObject(string _path){
	iniCap(_path);
}

frameObject::frameObject(){
}

frameObject::~frameObject(){
	video.release();
	filesPath.clear();
}

void frameObject::iniCap(string _path){
	if (_path[_path.length()-1]=='/' || _path[_path.length()-1]=='\\'){
		indexFrame = 0;
		this->isVideo = false;
		
		DIR *dp;
		struct dirent *dirp;
		if((dp = opendir(_path.c_str())) == NULL) {
			cout << "Error: "<<_path<< endl;
			CV_Assert(false);
		}
		
		//READING IMAGE
		while ((dirp = readdir(dp)) != NULL) {
			size_t posJpeg = string(dirp->d_name).find("jpeg");
			size_t posJpg = string(dirp->d_name).find("jpg");
			size_t posPng = string(dirp->d_name).find("png");
			size_t posPpm = string(dirp->d_name).find("ppm");
			
			if(posJpeg!=string::npos || posJpg!=string::npos || posPng!=string::npos || posPpm!=string::npos ){
				string dirFull;
				dirFull += _path;
				dirFull += "/";
				dirFull +=string(dirp->d_name);
				filesPath.push_back(dirFull);
			}
		}
		closedir(dp);
		sort(filesPath.begin(), filesPath.end());
		
	}
	else {
		this->isVideo = true;
		if (!this->video.open(_path)){
			cout << "WRONG FLAG -configPathRV: "<< _path <<endl;
			CV_Assert(false);
		}
		if (!this->video.isOpened()) {
			cout << "WRONG PATH -topVideo:"<< _path <<endl;
			CV_Assert(false);
		}
	}
}

Mat frameObject::getNextFrame(){
	Mat nextFrame;
	if(this->isVideo)
		this->video>> nextFrame;
	else{ 
		nextFrame = imread(filesPath[indexFrame]);
		indexFrame++;
	}
	return nextFrame;
}

void frameObject::setSeeker(double value, bool isFrame){
	if(this->isVideo){
		if(isFrame)
			video.set(CV_CAP_PROP_POS_FRAMES, value-3);
		else
			video.set(CV_CAP_PROP_POS_MSEC, value);
	}
	else{ 
		if(isFrame)
			video.set(CV_CAP_PROP_POS_FRAMES, value);
		else{
			cout << "IMAGE CANNOT USE MIN:SEC FORMAT: "<< value <<endl;
			CV_Assert(false);
		}
	}
	
}

int frameObject::getSeeker(){
	if(this->isVideo)
		return video.get(CV_CAP_PROP_POS_FRAMES);
	else
		return indexFrame;
}

double frameObject::getFPS(){
	if(this->isVideo)
		return video.get(CV_CAP_PROP_FPS);
	else
		return 23;
}