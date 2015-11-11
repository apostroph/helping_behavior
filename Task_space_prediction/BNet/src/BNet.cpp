/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "BNet.h"

BNet::BNet(int argc, char * argv[]){
	listOfNodes = std::vector<BNode*>();
	listOfEdges = std::vector<BEdge*>();
	
	nodesProbability = NULL;
}

void BNet::initProbaMatrix(){
	
	if(nodesProbability != NULL){
		delete nodesProbability;
	}
	nodesProbability = new MatrixXf(listOfNodes[0]->getNumberOfStates(), listOfNodes.size());
	
	for(int i = 0; i < listOfNodes[0]->getNumberOfStates(); i++){
		for(int j = 0; j < listOfNodes.size(); j++){
			nodesProbability[0](i,j) = -1;
		}  
	}
}

bool BNet::run(){
	initProbaMatrix();
	
	for(int i = 0; i < listOfNodes.size(); i++){
		setProbaNode(listOfNodes[i]);
	}
	return true;
}

void BNet::setProbaNode(BNode* i_node){
	//If the state is already known (as an evidence) the proability is set
	//If not, the probability of the parent is looked at
	
// 	cout<<i_node->getName()<<" has "<<listOfNodeInputEdges[getIdOfNode(i_node)]->rows() <<" parents"<<endl;
		
	if(i_node->getState() != -1){
		for(int row = 0; row < nodesProbability->rows(); row++){
			if(row == i_node->getState()){
				nodesProbability[0](row, getIdOfNode(i_node)) = 1; 
			}else{
				nodesProbability[0](row, getIdOfNode(i_node)) = 0; 
			}
		}
	}else{
		for(int i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i ++){
			//If any of the value in the parents' probability matrice is -1, it means that the probability has't yet been calculated
			if((listOfNodeInputEdges[getIdOfNode(i_node)][0](i) != getIdOfNode(i_node)	) && nodesProbability[0](0,listOfNodeInputEdges[getIdOfNode(i_node)][0](i)) == -1){
				setProbaNode(listOfNodes[listOfNodeInputEdges[getIdOfNode(i_node)][0](i)]);
			}
		}
		//Calculate own proba based on parents' proba
		for(int j = 0; j < i_node->getNumberOfStates(); j ++){
			double proba = 0;
			
			for(int k = 0; k < listOfNodeConditionalProbabilities[getIdOfNode(i_node)]->cols(); k++){
				double proba2 = 1;
				double proba1 = (double)listOfNodeConditionalProbabilities[getIdOfNode(i_node)][0](j, k);
				
// 				cout<<i_node->getName()<<" state: "<<j<<"; conditional prob at "<<k<<": "<<proba1<<endl;;
				
				//measure the parent probability
				int parentStates = 1;
				for(int i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i ++){
					if(listOfNodeInputEdges[getIdOfNode(i_node)][0](i) != getIdOfNode(i_node)){
						int index = (int)(k/parentStates);
// 						cout<<k<<"  ::  "<<parentStates<<endl;
						index = index % getNodeId(listOfNodeInputEdges[getIdOfNode(i_node)][0](i))->getNumberOfStates();
						parentStates *= getNodeId(listOfNodeInputEdges[getIdOfNode(i_node)][0](i))->getNumberOfStates();
						
						proba2 *= nodesProbability[0](index, listOfNodeInputEdges[getIdOfNode(i_node)][0](i));
						
// 						cout<<"\tParent "<<getNodeId(listOfNodeInputEdges[getIdOfNode(i_node)][0](i))->getName()<<" index "<<index<<"; prob: "<<proba2<<endl;
					}
				}
// 				proba += exp(log10(proba1)+log10(proba2));
				proba += proba1*proba2;
// 				proba += sqrt(proba1*proba2); //Eventual good solution
				
			}
			nodesProbability[0](j,getIdOfNode(i_node)) = proba;
		}
	}
}

/*addNode(BNode *node)
  * Add a new node to the bayesian network.
  * returns the id of the node if already included or adding successful
  * retunrs -1 if unknown error occured
  */
