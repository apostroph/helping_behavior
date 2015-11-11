#include "camera.h"

#define DIV 6 //Image size divider (to increase FPS)
#define MAX_CORNER 30

camera::camera(IplImage* img)
{	
	colorHueList.push_back(120); //blue
	colorHueList.push_back(50); //red
	colorHueList.push_back(12); //green
	//colorHueList.push_back(110); //orange
	//colorHueList.push_back(145); //purple
	//colorHueList.push_back(90); //yellow
	colors.push_back(cvScalar(0,0,200)); //blue
	colors.push_back(cvScalar(0,200,0)); //red
	colors.push_back(cvScalar(200,0,0)); //green
	//colors.push_back(cvScalar(0,100,200)); //orange
	//colors.push_back(cvScalar(200,0,200)); // purple
	//colors.push_back(cvScalar(0,200,200)); //yellow
	colorName.push_back("blue");	
	colorName.push_back("red");	
	colorName.push_back("green");	
	//colorName.push_back("orange");	
	//colorName.push_back("purple");	
	//colorName.push_back("yellow");	
	
	// set up the parameters (check the defaults in opencv's code in blobdetector.cpp)
	params.minDistBetweenBlobs = 50.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 20.0f;
	params.maxArea = 500.0f;
	
	//Create trackbars in "Control" window
	for(int count = 0; count < 6; count++)
		Hue.push_back(0);
    cvNamedWindow("Color offset", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	cvCreateTrackbar("red", "Color offset", &Hue[0], 180); //Hue (0 - 179)
	cvCreateTrackbar("green", "Color offset", &Hue[1], 180); //Hue (0 - 179)
	cvCreateTrackbar("blue", "Color offset", &Hue[2], 180); //Hue (0 - 179)
	cvCreateTrackbar("orange", "Color offset", &Hue[3], 180); //Hue (0 - 179)
	cvCreateTrackbar("purple", "Color offset", &Hue[4], 180); //Hue (0 - 179)
	//cvCreateTrackbar("yellow", "Color offset", &Hue[5], 180); //Hue (0 - 179)
	
	// set up and create the detector using the parameters
	blob_detector = new cv::SimpleBlobDetector(params);
	blob_detector->create("SimpleBlob");
	
	rawImage = cvCreateImage(cvSize(img->width,img->height), 8, 3);
	
	handPosition = -1;
	cout<<"Camera Init done"<<endl;
}

void camera::getImg(IplImage* img){
	if(rawImage != NULL){
		cvCopy(img, rawImage);
	}
}

void camera::getAllObject(bool mode){
	
	if(rawImage != NULL){
		IplImage* temp = cvCreateImage( cvSize(rawImage->width/DIV,rawImage->height/DIV), 8, 1);
		IplImage* smallRaw = cvCreateImage( cvGetSize( temp ), 8, 3);
		IplImage* skinImg = cvCreateImage( cvGetSize( temp ), 8, 1);
		cv::Point position;

		cvResize(rawImage, smallRaw);
		saturationThreshold(smallRaw, smallRaw, 0, 180);
		
		cvCvtColor(smallRaw, smallRaw, CV_RGB2HSV);

		for(unsigned int count1 = 0; count1 < colorHueList.size(); count1 ++){
			GetThresholdedImage(smallRaw, temp, (colorHueList[count1]+Hue[count1])%180);
			GetThresholdedImage(smallRaw, skinImg, 110, 30);
			getHandPosition(skinImg);
			position = blobDetector(temp);

			bool tracked = false;
			int index = -1;
			if(trackersList.size() > 0){
				for(unsigned int count = 0; count < trackersList.size(); count++){
					tracked = trackersList[count]->isRecognized(DIV*position, colorHueList[count1], 0);
					if(tracked && position.x > 0 && position.y > 0){
						index = count;
						break;
					}
				}
			}else
				tracked = false;

			if(tracked == false && position.x > 0 && position.y > 0){
				trackingC* newTracker = new trackingC(DIV*position, (colorHueList[count1]+Hue[count1])%180, 10);
				trackersList.push_back(newTracker);
			}else if(position.x > 0 && position.y > 0){
				cvDrawCircle(rawImage, DIV*position, 30, colors[count1], 5);
				trackersList[index]->update(DIV*position, (colorHueList[count1]+Hue[count1])%180, 10, mode);
			}
		}

		for(unsigned int count = 0; count < trackersList.size(); count++){
			trackersList[count]->step(rawImage);
		}
		//cvShowImage("Skin", skinImg);
		cvShowImage("Recognition display", rawImage);
		cvWaitKey(1);
		cvReleaseImage(&temp);
		cvReleaseImage(&skinImg);
		cvReleaseImage(&smallRaw);
	}
}

CvPoint camera::getHandPosition(IplImage* src){
	CvPoint handPositionPoint;
	handPositionPoint = blobDetector(src);
	handPosition = (int)handPositionPoint.x;
		
	if(handPosition > ((3.*src->width)/4.)){
		handPosition = 3;
	}else if(handPosition > ((src->width)/2.)){
		handPosition = 2;
	}else if(handPosition > ((1.*src->width)/4.)){
		handPosition = 1;
	}else if(handPosition > 0){
		handPosition = 0;
	}else{
		handPosition = -1;
	}
	
	if(handPosition != -1){
		cvDrawCircle(rawImage, cvPoint(DIV*handPositionPoint.x, DIV*handPositionPoint.y), 10, cvScalar(255,255,255), 15);
	}
		
	return handPositionPoint;
}

void camera::GetThresholdedImage(IplImage* imgHSV, IplImage* dst, double color, int satMin){        
	cvInRangeS(imgHSV, cvScalar((int)(color-5)%360,50,0), cvScalar((int)(color+5)%360,256,256), dst); 
	
	cvErode(dst, dst, 0, 1);
	cvDilate(dst, dst, 0, 1);
}

string camera::getObservation(int count){
	string out = "";
	if(trackersList[count]->getNameIfActivated()!= "nothingNew"){
		if(count < (int)colorName.size())
			out = colorName[count] + "_";
		else
			out = "error";
	}
	out = out + trackersList[count]->getNameIfActivated();
	return out;
}

int camera::getShape(int count){
	if(colorName[count] == "blue")
		return 1;
	else
		return 0;
}

void camera::skinMask(IplImage* src, IplImage* dst, bool on){
	for(int r = 0; r < src->width*src->height; r++){
		// 0<H<0.25 - 0.15<S<0.9 - 0.2<V<0.95
		if((src->imageData[3*r] > 5) && (src->imageData[3*r] < 17) && (src->imageData[3*r+1] > 38) && (src->imageData[3*r+1] < 250) && (src->imageData[3*r+2] > 51) && (src->imageData[3*r+2] < 242) ){
			for(int i=0; i<3; ++i){
				dst->imageData[3*r+i] = src->imageData[3*r+i];
			}
		}
		else{ 
			for(int i=0; i<3; ++i) {
				dst->imageData[3*r+i] = 0;
			}
		}
	}
}

void camera::saturationThreshold(IplImage* src, IplImage* dst, int min, int max){
	for(int r = 0; r < (src->width*src->height)/3; r++){
		if((src->imageData[r+1] > min) && (src->imageData[r+1] < max)){
			for(int i=0; i<3; ++i){
				dst->imageData[r+i] = src->imageData[r+i];
			}
		}
		else{ 
			for(int i=0; i<3; ++i) {
				dst->imageData[r+i] = 0;
			}
		}
	}
}

CvPoint camera::blobDetector(IplImage* image){
	// ... any other params you don't want default value
	CvPoint point;
	point.x = 0;
	point.y = 0;

	// detect!
	vector<cv::KeyPoint> keypoints;
	blob_detector->detect(image, keypoints);
	// extract the x y coordinates of the keypoints: 
	for (unsigned int i=0; i<keypoints.size(); i++){
		point.x	=keypoints[i].pt.x; 
		point.y	=keypoints[i].pt.y;
	}
	return point;
}

int camera::getPosition(int count){
	CvPoint position = trackersList[count]->getPosition();
	return position.x;
}

camera::~camera(void)
{
}
