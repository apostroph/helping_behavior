/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "Recognition.h"


Recognition::Recognition(ResourceFinder &rf)
{
}

Recognition::~Recognition()
{
}


bool Recognition::configure(yarp::os::ResourceFinder &rf) {    
    
	bool    bEveryThingisGood = true;

	//init the network
	Network::init();
	
	YARP_REGISTER_DEVICES(icubmod);

	//bEveryThingisGood = initRobotHead(rf);

	bEveryThingisGood = initGaze(rf);

	bEveryThingisGood = initPorts(rf);

	bEveryThingisGood = initBlobExtractor(rf);

	pastImg = currentImg = NULL;
	
	cameraT = NULL;
	
	return bEveryThingisGood ;     
}

/* 
 * Configure method. Receive a previously initialized
 * resource finder object. Use it to configure your module.
 * If you are migrating from the old Module, this is the 
 * equivalent of the "open" method.
 */
 
bool Recognition::initRobotHead(yarp::os::ResourceFinder &rf){
	//Connect to the robot head
	
	/*bool b_robot = rf.find("realRobot").asInt();
	cout<<"Using real robot: "<<b_robot<<endl;
	
	Property options;
	options.put("device", "remote_controlboard");
	options.put("local", "/mover/motor/client");
	
	if(!b_robot)
		options.put("remote", "/icubSim/head");
	else
		options.put("remote", "/icub/head");
	robotHead = new PolyDriver(options);

    if (!robotHead->isValid()) 
    {
        printf("Cannot connect to robot head\n");
        return false;
    }
    robotHead->view(pos);
    robotHead->view(vel);
    robotHead->view(enc);
    if (pos==NULL || vel==NULL || enc==NULL) 
    {
        printf("Cannot get interface to robot head\n");
        robotHead->close();
        return false;
    }
    
    int jnts = 0;
    pos->getAxes(&jnts);
    setpoints.resize(jnts);
    
    setpoints[0] = -25;
    pos->positionMove(setpoints.data());*/
    
    return true;
}

bool Recognition::initGaze(yarp::os::ResourceFinder &rf){
	//For gaze
	
	Property optionsGaze;
	optionsGaze.put("device","gazecontrollerclient");
	optionsGaze.put("remote","/iKinGazeCtrl");
	optionsGaze.put("local","/client/gaze");

	robotHeadGaze = new PolyDriver(optionsGaze);
	head = NULL;
	if (robotHeadGaze->isValid()) {
		cout<<"Head opened properly"<<endl;
		robotHeadGaze->view(head);
	}
	
	finishedGaze = true;
	goodGaze = false;
	actionDone = true;
	lookAtHand = false;
	
    return true;
}

bool Recognition::initPorts(yarp::os::ResourceFinder &rf){
	moduleName            = rf.check("name", 
                           Value("Recognition"), 
                           "module name (string)").asString();
        /*
    * before continuing, set the module name before getting any other parameters, 
    * specifically the port names which are dependent on the module name
    */
    setName(moduleName.c_str());

    // Open cam left
	camLeftName = "/";
    camLeftName += getName() + "/in/left";

    if (!camLeft.open(camLeftName.c_str())) {           
        cout << getName() << ": Unable to open port " << camLeftName << endl;  
        return false;
    }
   
	portAckName = "/";
    portAckName += getName() + "/out";

    if (!portAck.open(portAckName.c_str())) {           
        cout << getName() << ": Unable to open port " << portAckName << endl;  
        return false;
    }
   
	portInName = "/";
    portInName += getName() + "/in";

    if (!portIn.open(portInName.c_str())) {           
        cout << getName() << ": Unable to open port " << portInName << endl;  
        return false;
    }
    attach(portIn);
    
    return true;
}



bool Recognition::initBlobExtractor(yarp::os::ResourceFinder &rf){
	//blobs extraction parameters
	params.minDistBetweenBlobs = 0.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 5.0f;
	params.maxArea = 10000.0f;

	blob_detector = new cv::SimpleBlobDetector(params);
	blob_detector->create("SimpleBlob");
	
	handSeeCount = 0;
	
	return true;
}

bool Recognition::interruptModule() {
    return true;
}

bool Recognition::close() {
	
	head->stopControl();

	delete head;

    return true;
}

bool Recognition::respond(const Bottle& command, Bottle& reply) {
   if(command.get(0).asString() == "actionStarted"){
		actionDone = false;
	}
	else if (command.get(0).asString() == "actionStoped"){
		actionDone = true;
	}
	
	if(command.get(0).asString() == "observ"){
		lookAtHand = true;
	}else if(command.get(0).asString() == "stopObserv"){
		lookAtHand = false;
	}
		
	reply.clear();
	reply.addString("Recongition module over");
	
   return true;
}

