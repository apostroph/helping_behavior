/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _BEDGE_H_
#define _BEDGE_H_

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#include <unordered_map> //Only with C++ 11 or above

#include "BNode.h"

using namespace std;
using namespace Eigen;


class BEdge{

public:
    /** 
     * document your methods too.
     */
    BEdge(BNode *i_from, BNode *i_to, string i_color = "black");
    
    //Get methods
    BNode* getParent();
    BNode* getChild();
    
    ~BEdge();


private:
    BNode *parent;
    BNode *child;
  
    
    char RGB[3]; //Used to display the graph
    
    //Private methods
    void setColorFromString(string i_color);
 
};


#endif // _BNODE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

