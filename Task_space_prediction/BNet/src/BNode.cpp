/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "BNode.h"

BNode::BNode(string i_name, int i_numberOfStates, string i_color){
	this->name = i_name;
	this->numberOfStates = i_numberOfStates;
	
	for(auto count = 0; count < numberOfStates; ++count){
		stateActivations.push_back(0);
	}
	
	currentState = -1;
	
	setColorFromString(i_color);	
}
    
/*setState
  * input: new state
  * return: 1 if change ok, 0 if no change
  */
bool BNode::setState(int i_newState){
	bool change = false;
	
	if(this->currentState != i_newState){
		change = true;
		currentState = i_newState;
		if(i_newState != -1){
			stateActivations[currentState] ++;
		}	
	}
	return change;
}

//Get methods listed below


int BNode::getState(){
	return this->currentState;
}

int BNode::getNumberOfStates(){
	return this->numberOfStates;
}

int BNode::getActivationNumber(int stateID){
	return this->stateActivations[stateID];
}

string BNode::getName(){
	return this->name;
}

char* BNode::getColor(){
	return NULL;
}



BNode::~BNode(){
  
}

//Display methods listed bellow


void BNode::setColorFromString(string i_color){
	if(i_color.find("black") != std::string::npos){
		RGB[0] = 0; RGB[1] = 0; RGB[2] = 0;
	}else if(i_color.find("red") != std::string::npos){
		RGB[0] = 254; RGB[1] = 0; RGB[2] = 0;
	}else if(i_color.find("blue") != std::string::npos){
		RGB[0] = 0; RGB[1] = 0; RGB[2] = 254;
	}else if(i_color.find("green") != std::string::npos){
		RGB[0] = 0; RGB[1] = 254; RGB[2] = 0;
	}else if(i_color.find("purple") != std::string::npos){
		RGB[0] = 128; RGB[1] = 0; RGB[2] = 128;
	}else if(i_color.find("orange") != std::string::npos){
		RGB[0] = 254; RGB[1] = 140; RGB[2] = 0;
	}else if(i_color.find("yellow") != std::string::npos){
		RGB[0] = 255; RGB[1] = 215; RGB[2] = 0;
	}else if(i_color.find("pink") != std::string::npos){
		RGB[0] = 255; RGB[1] = 105; RGB[2] = 180;
	}else if(i_color.find("gray") != std::string::npos){
		RGB[0] = 67; RGB[1] = 67; RGB[2] = 67;
	}
}