/* Called periodically every getPeriod() seconds */
bool Recognition::updateModule() {
	
    //Here is blocking. What is the non blocking function?
    imgLeft = camLeft.read(false);
    if(imgLeft != NULL){
	    left = cvCreateImage(cvSize(imgLeft->width(),imgLeft->height()),IPL_DEPTH_8U, 3 ); //Create an IplImage*
	    cvCvtColor((IplImage*)imgLeft->getIplImage(), left, CV_RGB2BGR);
	    Mat leftM = cv::cvarrToMat(left);
		
	    if(cameraT == NULL){
			cameraT = new camera(left);
		}else if(actionDone && left != NULL){
			cameraT->getImg(left);
			cameraT->getAllObject(actionDone);
			//cameraT->getAllObject(true);
				
			if(cameraT->getNbObj() > (int)certaintyLevels.size()){
				int nbIter = cameraT->getNbObj() - certaintyLevels.size();
				for(int count = 0; count < nbIter; count++){
					certaintyLevels.push_back(0);		
					previousNames.push_back("");
				}
			}
			
			for(int count = 0; count < cameraT->getNbObj(); count++){
				string name = "";
				name = cameraT->getObservation(count);
				if(name != "nothingNew"){
					if(previousNames[count] == name){
						certaintyLevels[count] ++;
					}else{
						certaintyLevels[count] = 0;
						previousNames[count] = name;
					}
				}else{
					previousNames[count] == "";
					certaintyLevels[count] = 0;
				}
				if(((certaintyLevels[count] >= 10 && certaintyLevels[count]%10 == 0) || name.find("Moving") != std::string::npos) && actionDone){
					Bottle ACKNode;
					ACKNode.addString("node");
					ACKNode.addInt(2);
					switch(cameraT->getShape(count)){
						case 0: ACKNode.addString("Marker"); break;
						case 1: ACKNode.addString("Car");  break;
					}
					ACKNode.addString(name);
					if((double)cameraT->getPosition(count) > cameraT->getRaw()->width/2.){
						ACKNode.addString("Right");
					}else{
						ACKNode.addString("Left");
					}
					portAck.write(ACKNode);
				}
			}
			int tempPosition = 0;
			if(lookAtHand && actionDone){
			tempPosition = cameraT->getHandPosition();
				if(tempPosition != -1 && positionHand == tempPosition){
					handSeeCount ++;
					if(handSeeCount > 30){
						bool send = true;
						handSeeCount = 0;
						Bottle ACKNode;
						ACKNode.addString("observation");
						ACKNode.addInt(0);
						switch(positionHand){
							case 0: ACKNode.addString("State_Reach_Push_Left"); break;
							case 1: ACKNode.addString("State_Reach_Left"); break;
							case 2: ACKNode.addString("State_Reach_Right"); break;
							case 3: ACKNode.addString("State_Reach_Push_Right"); break;
							default: send = false; break;
						}
						if(send)
							portAck.write(ACKNode);
							
					}
				}else{
					handSeeCount = 0;
				}
			}else{
				handSeeCount = 0;
			}
			positionHand = tempPosition;
		}
		
		if(left != NULL){
			moveHeadTo();
			cvWaitKey(1);	
			cvReleaseImage(&left);
		}	
    }
	
    return true;
}

double Recognition::getPeriod() {
    /* module periodicity (seconds), called implicitly by myModule */    
    return 0.05;
}

//Head methods

void Recognition::lookingAround(){
}

void Recognition::moveHeadTo(){
	
	goodGaze = true;
	
	yarp::sig::Vector explorationGaze(3);
    
	head->getFixationPoint(explorationGaze);
	
	if(explorationGaze[0] < -0.45 || explorationGaze[0] > -0.35){
		goodGaze = false;
	}else if(explorationGaze[1] < -0.025 || explorationGaze[1] > 0.075){
		goodGaze = false;
	}else if(explorationGaze[2] < -0.05 || explorationGaze[2] > 0.05){
		goodGaze = false;
	}else{
		goodGaze = true;
	}
	if(!goodGaze){
		explorationGaze[0]=-0.40;
		explorationGaze[1]=+0.05;
		explorationGaze[2]=+0.00; 
	   
		if(head != NULL && finishedGaze){
			head->clearNeckPitch();
			head->lookAtFixationPoint(explorationGaze);
		}
		finishedGaze = head->waitMotionDone(0.01, 0.02);
	}
}

Point Recognition::getCenterOfColoredObject(int color, Mat imgOriginal, double *size){
	Point center;
	return center;
}