int BNet::addNode(BNode* i_node, int i_setNbAct){
	int returnValue = -1;
	
	//If the node is already included in listOfNodes, we return 0
	for(auto i: listOfNodes){
		if( i->getName() == i_node->getName() ){
			i->setState(i_node->getState());
			returnValue = getIdOfNode(i);
			
			break;
		}
	}
	
	//If node not included, we add it
	if(returnValue == -1){
		listOfNodes.push_back(i_node);
		listOfNodesActivation.push_back(i_setNbAct);
		
		MatrixXf *newProbabilityMatrix = new MatrixXf(i_node->getNumberOfStates(),1);
		listOfNodeConditionalProbabilities.push_back(newProbabilityMatrix);
		
		VectorXd *newInputEdgeVector = new VectorXd(1);
		newInputEdgeVector[0](0) = getIdOfNode(i_node);
		listOfNodeInputEdges.push_back(newInputEdgeVector);
		returnValue = getIdOfNode(i_node);
	}else{
		listOfNodesActivation[returnValue] ++;
	}

	initProbaMatrix();
	
	return returnValue;
}

/*addEdge(BEdge *edge)
  * Add a new edge to the bayesian network.
  * returns 1 is successful
  * returns 0 is a edge was already added
  * retunrs -1 if unknown error occured
  */
int BNet::addEdge(BEdge* i_edge){
	int returnValue = -1;
	
	//If the edge is already included in listOfEdges, we return 0
	for(int i = 0; i < listOfEdges.size(); i++){
		if( (listOfEdges[i]->getParent()->getName().compare(i_edge->getParent()->getName()) == 0) &&  
		    (listOfEdges[i]->getChild()->getName().compare(i_edge->getChild()->getName()) == 0)){
			returnValue = i;
			break;
		}
	}
	
	//If nde not included, we add it
	if(returnValue == -1){
		listOfEdges.push_back(i_edge);
		listOfEdgesActivation.push_back(1);
		
 		updateNodeProbabilityMatrix(getIdOfNode(i_edge->getChild()), getIdOfNode(i_edge->getParent()));
		returnValue = 1;
	}else{
		listOfEdgesActivation[returnValue]++;
	}
	return returnValue;
}

/*updateNodeProbabilityMatrixs
  * Update the size of the matrices when a node or and edge is added/removed
  */
void BNet::updateNodeProbabilityMatrix(int i_nodeNumber, int i_newEdgeFromNodeID){
  
	MatrixXf oldMatrix(listOfNodeConditionalProbabilities[i_nodeNumber]->cols(), listOfNodeConditionalProbabilities[i_nodeNumber]->rows()); 
 	oldMatrix = listOfNodeConditionalProbabilities[i_nodeNumber][0];
	
	VectorXd oldVector(listOfNodeInputEdges[i_nodeNumber]->rows()); 
 	oldVector = listOfNodeInputEdges[i_nodeNumber][0];
	
	
	int nbParent = getNumberOfParentsOf(getNodeId(i_nodeNumber));
	int nbConditionalProb = 1;
	for(int c = 0; c < nbParent; c++){
		nbConditionalProb *= getParentNb(getNodeId(i_nodeNumber), c)->getNumberOfStates();
	}
	
	//Create a matrix with cols being the number of states of the node and rows all the possibilities for its parent value
	listOfNodeConditionalProbabilities[i_nodeNumber] = new MatrixXf(listOfNodes[i_nodeNumber]->getNumberOfStates(), nbConditionalProb);
	
// 	cout<<getNodeId(i_nodeNumber)->getName()<<" is size "<<listOfNodes[i_nodeNumber]->getNumberOfStates()<<" x "<<nbConditionalProb<<endl;
	
	for(int cRows = 0; cRows < listOfNodeConditionalProbabilities[i_nodeNumber]->rows(); cRows++){  
		for(int cCols = 0; cCols < listOfNodeConditionalProbabilities[i_nodeNumber]->cols(); cCols++){
			listOfNodeConditionalProbabilities[i_nodeNumber][0](cRows, cCols) = 1/(listOfNodes[i_nodeNumber]->getNumberOfStates());
		}
	}
	
	//Create a matrix with rows being the number of states of the node and rows all the possibilities for its parent value
	listOfNodeInputEdges[i_nodeNumber] = new VectorXd(getNumberOfParentsOf(listOfNodes[i_nodeNumber]));
	
	for(int cRows = 0; cRows < oldVector.rows(); cRows++){
		listOfNodeInputEdges[i_nodeNumber][0](cRows) = oldVector(cRows);
	}
	listOfNodeInputEdges[i_nodeNumber][0]((listOfNodeInputEdges[i_nodeNumber]->rows())-1) = i_newEdgeFromNodeID;
  
}
    
    
/*setNodeProbability(BNode* i_node, BPVector i_parentsStates, int i_inputNodeState, int i_probability)
  * set the probability for a node to be a given state knowing the state of his parents
  * returns 1 is successful
  * retunrs -1 if unknown error occured
  * !! set nodes' probabilities everytime you add or remove an edge or node
  */
