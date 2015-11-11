/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _BPVECTOR_H_
#define _BPVECTOR_H_

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#include <unordered_map> //Only with C++ 11 or above

#include "BNode.h"

using namespace std;
using namespace Eigen;


class BPVector{

public:
    /** 
     * document your methods too.
     */
    BPVector();
    
    void addParent(BNode* i_parent, int i_state);
    
    bool set(BNode* i_parent, int i_state, bool i_addIfNotFound = true);//return false if i_parent not found and addIfNotFound == false
    
    void clear();
    
    //Get methods
    int getStateOf(BNode* i_node);
    int getSizeOfParentList();
    
    ~BPVector();


private:
    std::vector<BNode*> listOfParents;
    
    std::unordered_map<std::string, int> stateMap;
 
};


#endif // _BPVECTOR_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

