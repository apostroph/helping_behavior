/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _Recognition_H_
#define _Recognition_H_

#include <time.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <string>

#include <ace/config.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>	

#include <yarp/dev/Drivers.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/PolyDriver.h>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <opencv/highgui.h>
#include <opencv/cv.h>

#include "camera.h"

//#include <opencv/cvaux.h>
//#include <opencv/highgui.h>
//#include <opencv2/highgui/highgui.hpp>
//#include "opencv2/imgproc/imgproc.hpp"

YARP_DECLARE_DEVICES(icubmod);


using namespace std;
using namespace cv;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::sig::draw;

	
enum colorSelection{red = 120, green = 55, blue = 10, orange = 110, purple = 165, yellow = 90};


class Recognition : public RFModule {

public:
    /** 
     * document your methods too.
     */
    Recognition(ResourceFinder &rf);
    ~Recognition();

    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports 
    bool close();                                 // close and shut down the module
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);
    double getPeriod(); 
    bool updateModule();

private:
    string moduleName;
    Network yarp;
    
    PolyDriver* robotHead;
    PolyDriver* robotHeadGaze;
    IGazeControl *head;
    bool finishedGaze;
    bool goodGaze;
    bool actionDone;
    
    //Initialisation methods
    bool initRobotHead(yarp::os::ResourceFinder &rf);
    bool initGaze(yarp::os::ResourceFinder &rf);
    bool initPorts(yarp::os::ResourceFinder &rf);
    bool initBlobExtractor(yarp::os::ResourceFinder &rf);

    ImageOf<PixelRgb> *imgY;
    
    ImageOf<PixelRgb> *imgLeft;
    ImageOf<PixelRgb> *imgRight;
    
    IplImage *left, *right;
    
    IplImage *currentImg;
    IplImage *pastImg;

    string imagePortName;
    string Port4RecogitionName;
    string portAckName;
    string portInName;

    BufferedPort<ImageOf<PixelRgb> > imagePort;             // a port to get images from iCub 
    yarp::os::Port Port4Recognition;        // a port to send recognition
    yarp::os::Port portAck; // a port to communicate what the system sees
   yarp::os::Port portIn;
    
    string camLeftName;
    string camRightName;
    BufferedPort<ImageOf<PixelRgb> > camLeft;
    BufferedPort<ImageOf<PixelRgb> > camRight;

	//Private method for head control
	void moveHeadTo();
	void lookingAround();
    
    //Private variable for the head control
    IPositionControl *pos;
    IVelocityControl *vel;
    IEncoders *enc;
    int jnts;
    yarp::sig::Vector setpoints;
    yarp::sig::Vector encoders;
    
    bool vUp, vRight;
    std::vector<int> certaintyLevels;
    std::vector<string> previousNames;

    //Private methods for imgProcessing
    Point getCenterOfColoredObject(int color, Mat img, double *size);

    //Private variable for imgProcessing
    
    int iLowH;
    int iHighH;
    int iLowS; 
    int iHighS;
    int iLowV;
    int iHighV;
    
	cv::SimpleBlobDetector::Params params;
	cv::Ptr<cv::FeatureDetector> blob_detector;
	vector<cv::KeyPoint> keypoints;
	
	//Varibale for tracking
	colorSelection colorsEnum;
	camera* cameraT;
	
	int handSeeCount;
	int positionHand;
	bool lookAtHand;
};


#endif // __Recognition_H__

//----- end-of-file --- ( next line intentionally left blank ) ------------------

