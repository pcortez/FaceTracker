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
	if(!iniCap(_path)){
		cout << "ERROR ON PATH LOADING VIDEO"<<endl;
		CV_Assert(false);
	}
}

frameObject::frameObject(){
}

void frameObject::release(){
	video.release();
	filesPath.clear();
}

bool frameObject::iniCap(string _path){
	if (_path[_path.length()-1]=='/' || _path[_path.length()-1]=='\\'){
		indexFrame = 0;
		this->isVideo = false;
		
		DIR *dp;
		struct dirent *dirp;
		if((dp = opendir(_path.c_str())) == NULL)
			return false;
		
		
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
		if (!this->video.open(_path))
			return false;
		if (!this->video.isOpened())
			return false;
		
	}
	
	return true;
}

bool frameObject::grabNextFrame(){
	if(this->isVideo)
		return this->video.grab();
	else{
		if(indexFrame>=filesPath.size())
			return false;
		else{
			indexFrame++;
			return true;
		}
	}
}

Mat frameObject::getNextFrame(){
	Mat nextFrame;
	if(this->isVideo)
		this->video.retrieve(nextFrame);
	else{
		if (indexFrame>=filesPath.size()){
			cout << "INI FRAME EXCEDE NUM IMAGES"<<endl;
			CV_Assert(false);
		}
		
		nextFrame = imread(filesPath[indexFrame]);
		
	}
	return nextFrame;
}

void frameObject::setSeeker(double value, bool isFrame){
	if(this->isVideo){
		if(isFrame)
			video.set(CV_CAP_PROP_POS_FRAMES, value-1);
		else
			video.set(CV_CAP_PROP_POS_MSEC, value);
	}
	else{ 
		if(isFrame)
			indexFrame = floor(value-1);
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