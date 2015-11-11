#include "trackingC.h"


trackingC::trackingC(CvPoint position, double color, double size)
{
	this->position = position;
	approxSize = size;

	this->dTraveled.x = 0;
	this->dTraveled.y = 0;
		
	searchRectangle.width = approxSize+1.6*this->dTraveled.x;
	searchRectangle.height = approxSize+1.6*this->dTraveled.y;

	meanColor = color;
	
	isVisible = false;
	isOcluded = false;
	isGone = false;
	isMoved = false;

	pTraveled = 0;
}

bool trackingC::isRecognized(CvPoint position, double color, double size){
	if(((color - 5) < meanColor) && (meanColor  < (color + 5))){
			return true;
	}
	return false;
}

bool trackingC::update(CvPoint position, double color, double size, bool mode){
	if(!mode){
		viewed = true;
		visible = isVisible = true;
		gone = isOcluded = isGone = false;
	}
	else if(mode && tracked && ((color - 5) < meanColor) && (meanColor  < (color + 5))){
		viewed = true;	
		visible = isVisible = true;
		gone = isOcluded = isGone = false;
		
		this->dTraveled.x = 0.99*this->dTraveled.x + this->position.x-position.x;
		this->dTraveled.y = 0.99*this->dTraveled.y + this->position.y-position.y;
		
		this->position.x = position.x;
		this->position.y = position.y;
		approxSize = size;
		
		searchRectangle.width = approxSize+1.6*this->dTraveled.x;
		searchRectangle.height = approxSize+1.6*this->dTraveled.y;

		visible = true;
		timerReset = 0;

		pTraveled = sqrt((double) ((dTraveled.x*dTraveled.x) + (dTraveled.y*dTraveled.y)));

		if( pTraveled > 30){
			isMoved = true;
			inMotion = true;
			
			dTraveled.x = dTraveled.y = 0;
		}
		else{
			inMotion = isMoved = false;
		}

		return true;
	}else if(tracked){
		viewed = false;
		isMoved = inMotion = false;
		isVisible = visible;
	}
	return false;
}

int trackingC::step(IplImage* img){
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1, 1);

	if(tracked){
		if(!gone && visible && !inMotion){
			cvDrawCircle(img, this->position, approxSize, cvScalar(0, 0, 0), -1);
			cvDrawLine(img, position, cvPoint(position.x+dTraveled.x, position.y+dTraveled.y), cvScalar(255,255,255), 3);
			cvPutText(img,  "D", cvPoint(position.x, position.y-10), &font, cvScalar(0,255,0));
		}else if (!gone && visible && inMotion){
			cvDrawCircle(img, this->position, approxSize, cvScalar(200, 100, 0), 3);
			cvPutText(img,  "M", cvPoint(position.x, position.y-10), &font, cvScalar(0,255,0));
		}else if(!gone){
			cvDrawCircle(img, this->position, approxSize, cvScalar(255, 255, 255), -1);
			cvPutText(img,  "X", cvPoint(position.x, position.y-10), &font, cvScalar(0,255,0));
		}
	}

	if(!viewed){
		timerReset ++;
		isVisible = false;
		if(timerReset > 10 && timerReset < 60){
			if(visible)
				isOcluded = true;
			visible = false;
		}
		else if(timerReset >= 60){
			if(!gone){
				isGone = true;
				isOcluded = false;
			}else{
				isGone = false;
			}
			gone = true;
			
		}
	}

	viewed = false;

	return 0;
}

string trackingC::getNameIfActivated(){
	if(isVisible && !isMoved && !isGone && !isOcluded){
		return "Stationary";
	}else if(isVisible && isMoved && !isGone && !isOcluded){
		return "Moving";
	}else if(isOcluded && !isGone){
		return "Ocluded";
	}else if(isGone){
		return "Gone";
	}else
		return "nothingNew";
}


trackingC::~trackingC(void)
{
}
