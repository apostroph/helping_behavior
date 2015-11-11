/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "BEdge.h"

BEdge::BEdge(BNode *i_from, BNode *i_to, string i_color){
	this->parent = i_from;
	this->child = i_to;
	
	setColorFromString(i_color);	
}

//Get methods listed below

BNode* BEdge::getParent(){
	return this->parent;
}

BNode* BEdge::getChild(){
	return this->child;
}



BEdge::~BEdge(){
  
}

//Display methods listed bellow


void BEdge::setColorFromString(string i_color){
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


