#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <math.h>

#include <cv.h>
#include <highgui.h>
#pragma once

using namespace std;
class trackingC
{
public:
	trackingC(CvPoint position, double color, double size);
		
	bool isRecognized(CvPoint position, double color, double size);
	bool update(CvPoint position, double color, double size, bool mode);
	int step(IplImage* img);
	
	string getNameIfActivated();
	
	CvPoint getPosition(){return position;}

	~trackingC(void);

private:

	CvPoint position;
	CvPoint dTraveled;

	double pTraveled;

	CvRect searchRectangle;

	double approxSize;

	double meanColor;

	bool tracked;
	bool viewed;
	bool gone;
	bool visible;
	int timerReset;
	bool inMotion;
	
	bool isVisible;
	bool isOcluded;
	bool isGone;
	bool isMoved;
};

