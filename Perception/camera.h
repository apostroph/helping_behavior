#ifndef CAMERA_H
#define CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <cv.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <math.h>

#include "trackingC.h"

using namespace std;
using namespace cv;

#pragma once
class camera
{
public:
	camera(IplImage* img);
	~camera(void);

	void getImg(IplImage* img);
	void getAllObject(bool mode = true);
	
	int getNbObj(){ return trackersList.size(); }
	string getObservation(int count);
	int getShape(int count);
	int getPosition(int count);
	int getHandPosition(){return handPosition;}
	
	IplImage* getRaw(){ return rawImage;}
	
	CvPoint getHandPosition(IplImage* src);

private:
	void GetThresholdedImage(IplImage* imgHSV, IplImage* dst, double color, int satMin = 120);
	void skinMask(IplImage* src, IplImage* dst, bool on);
	void saturationThreshold(IplImage* src, IplImage* dst, int min, int max);
	CvPoint blobDetector(IplImage* image);
	
	//variables
	std::vector<trackingC*> trackersList;
	std::vector<int> colorHueList;
	std::vector<CvScalar> colors;
	
	//tracking
	cv::SimpleBlobDetector::Params params;
	cv::Ptr<cv::FeatureDetector> blob_detector;
	
	IplImage* rawImage;
	std::vector<int> Hue;
	std::vector<string> colorName;
	
	int handPosition;
};

#endif