int BNet::setNodeProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState, float i_probability){
	int idCol, idRow;
	int idNode;
	
	//The row number is equivalent to the state number
	idRow = i_inputNodeState;
	
	//Get the id of the node to find the correct probability matrix
	idNode = getIdOfNode(i_node);
	
	//If the number of parents in the BPVector is different from the real number of parents
	if(i_parentsStates->getSizeOfParentList() != getNumberOfParentsOf(i_node)){
		cerr<<"Wrong number of parent in BPVector in classs setNodeProbability"<<endl;
		return -1;
	}
	
	//If the size of the probability matrix is not what it should be
	if(listOfNodeConditionalProbabilities[idNode]->cols() != (pow(2, getNumberOfParentsOf(i_node)))){
		cerr<<"Error: probability matrix size mismatch in class setNodeProbability"<<endl;
		return -1;
	}
	
	
	idCol = getCol(i_node, i_parentsStates);	
	
	//For now the proba are saved as percentages
	listOfNodeConditionalProbabilities[getIdOfNode(i_node)][0](idRow, idCol) = (float)i_probability;
	
}
    
    
/*setNodeProbability(BNode* i_node, int row, int cols, int i_probability)
  * set the probability for a node to be a given state knowing the state of his parents
  * returns 1 is successful
  * retunrs -1 if unknown error occured
  * !! set nodes' probabilities everytime you add or remove an edge or node
  */
int BNet::setNodeProbability(BNode* i_node, int row, int cols, float i_probability){
	int idNode;
	//Get the id of the node to find the correct probability matrix
	idNode = getIdOfNode(i_node);

	try
	{
		//For now the proba are saved as percentages
		listOfNodeConditionalProbabilities[idNode][0](row, cols) = (float)i_probability;
	}
	catch(const char* Message)
	{
		cerr<<"out of bound\n";
	}
	
}

/*newEvidence(BNode* i_node, int i_state)
  * Add a state to a node
  * returns 1 is successful
  * retunrs -1 if unknown error occured
  */
int BNet::newEvidence(BNode* i_node, int i_state){
	int returnValue = 1;
	
	int idOfNode = getIdOfNode(i_node);
	
	if(idOfNode != -1){
		listOfNodes[idOfNode]->setState(i_state);
		initProbaMatrix();
	}else{
		returnValue = -1;
	}
	
	return returnValue;
}

/* posteriorProbabilityOf(BNode* i_node)
  * return the posterior probability of a node if possible
  * return -1 if the posterior probability was not calculated
  */
float BNet::getPosteriorProbabilityOf(BNode* i_node){
	float returnValue = -1;
	
	
	return returnValue;
}

//Get methods listed below

MatrixXf* BNet::getConditionalProbOfNode(int i_index)
{
	return listOfNodeConditionalProbabilities[i_index];
}

void BNet::setConditionalProbOfNode(int i_index, MatrixXf* i_inputMatrix)
{
	for(int cRows = 0; cRows < i_inputMatrix->rows(); cRows++){  
		for(int cCols = 0; cCols < i_inputMatrix->cols(); cCols++){
			listOfNodeConditionalProbabilities[i_index][0](cRows, cCols) = i_inputMatrix[0](cRows, cCols);
		}
	}
}



int BNet::getNumberOfNodes(){
	return this->listOfNodes.size();
}

int BNet::getNumberOfEdges(){
	return this->listOfEdges.size();
}

int BNet::getIdOfNode(BNode* i_node){
	int id = -1;
		
	for(int count = 0; count < listOfNodes.size(); count++){
		if(listOfNodes[count]->getName().compare(i_node->getName()) == 0){
			id = count;
			break;
		}
	}
	
	return id;
}

BNode* BNet::getNodeId(int i_index){
	return listOfNodes[i_index];
}

BNode* BNet::getNodeNamed(string i_name){
	for(auto i: listOfNodes){
		if(i->getName() == i_name){
			return i;
		}
	}
	return NULL;
}
    
int BNet::getNumberOfParentsOf(BNode* i_node){
	int numberOfParents = 0;
		
	for(auto i: listOfEdges){
		if(i->getChild()->getName() == i_node->getName()){
			numberOfParents++;
		}
	}
	
	return numberOfParents;
}

