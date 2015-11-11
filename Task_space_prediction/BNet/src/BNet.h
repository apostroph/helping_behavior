/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _BNET_H_
#define _BNET_H_

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#include <unordered_map> //Only with C++ 11 or above


#include "BEdge.h"
#include "BNode.h"
#include "BPVector.h"

using namespace std;
using namespace Eigen;


class BNet{

public:
    /** 
     * document your methods too.
     */
    BNet(int argc = 0, char * argv[] = NULL);
    
     /*
      * Look at the current state of each node and calculate the praboality of each node to be active
      */
     bool run();
    


    /*addNode(BNode *node)
      * Add a new node to the bayesian network.
      * returns the id of the node if already included or adding successful
      * retunrs -1 if unknown error occured
      */
    int addNode(BNode* i_node, int i_setNbAct = 1);
    
    /*newEvidence(BNode* i_node, int i_state)
     * Add a state to a node
     * returns 1 is successful
     * retunrs -1 if unknown error occured
     */
    int newEvidence(BNode* i_node, int i_state);
    
    /*addEdge(BEdge *edge)
     * Add a new edge to the bayesian network.
     * returns 1 is successful
     * returns 0 is a edge was already added
     * retunrs -1 if unknown error occured
     * !! Changing the number of edges will results in changing the node's probability matrix size !!
     */
    int addEdge(BEdge* i_edge);
    
    
    /*setNodeProbability(BNode* i_node, BPVector i_parentsStates, int i_inputNodeState, int i_probability)
     * set the probability for a node to be a given state knowing the state of his parents
     * returns 1 is successful
     * retunrs -1 if unknown error occured
     * !! sets nodes' probabilities everytime you add or remove an edge or node
     */
    int setNodeProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState, float i_probability);
    
    
    /*setNodeProbability(BNode* i_node, int row, int cols, int i_probability)
     * set the probability for a node to be a given state knowing the state of his parents
     * returns 1 is successful
     * retunrs -1 if unknown error occured
     * !! sets nodes' probabilities everytime you add or remove an edge or node
     */
    int setNodeProbability(BNode* i_node, int row, int cols, float i_probability);
    
    /* posteriorProbabilityOf(BNode* i_node)
     * return the posterior probability of a node if possible
     * return -1 if the posterior probability was not calculated
     */
    float getPosteriorProbabilityOf(BNode* i_node);
    
    ~BNet();

    //Get methods
    int getNumberOfNodes();
    int getNumberOfEdges();
    
    int getIdOfNode(BNode* i_node);
    
    int getNumberOfParentsOf(BNode* i_node);
    BNode* getParentNb(BNode* i_node, int parentIndex);
    
    int getNumberOfChildsOf(BNode* i_node);
    BNode* getChildID(BNode* i_node, int childIndex);
    
    int getNumberOfActivationOfNodeId(int i_index);
    
    BNode* getNodeId(int i_index);
    BNode* getNodeNamed(string i_name);
    
    MatrixXf* getConditionalProbOfNode(int i_index);
    void setConditionalProbOfNode(int i_index, MatrixXf* i_inputMatrix);
    
    float getNodeConditionalProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState);
    float getNodeProbability(BNode* i_node, int state);
    
    string toString();

private:
    std::vector<BNode*> listOfNodes;
    std::vector<int> listOfNodesActivation;
    std::vector<BEdge*> listOfEdges;
    std::vector<int> listOfEdgesActivation;
    
    MatrixXf* nodesProbability;
    
    unordered_map<string, string> shortenName;
    
    std::vector<MatrixXf*> listOfNodeConditionalProbabilities;
    std::vector<VectorXd*> listOfNodeInputEdges;
        
    void setProbaNode(BNode* i_node);
    
    /*updateNodeProbabilityMatrixs
     * Update the size of the matrice coreresponding to i_nodeNumber when a node or and edge is added/removed
     */
    void updateNodeProbabilityMatrix(int i_nodeNumber, int i_newEdgeFromNodeID);
    void initProbaMatrix();
    inline int getCol(BNode* i_node, BPVector* i_parentsStates);
};


#endif // _BNET_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

