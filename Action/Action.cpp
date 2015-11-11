/*
* Copyright: none
* Author: Jimmy Baraglia
* CopyPolicy: All copy allowed
*/

#include "Action.h"

#define TABLE_HEIGHT 0.05
#define PUSH_DOWN_RIGHT 0.06
#define COVER_DOWN_RIGHT 0.06
#define PUSH_DOWN_LEFT 0.04
#define COVER_DOWN_LEFT 0.04

YARP_DECLARE_DEVICES(icubmod)

Action::Action(ResourceFinder &rf) : cartesianCtrlLeft(NULL), cartesianCtrlRight(NULL), clientCartCtrlLeft(NULL), clientCartCtrlRight(NULL) {
	
}

Action::~Action(){
	
}

bool Action::openPorts(yarp::os::ResourceFinder &rf){
	YARP_REGISTER_DEVICES(icubmod);

	//init the network
	yarp = new Network();
	Network::init();
	
    moduleName            = rf.check("name", 
                           Value("Action"), 
                           "module name (string)").asString();
        /*
    * before continuing, set the module name before getting any other parameters, 
    * specifically the port names which are dependent on the module name
    */
    setName(moduleName.c_str());
    
    // Open OUT
	portAckName = "/";
    portAckName += getName() + "/out";

    if (!portAck.open(portAckName.c_str())) {           
        cout << getName() << ": Unable to open port " << portAckName << endl;  
        return false;
    }
    
    // Open RPC
	armPositionName = "/";
    armPositionName += getName() + "/rpc";

    if (!armPosition.open(armPositionName.c_str())) {           
        cout << getName() << ": Unable to open port " << armPositionName << endl;  
        return false;
    }
    attach(armPosition);
    
	return true;
}

bool Action::openControllers(yarp::os::ResourceFinder &rf){
	bool b_robot = rf.find("realRobot").asInt();
	cout<<"Using real robot: "<<b_robot<<endl;
	
	if(b_robot){
		remoteNameLeft="/icub/cartesianController/left_arm";
		remoteNameRight="/icub/cartesianController/right_arm";
	}
	else{
		remoteNameLeft="/icubSim/cartesianController/left_arm";
		remoteNameRight="/icubSim/cartesianController/right_arm";
	}
		
	localNameLeft="/client/left_arm";
	localNameRight="/client/right_arm";
	
	openCartCon(localNameRight, remoteNameRight, &clientCartCtrlRight, &cartesianCtrlRight, &dofRight);
	cartesianCtrlRight->getPose(homePositionRight,homeOrientationRight);
	    
	openCartCon(localNameLeft, remoteNameLeft, &clientCartCtrlLeft, &cartesianCtrlLeft, &dofLeft);
	cartesianCtrlLeft->getPose(homePositionLeft,homeOrientationLeft);
	
	if(cartesianCtrlLeft != NULL && cartesianCtrlRight != NULL){
		cout<<"I am not null"<<endl;
		cartesianCtrlLeft->setPosePriority("position");
		cartesianCtrlRight->setPosePriority("position");
	}else{
		cerr<<"I am null"<<endl;
	}
	
	return true;
}


bool Action::openCartCon(string localName, string remoteName, PolyDriver **clientCartCtrl, ICartesianControl **cartesianCtrl, yarp::sig::Vector* dof){
	Property option("(device cartesiancontrollerclient)");
	option.put("remote",remoteName.c_str());
	option.put("local",localName.c_str());	

	*clientCartCtrl = new PolyDriver(option);

	*cartesianCtrl = NULL;

	if ((*clientCartCtrl)->isValid()) {
		(*clientCartCtrl)->view(*cartesianCtrl);
		cout<<"CartesianControl Valid"<<endl;
	}else{
		cout<<"CartesianControl non Valid"<<endl;
		return false;
	}
	   
	(*cartesianCtrl)->setPosePriority("position");

	(*cartesianCtrl)->setTrajTime(1.0);
	(*cartesianCtrl)->getDOF(*dof);


	Vector newDof(3);
	newDof[0]=0; // torso pitch: 1 => enable
	newDof[1]=0; // torso roll: 2 => skip
	newDof[2]=0; // torso yaw: 1 => enable
	(*cartesianCtrl)->setDOF(newDof,*dof);

	return true;
	
}

