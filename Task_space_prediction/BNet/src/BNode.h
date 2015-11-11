/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _BNODE_H_
#define _BNODE_H_

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#include <unordered_map> //Only with C++ 11 or above

using namespace std;
using namespace Eigen;


class BNode{

public:
    BNode(string i_name, int i_numberOfStates, string i_color = "black");
    
    /*setState
     * input: new state
     * return: 1 if change ok, 0 if no change
     */
    bool setState(int i_newState); //-1 if we don't know
    
    //Get methods
    int getState();
    
    int getNumberOfStates();
    int getActivationNumber(int stateID);
    string getName();
    char* getColor();
    
    ~BNode();


private:
    string name;//Name of the Node
    
    int numberOfStates;//Number of state for the node, 2 for binary nodes
    
    int currentState; //-1 for unknown
    
    vector<int> stateActivations; //counts the number of time a state has been active
    
    char RGB[3]; //Used to display the graph
    
    //Private methods
    void setColorFromString(string i_color);
 
};


#endif // _BNODE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