BNode* BNet::getParentNb(BNode* i_node, int parentIndex){
	int count = 0;
		
	for(auto i: listOfNodes){
		for(auto j: listOfEdges){
			if((j->getParent()->getName().compare(i->getName()) ==0 ) && (j->getChild()->getName().compare(i_node->getName()) == 0)){
				if(count == parentIndex){
					return i;
				}
				count++;
			}
		}
	}
	
	return NULL;
}
    
int BNet::getNumberOfChildsOf(BNode* i_node){
	int numberOfChild = 0;
		
	for(auto i: listOfEdges){
		if(i->getParent()->getName() == i_node->getName()){
			numberOfChild++;
		}
	}
	
	return numberOfChild;
}

BNode* BNet::getChildID(BNode* i_node, int childIndex){
	int count = 0;
		
	for(auto i: listOfNodes){
		for(auto j: listOfEdges){
			if((j->getChild()->getName() == i->getName()) && (j->getParent()->getName() == i_node->getName())){
				if(count == childIndex){
					return i;
				}
				count++;
			}
		}
	}
	
	return NULL;
}

int BNet::getNumberOfActivationOfNodeId(int i_index){
  
	return listOfNodesActivation[i_index];
}

    
float BNet::getNodeConditionalProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState){
	int idCol, idRow;
	int idNode;
	
	//The row number is equivalent to the state number
	idRow = i_inputNodeState;
	
	//Get the id of the node to find the correct probability matrix
	idNode = getIdOfNode(i_node);
	
	//If the number of parents in the BPVector is different from the real number of parents
	if(i_parentsStates->getSizeOfParentList() != getNumberOfParentsOf(i_node)){
		cerr<<"Wrong number of parent in BPVector in classs getNodeProbability"<<endl;
		return -1;
	}
	
	//If the size of the probability matrix is not what it should be
	if(listOfNodeConditionalProbabilities[idNode]->cols() != (pow(2, getNumberOfParentsOf(i_node)))){
		cerr<<"Error: probability matrix size mismatch in class getNodeProbability"<<endl;
		return -1;
	}
	
	idCol = getCol(i_node, i_parentsStates);	
	
	return listOfNodeConditionalProbabilities[getIdOfNode(i_node)][0](idRow, idCol);
}

float BNet::getNodeProbability(BNode* i_node, int state){
	return nodesProbability[0](state, getIdOfNode(i_node));
}

inline int BNet::getCol(BNode* i_node, BPVector* i_parentsStates){
	int idCol = 0;
	
	int mult = ((pow(2, getNumberOfParentsOf(i_node))))/2;
	for(auto i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i++){
		idCol += mult*i_parentsStates->getStateOf(listOfNodes[listOfNodeInputEdges[getIdOfNode(i_node)][0](i)]);
		mult /= 2;
	}
	
	return idCol;
}

string BNet::toString(){
	string returnString = "";
	
	if(listOfNodes.size() > 0){
	
		returnString+= "----------------------------------------------------------------\n";
		returnString+= "The network is composed of " + to_string(listOfNodes.size()) + " node(s) and " + to_string(listOfEdges.size()) + " edge(s)\n\n";
		
		for(auto i:listOfNodes){
			returnString+= i->getName() +" has " + to_string(i->getNumberOfStates()) + " states and was observed [" + to_string(listOfNodesActivation[getIdOfNode(i)]) + " times] and has " + to_string(getNumberOfChildsOf(i)) + " child(s)\n";
			for(int c = 0; c < getNumberOfChildsOf(i); c++){
				returnString+= "\t--> " + getChildID(i, c)->getName()+ "\n";
			}
			returnString+= "\n";
		}
	  
		if(listOfEdges.size() > 0){
			if(nodesProbability!= NULL && nodesProbability[0](0,0) != -1){
				for(auto cCols = 0; cCols < nodesProbability->cols(); cCols++){
					for(auto cRows = 0; cRows < nodesProbability->rows(); cRows++){  
						if(listOfNodes[cCols]->getNumberOfStates() > cRows)
							returnString += "P(" + listOfNodes[cCols]->getName() + "=" + to_string(cRows) + ") = " + to_string((float)nodesProbability[0](cRows, cCols)) + "; ";
					}
					returnString += "\n";
				}
			}else{
				returnString += "The probability matrix is no available \n";
			}
		}
	}else{
		returnString = "Empty network\n";
	}
	
	return returnString;
}


BNet::~BNet(){
  
}