bool Action::configure(yarp::os::ResourceFinder &rf){
	
	bool bEveryThingisGood = true;
	
	
    //ICartesianControllers
	bEveryThingisGood = openPorts(rf);
	bEveryThingisGood = openControllers(rf);
	//Environment meta-data
	tableHeight = 0.5;
	posCount = 0;
	
	cout<<"Init Action over"<<endl;
    
    return bEveryThingisGood;
}

bool Action::interruptModule(){
	return true;
}   

bool Action::close(){
	cout<<"Closing"<<endl;
	clientCartCtrlLeft->close();
	clientCartCtrlRight->close();
	return true;
}

bool Action::respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply){
	if(command.get(0).asString() == "Act"){
		switch(command.get(1).asInt()){
			case 1: cover_Left(); break;
			case 2: cover_Right(); break;
			case 3: push_Left(); break;
			case 4: push_Right(); break;
		}
	}
	else if(command.get(0).asString() == "helping"){
		cout<<command.get(1).asString().substr(6)<<endl;
		if(command.get(2).asString() == "Right"){
			actionPrimitive(command.get(1).asString().substr(6));
			usleep(500000);
		}
		else if(command.get(2).asString() == "Left"){
			actionPrimitive(command.get(1).asString().substr(6));
			usleep(500000);
		}
	}
	sendHomeNode("Home"); //Send Node
	goHome();
	
	reply.clear();
	reply.addString("Action module over");
	return true;
}

double Action::getPeriod(){
	return 2;
}

void Action::sendHomeNode(string name){
	Bottle ACKNode2;
	ACKNode2.addString("home");
	ACKNode2.addInt(0);
	ACKNode2.addString("State_"+name);
	portAck.write(ACKNode2);
}

void Action::sendActionNode(string name){
	Bottle ACKNode2;
	ACKNode2.addString("node");
	ACKNode2.addInt(0);
	ACKNode2.addString("State_"+name);
	portAck.write(ACKNode2);
}

void Action::cover_Left(){
    
    actionPrimitive("Reach_Left");
    
    actionPrimitive("Cover_Left");
	
	usleep(2000000);
}

void Action::cover_Right(){
	//cartesianCtrlLeft->goToPose(homePositionLeft,homeOrientationLeft);
    
    actionPrimitive("Reach_Right");
    
    actionPrimitive("Cover_Right");

	usleep(2000000);
}

void Action::push_Left(){
    
    actionPrimitive("Reach_Push_Left");
    
    actionPrimitive("Push_Left");
	
	usleep(2000000);
}

void Action::push_Right(){
    
    actionPrimitive("Reach_Push_Right");
    
    actionPrimitive("Push_Right");
	
	usleep(2000000);
}

void Action::start(){
	Bottle ACKNode2;
	ACKNode2.addString("actionStarted");
	portAck.write(ACKNode2);
}

void Action::stop(){
	Bottle ACKNode2;
	ACKNode2.addString("actionStoped");
	portAck.write(ACKNode2);
}

bool Action::enableBody(ICartesianControl* cartesianCtrl, yarp::sig::Vector* dof){
	Vector newDof(3);
	
	newDof[0]=1; // torso pitch: 1 => enable
	newDof[1]=0; // torso roll: 2 => skip
	newDof[2]=1; // torso yaw: 1 => enable
	
	return cartesianCtrl->setDOF(newDof,*dof);
}

bool Action::desableBody(ICartesianControl* cartesianCtrl, yarp::sig::Vector* dof){
	Vector newDof(3);
	
	newDof[0]=0; // torso pitch: 1 => skip
	newDof[1]=0; // torso roll: 2 => skip
	newDof[2]=0; // torso yaw: 1 => skip
	
	return cartesianCtrl->setDOF(newDof,*dof);
}

void Action::moves(ICartesianControl *icart, double x, double y, double z, double orientation, double push)
{
	Vector xd, od;
	icart->getPose(xd,od);
	double angle = (orientation*2.*3.14)/360.;
	
	xd[0] = x;  xd[1] = y;  xd[2] = z; 
	
	yarp::sig::Matrix r(3,3);
	r.zero();
	
	r(0,0) = -1; r(0,1) = 0.0; r(0,2) = 0.0;
	r(1,0) = 0.0; r(1,1) = orientation; r(1,2) = push;
	r(2,0) = 0.0; r(2,1) = -1+(abs(orientation)); r(2,2) = -1.0+abs(push);

	od = dcm2axis(r);

	start();
	icart->goToPose(xd, od);   // send request and forget
	stop();
	
}

