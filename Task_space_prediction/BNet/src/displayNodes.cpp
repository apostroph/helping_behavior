/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#define LENGTH 300	

#include "displayNodes.h"

displayNodes::displayNodes(){

}

void displayNodes::displayDBN(BNet* i_inputNet){
	int max_t = 0;
	int max_nodePerTimeStep = 0;
	vector<int> nodePerTimeStep;
	for(int c = 0; c < i_inputNet->getNumberOfNodes(); c++){
		string nodeName = i_inputNet->getNodeId(c)->getName();
		int id = stoi(nodeName.substr(nodeName.find("~t+")+3, -1));
		
		if(id >= nodePerTimeStep.size()){
			nodePerTimeStep.push_back(1);
		}else{
			nodePerTimeStep[id] ++;
		}
		
		id > max_t ? max_t = id : 0;
		
		nodePerTimeStep[id] > max_nodePerTimeStep ? max_nodePerTimeStep = nodePerTimeStep[id] : 0;
	}
	width = (max_t+1)*LENGTH + (max_t)*50;
	height = 50+(max_nodePerTimeStep)*100;
	
	canvas = Mat(Size(width, height), CV_8UC3);
	canvasTop = Mat(Size(width, height), CV_8UC3);
	canvas = Scalar(255, 255, 255);
	canvasTop = Scalar(255, 255, 255);
	
	vector<int> nbPerTimeStep;
	for(int c = 0; c <= max_t; c++)
		nbPerTimeStep.push_back(0);
	
	
	for(int c = 0; c < i_inputNet->getNumberOfNodes(); c++){
		string nodeName = i_inputNet->getNodeId(c)->getName();
		int id = stoi(nodeName.substr(nodeName.find("~t+")+3, -1));
		
		nodeToPosition[nodeName] = Point(LENGTH/2 + LENGTH*id , 100+100*nbPerTimeStep[id]);
		
		drawNodes(nodeName, Point(LENGTH/2 + LENGTH*id , 100+100*nbPerTimeStep[id]),i_inputNet->getNumberOfActivationOfNodeId(c));
		nbPerTimeStep[id] ++;
		
	}
	
	for(int c = 0; c < i_inputNet->getNumberOfNodes(); c++){
		int nbChild = i_inputNet->getNumberOfChildsOf(i_inputNet->getNodeId(c));
		for(int k = 0; k < nbChild; k++){
			drawEdges(nodeToPosition[i_inputNet->getNodeId(c)->getName()], nodeToPosition[i_inputNet->getChildID(i_inputNet->getNodeId(c),k)->getName() ] );
		}
	}
	
	for(int c = 0; c <= max_t; c++){
		putText(canvasTop, "t+"+to_string(c), Point(LENGTH/2 + LENGTH*c - 15, 20), CV_FONT_HERSHEY_COMPLEX, 0.6, Scalar(255, 255, 255));
	}
	
	addWeighted(canvasTop , 1,canvas , 1, 1.0, canvas);
	while(1){
		imshow("network", canvas);
		
		waitKey(1);
	}
}

displayNodes::~displayNodes(){

}

void displayNodes::drawNodes(string name, Point position, int nbActivation){
	circle(canvasTop, Point(position.x, position.y), 30, Scalar(70, 70, 70), -1);
	putText(canvasTop, name, Point(position.x-(LENGTH/2.5)+10, position.y+15), CV_FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 0));
	putText(canvasTop, to_string(nbActivation), Point(position.x, position.y-10), CV_FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 0));
}

void displayNodes::drawEdges(Point left, Point right){
	line(canvas, Point(left.x, left.y), Point(right.x, right.y), Scalar(0, 255, 0), 1);

	int length = sqrt( pow(left.x - right.x, 2) + pow(left.y - right.y, 2) );
	double angle = atan2((right.y-left.y), (right.x-left.x));
	double orientation = M_PI/4;
	for(int c = 0; c < length; c+=20){
		line(canvas, Point(left.x+(c*cos(angle)), left.y+(c*sin(angle))), Point(left.x+(c*cos(angle))-12*cos(orientation+angle), left.y+(c*sin(angle))-12*sin(orientation+angle)), Scalar(0, 255, 0));
		line(canvas, Point(left.x+(c*cos(angle)), left.y+(c*sin(angle))), Point(left.x+(c*cos(angle))-12*cos(orientation-angle), left.y+(c*sin(angle))+12*sin(orientation-angle)), Scalar(0, 255, 0));
	}
	
	rectangle(canvas, Point(left.x-(LENGTH/2.5), left.y-25), Point(left.x+(LENGTH/2), left.y+25), Scalar(0, 0, 0), -1);
	rectangle(canvas, Point(right.x-(LENGTH/2.5), right.y-25), Point(right.x+(LENGTH/2), right.y+25), Scalar(0, 0, 0), -1);
	
}