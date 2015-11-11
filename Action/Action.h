/*
* Copyright: none
* Author: Jimmy Baraglia
* CopyPolicy: All copy allowed
*/

#ifndef _ACTION_H_
#define _ACTION_H_

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdarg>
#include <string>
#include <cmath>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/os/Time.h>
#include <yarp/dev/all.h>
#include <yarp/math/Math.h>

#include <time.h>
#include <unistd.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::sig::draw;
using namespace yarp::math;

class Action : public RFModule {

public:
	Action(ResourceFinder &rf);
    ~Action();
    
    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports 
    bool close();                                 // close and shut down the module
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);
    double getPeriod();
    bool updateModule();

private:
    string moduleName;
    Network *yarp;
    
    string remoteNameLeft;
    string remoteNameRight;
	string localNameLeft;
	string localNameRight;
	
	void moves(ICartesianControl *icart, double x, double y, double z, double orientation, double push);
	void sendActionNode(string name);
	void sendHomeNode(string name);
	
    /*void goToPosImg(Point left, Point right);
    void goToPosImg(Point position);
    void goToPosImg(Vector _3Dposition);*/
    
	string portAckName;
    yarp::os::Port portAck; // a port to send POMDP
    
	string armPositionName;
    yarp::os::Port armPosition;
    
    PolyDriver *clientCartCtrlLeft;
    ICartesianControl *cartesianCtrlLeft;
    
    PolyDriver *clientCartCtrlRight;
    ICartesianControl *cartesianCtrlRight;
    
    yarp::sig::Vector dofLeft;
    yarp::sig::Vector dofRight;
    
    yarp::sig::Vector homePositionLeft;
    yarp::sig::Vector homeOrientationLeft;
    yarp::sig::Vector homePositionRight;
    yarp::sig::Vector homeOrientationRight;
    
    //Initialisation methods
    bool openCartCon(string localName, string remoteName, PolyDriver **clientCartCtrl, ICartesianControl **cartesianCtrl, yarp::sig::Vector* dof);
    bool openPorts(yarp::os::ResourceFinder &rf);
    bool openControllers(yarp::os::ResourceFinder &rf);
    
    //Environment meta-data
    double tableHeight;
    int posCount;
    
    //Action types
    void cover_Left();
    void cover_Middle();
    void cover_Right();
    
    void push_Left();
    void push_Middle();
    void push_Right();
    
    bool waitUntilTimeOut(double milliSec, ICartesianControl* cartesianCtrl);
    
    void actionPrimitive(string name);
    void goHome();
    
    bool enableBody(ICartesianControl* cartesianCtrl, yarp::sig::Vector* dof);
    bool desableBody(ICartesianControl* cartesianCtrl, yarp::sig::Vector* dof);
    
    void start();
    void stop();
};

#endif