void Action::goHome(){
	bool done = false;
	moves(cartesianCtrlLeft, -0.25, -0.2, TABLE_HEIGHT, 0, -1); //home Left
	while(!done){
		cartesianCtrlLeft->waitMotionDone(0.04);
		cartesianCtrlLeft->checkMotionDone(&done);
	}
	
	done = false;
	moves(cartesianCtrlRight, -0.25, 0.2, TABLE_HEIGHT, 0, -1); //Home Right
	while(!done){
		cartesianCtrlRight->waitMotionDone(0.04);
		cartesianCtrlRight->checkMotionDone(&done);
	}
}

void Action::actionPrimitive(string name){
	
	sendActionNode(name); //Send Node
	
	if(name == "Reach_Right"){
		moves(cartesianCtrlRight, -0.25, 0.2, TABLE_HEIGHT, 1, 0); //Reach Right to cover
		waitUntilTimeOut(500000, cartesianCtrlRight);
	}else if(name == "Cover_Right"){	
		actionPrimitive("Reach_Right");
		moves(cartesianCtrlRight, -0.25, 0.10, TABLE_HEIGHT-COVER_DOWN_RIGHT, 1, 0); //Cover Right
		waitUntilTimeOut(500000, cartesianCtrlRight);
		actionPrimitive("Reach_Right");
	}else if(name == "Reach_Left"){	
		moves(cartesianCtrlLeft, -0.25, -0.2, TABLE_HEIGHT, -1, 0); //Reach Left to cover
		waitUntilTimeOut(500000, cartesianCtrlLeft);
	}else if(name == "Cover_Left"){	
		actionPrimitive("Reach_Left");
		moves(cartesianCtrlLeft, -0.25, -0.10, TABLE_HEIGHT-COVER_DOWN_LEFT, -1, 0);
		waitUntilTimeOut(500000, cartesianCtrlLeft);
		actionPrimitive("Reach_Left");
	}else if(name == "Reach_Push_Right"){
		moves(cartesianCtrlRight, -0.25, 0.22, TABLE_HEIGHT-PUSH_DOWN_RIGHT, 0, -1); //Reach Right to push
		waitUntilTimeOut(500000, cartesianCtrlRight);
	}else if(name == "Push_Right"){	
		actionPrimitive("Reach_Push_Right");
		moves(cartesianCtrlRight, -0.25, 0.1, TABLE_HEIGHT-PUSH_DOWN_RIGHT, 0, -1); //Push Right
		waitUntilTimeOut(500000, cartesianCtrlRight);
		actionPrimitive("Reach_Push_Right");
	}else if(name == "Reach_Push_Left"){	
		moves(cartesianCtrlLeft, -0.25, -0.22, TABLE_HEIGHT-PUSH_DOWN_LEFT, 0, -1); //Reach Left to push
		waitUntilTimeOut(500000, cartesianCtrlLeft);
	}else if(name == "Push_Left"){   
		actionPrimitive("Reach_Push_Left");
		moves(cartesianCtrlLeft, -0.25, -0.1, TABLE_HEIGHT-PUSH_DOWN_LEFT, 0, -1); //Push Left
		waitUntilTimeOut(500000, cartesianCtrlLeft);
		actionPrimitive("Reach_Push_Left");
	}
	
	//cartesianCtrlRight->setPosePriority("position");
	//cartesianCtrlLeft->setPosePriority("position");
	
}

bool Action::waitUntilTimeOut(double milliSec, ICartesianControl* cartesianCtrl){
	double timer = 0;
	bool motionSuccess = true;
	
	while(!cartesianCtrl->waitMotionDone(0.02, 0.1)){
		if(timer >= 5){
			cerr<<"Warning: Action not fully completed"<<endl;
			motionSuccess = false;
			break;
		}
		timer += 0.1;
	}
	
	usleep(milliSec);
	
	return motionSuccess;
}

bool Action::updateModule(){
	return true;
}

