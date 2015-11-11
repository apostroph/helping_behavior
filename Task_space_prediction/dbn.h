/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _DBN_H_
#define _DBN_H_

#include <string>

#include <iostream>
#include <fstream> 
#include <iomanip>
#include <eigen3/Eigen/Dense>

#include "ros/ros.h"
#include <ros/package.h>
#include "std_msgs/String.h"
#include <signal.h>

#include "BNet/src/BNet.h"
#include "BNet/src/displayNodes.h"

#include "RGB_pcl/States.h"

using namespace std;

class dbn{

public:
    /** 
     * document your methods too.
     */
    dbn(int argc, char** argv);
    ~dbn();
    
    bool loop();

private:  
    ros::NodeHandle n;
    ros::Subscriber sub;
    ros::Publisher pub;
    ros::Publisher pubActSel;
    
    BNet* dynamicBNet;
    BNet* staticBNet;  
    
    BNode* state_node;
    BNode* state_node_t_1;
    
    int maxTimeStep;
    
    int stateSpaceSize;
    unordered_map<int, string> observedStates;
    unordered_map<string, int> valueToState;
    unordered_map<int, string> stateToValue;
    unordered_map<int, int> statesObservationCount;
    
    BNode* action_node;
    BNode* action_node_t_1;
    int actionSpaceSize;
    unordered_map<int, string> observedActions;
    unordered_map<int, string> observedActionsPrimitive;
    unordered_map<string, int> affectToAction;
    unordered_map<int, string> actiontoAffect;
    unordered_map<int, int> actionsObservationCount;
      
    vector<BEdge*> observedEdgeList;
    vector< vector<string>> listOfEdges;
      
    vector<BNode*> listNodes;
    
    vector<BNode*> activeNodes;
    vector<BNode*> newStateVector;
    BNode *currentAction;
    
    bool VERBOSE;
    
    //ROS quit handler
    static void sigintHandler(int sig);
    
    void generateDynamicBNet(int stepNumber = 1);
    void sendToAction(const RGB_pcl::States& input, string s_value, string state, string action, int count);
    double predictFutureStates(const RGB_pcl::States& input, string currentState, int timeStep, string &action, string &state, int count);//vector<string> &states, vector< vector<string>> &actions);
    double backTrace_1(const int nodeID, const int stateID, const int timStep, string currentState, vector<string> &states, vector<string> &actions);
    
    //load data methods
    void loadData(string src);
    void stateSpace(ifstream &input);
    void actionSpace(ifstream &input);
    void edgeList(ifstream &input);
    void estimateConditionalProbabilities();
    
    void callback_newEvent(string msg);//!! const std_msgs::String& msg);
    void newActionNode(BNode* newActionNode);
    void newStateNodeList(vector<BNode*> i_newStateVector);
    
    string getStaticName(const string input);
  
    vector<string> getParamList(string input);
    int getID(string input);
    
    void vision_in(const RGB_pcl::States& input);
  
};


#endif // __DBN_H__

//----- end-of-file --- ( next line intentionally left blank ) ------------------

