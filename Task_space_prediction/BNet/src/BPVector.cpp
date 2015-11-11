/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "BPVector.h"

BPVector::BPVector(){
}

void BPVector::addParent(BNode* i_parent, int i_state){
  
	bool contained = false;
	for(auto i: listOfParents){
		if( i->getName() == i_parent->getName() ){
			contained = true;
			break;
		}
	}
	
	if(!contained){
		listOfParents.push_back(i_parent);
		if(i_state >= 0 && i_state < i_parent->getNumberOfStates()){
			stateMap.insert({{i_parent->getName(), i_state}});
		}else{
			cerr<<"Entered state out of border"<<endl;
		}
	}
}

//return false if i_parent not found and addIfNotFound == false
bool BPVector::set(BNode* i_parent, int i_state, bool addIfNotFound){
	bool contained = false;
	
	for(auto i: listOfParents){
		if( i->getName() == i_parent->getName() ){
			contained = true;
			break;
		}
	}
	
	
	if(contained){
		if(i_state >= 0 && i_state < i_parent->getNumberOfStates()){
		      auto it = stateMap.find(i_parent->getName());
		      if(it != stateMap.end()) 
			  it->second = i_state;
		}else{
			cerr<<"Entered state out of border"<<endl;
		}
	}
	else if(!contained && addIfNotFound){
		listOfParents.push_back(i_parent);
		stateMap.insert({{i_parent->getName(), i_state}});
	}
	 
	return contained;
}

void BPVector::clear(){
	listOfParents.clear();
	stateMap.clear();
}

//Get methods listed below


int BPVector::getStateOf(BNode* i_node){
	int returnValue = -1;
  
	auto it = stateMap.find(i_node->getName());
	if(it != stateMap.end()) 
	    returnValue= it->second;
	
	return returnValue;
}

int BPVector::getSizeOfParentList(){
	return listOfParents.size();
}

BPVector::~BPVector(){
  
}