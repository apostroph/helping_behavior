/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/
#include "dbn.h"

#define MAX_PREDICTION_STEPS 10


dbn::dbn(int argc, char** argv):
VERBOSE(0)
{    
	n = ros::NodeHandle("~");
	
	int input = 0;
	n.getParam("verbose", input);
	VERBOSE = (bool)input;
	
	int task_id = 0;
	n.getParam("task", task_id);
	
	string task_in = "trial_task";
	switch(task_id){
	  case 1: task_in = "task_A_1"; break;
	  case 2: task_in = "task_B_1"; break;
	  case 3: task_in = "task_A_2"; break;
	  case 4: task_in = "task_B_2"; break;
	  case 5: task_in = "task_A_3"; break;
	  case 6: task_in = "task_B_3"; break;
	}
	

//! 	sub = n.subscribe("recognition/event/newState", 25, &dbn::callback_newEvent, this);
	
	signal(SIGINT, &dbn::sigintHandler);
	
	staticBNet = new BNet();
	dynamicBNet = new BNet();
	
	currentAction = NULL;
	
	// Create a ROS subscriber for the current evidences
	sub = n.subscribe ("/Recognition/actions", 1, &dbn::vision_in, this);
	
	pubActSel = n.advertise<RGB_pcl::States> ("states_list", 1);
	
	std::string path = ros::package::getPath("dbn");
	loadData(path + "/data/"+task_in+".xml");
	
	estimateConditionalProbabilities();
	
	maxTimeStep = 2;
	
	ROS_INFO("Ready start the dbn");
}

void dbn::sigintHandler(int sig)
{
	cout<<endl;
	ROS_INFO("dbn module shutdown...");
	ros::shutdown();
	
	exit(0);
}

void dbn::vision_in(const RGB_pcl::States& input){
	generateDynamicBNet(maxTimeStep);
	
	for(int count = 0; count < input.states.size(); count++){
		string s_value = (string)input.states[count];
		
		dynamicBNet->newEvidence(dynamicBNet->getNodeId(0),  valueToState[s_value]);
		
		dynamicBNet->run();
		
		string action, state;
		double proba = predictFutureStates(input, s_value, maxTimeStep, action, state, count);
		
	}	
}

void dbn::sendToAction(const RGB_pcl::States& input, string s_value, string state, string action, int count){
	bool possible = false;
	ROS_INFO("DBN module");
	for(auto s_state: input.states){
		if(action.find("a") != std::string::npos && actiontoAffect[stoi(action.substr(1, -1))].compare(s_value) == 0){
			string act_tgt = observedActionsPrimitive[stoi(action.substr(1, -1))];
			act_tgt = act_tgt.substr(act_tgt.find("[")+1, act_tgt.find("]")-act_tgt.find("[")-1);
// 			cout<<act_tgt<<" :: "<<s_state<<endl;
			if(state.find("s") != std::string::npos && act_tgt.compare(s_state)  == 0){
				possible = true;
				break;
			}
		}
	}
	
	if(possible){
		cv::Point3d position;
		int index = count;
		for(auto s_state: input.states){
			if(actiontoAffect[stoi(action.substr(1, -1))].compare(s_value) == 0){
				position.x = input.x[index]; position.y = input.y[index]; position.z = input.z[index]; 
				break;
			}
			index++;
		}
		RGB_pcl::States message;
		
		message.action = observedActionsPrimitive[stoi(action.substr(1, -1))];
		
		message.states.push_back(observedActionsPrimitive[stoi(action.substr(1, -1))]);
		message.x.push_back(position.x); message.y.push_back(position.y); message.z.push_back(position.z);
		message.clusters.push_back(input.clusters[index]);
		
		message.states.push_back(stateToValue[stoi(state.substr(1, -1))]);
		message.x.push_back(input.x[count]); message.y.push_back(input.y[count]); message.z.push_back(input.z[count]);
		message.clusters.push_back(input.clusters[count]);
		
		cout<<"DBN ==> "<<observedActionsPrimitive[stoi(action.substr(1, -1))]<< "  ::  "<<stateToValue[stoi(state.substr(1, -1))]<<endl;
		
		pubActSel.publish (message);
	}
}

double dbn::predictFutureStates(const RGB_pcl::States& input, string currentState, int timeStep, string &action, string &state, int count){//vector<string> &states, vector< vector<string>> &actions){
	
	int N = timeStep;
	float maxProba = 0;
	int winnerState = -1;
	int winnerNode = -1;
	double totalProba = 0;
  
	for(int c = 0; c < dynamicBNet->getNumberOfNodes(); c++){
		int sub = stoi(dynamicBNet->getNodeId(c)->getName().substr(4, -1));
		string name = dynamicBNet->getNodeId(c)->getName().substr(0, 1);
		
		if(sub == N){
			for(int s = 0; s < dynamicBNet->getNodeId(c)->getNumberOfStates(); s++){
				if(name.compare("s") == 0){
					float proba = dynamicBNet->getNodeProbability(dynamicBNet->getNodeId(c), s);
					if(proba > maxProba){
						maxProba = proba;
						winnerState = s;
						winnerNode = c;
					}
					if(proba > 0){
						vector<string> states, actions;
						backTrace_1(winnerNode, s, timeStep, currentState, states, actions);
						if(states.size() > 0 && actions.size() > 0){
							sendToAction(input, currentState, states[0], actions[0], count);
						}
					}
				}
			}
		}
		
	}
	
	if(N != 0 && maxProba > 0.01){
	
		vector<string> states, actions;
		backTrace_1(winnerNode, winnerState, timeStep, currentState, states, actions);
		states.push_back(observedStates[winnerState]);
		
		if(states.size() > 0)
			state = states[0];
		
		if(actions.size() > 0)
			action = actions[0];
		
		
// 		for(int count = 0; count < actions.size(); count++){
		if(states.size() > 1){
			if(VERBOSE){
				cout<<observedActionsPrimitive[stoi(actions[0].substr(1, -1))]<<" on "<<actiontoAffect[stoi(actions[0].substr(1, -1))]<<endl;
				cout<<"\t"<<stateToValue[stoi(states[0].substr(1,-1))]<<" >> "<<stateToValue[stoi(states[1].substr(1,-1))]<<endl;
			}
			totalProba = maxProba;
			
		}
// 		}
	}else if(N != 0){
		totalProba = predictFutureStates(input, currentState, timeStep-1, action, state, count);
	}else{
	}
	return totalProba;
}

double dbn::backTrace_1(const int nodeID, const int stateID, const int timeStep, string currentState, vector<string> &states, vector<string> &actions){

	double proba = 1;
	double maxProba = 0;
	double maxPAction = 0;
	string s_state, s_action;
	
	BNode currentNode = *dynamicBNet->getNodeId(nodeID);
	//for each element of the probability distribution of node nodeID
	for(int condP_c = 0; condP_c < dynamicBNet->getConditionalProbOfNode(nodeID)->cols(); condP_c++){
		double probaOfLine = 1;
		
		double posterior = 1;
		//p1 ==> conditional probability  P(nodeID=stateID|parent combination condP_c)
		double p1 = dynamicBNet->getConditionalProbOfNode(nodeID)[0](stateID, condP_c);
		//p2 ==> dynamicBNet->getNodeId(nodeID) to be state stateID
		double p2 = dynamicBNet->getNodeProbability(dynamicBNet->getNodeId(nodeID), stateID);
		double p3 = 1;
		 
		//For each parent of node nodeID
		for(int parent_c = 0; parent_c < dynamicBNet->getNumberOfParentsOf(dynamicBNet->getNodeId(nodeID)); parent_c++){
			//is the node s_t+n
			string nameParent = dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c)->getName().substr(0, 1);
			if(nameParent.compare("s") == 0){	
				//if we are looking at a state node
			  
				//index_state ==> value of S for node nodeID at the condP position in the conditional prob
				int index_state = condP_c % dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c)->getNumberOfStates();
				
				int index_action = -1;
				//p3 ==> probability of node dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c) to be state index_state
				p3 = dynamicBNet->getNodeProbability(dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c), index_state);
				
				//Calculate the posterior probability
				posterior = (p1*p2)/p3;
				
				if(p3 != 0 && posterior != 0 && !(std::isnan(posterior))){
					
					//For each parent of node nodeID
					double p4 = 0;
					for(int parent_c2 = 0; parent_c2 < dynamicBNet->getNumberOfParentsOf(dynamicBNet->getNodeId(nodeID)); parent_c2++){
					
						//is the node a_t+n
						string nameParent2 = dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c2)->getName().substr(0, 1);
						if(nameParent2.compare("a") == 0){	
						
							//index_state ==> value of A at the condP position in the conditional prob
							index_action = (condP_c/dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c2-1)->getNumberOfStates()) % dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c2)->getNumberOfStates();
						
							//if we are looking at a action node
							p4 = dynamicBNet->getNodeProbability(dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c2), index_action);
							if(p4 > maxPAction){
								maxPAction = p4;
								s_action = "a" + to_string(index_action);
							}
						}
					}
					
					//state name
					s_state = "s" + to_string(index_state);
					
					if(timeStep > 1 && currentState.compare(s_state) != 0){
						//get the proba for the slice before
						probaOfLine = backTrace_1(dynamicBNet->getIdOfNode(dynamicBNet->getParentNb(dynamicBNet->getNodeId(nodeID), parent_c)), index_state, timeStep-1, currentState, states, actions);
					}
						
					if(posterior*probaOfLine > maxProba){// || (posterior*probaOfLine == maxProba && pAction > maxPAction)){
						maxProba = posterior*probaOfLine;
// 						s_action = "a" + to_string(index_action);
						s_state = "s" + to_string(index_state);
					}
				}
			}
		}
	}
	states.push_back(s_state);
	actions.push_back(s_action);
	
	return maxProba;
}



void dbn::loadData(string src){
	ifstream input(src);
	string line;
	
	if (input.is_open())
	{
		cout<<"Loading "<<src<<endl;
		while ( getline (input, line) )
		{
			if(line.find("<StateSpace>") != std::string::npos){
				stateSpace(input);
			}else if(line.find("<ActionList>") != std::string::npos){
				actionSpace(input);
			}else if(line.find("<EdgeList>") != std::string::npos){
				edgeList(input);
			}
		}
		input.close();
	}else{
		cerr<<"Initialization file not found at "<<src<<endl;
	}
	
	if(actionSpaceSize != 0 && stateSpaceSize != 0){
		staticBNet = new BNet();
		staticBNet->addNode(state_node);
		staticBNet->addNode(action_node);
		
		staticBNet->addNode(state_node_t_1);
		staticBNet->addNode(action_node_t_1);
		
		for(auto edge:observedEdgeList){
			staticBNet->addEdge(edge);
		}
	}
	
}

void dbn::stateSpace(ifstream &input){
	string line;
	if(VERBOSE)
		cout<<endl<<"Loading state information: "<<endl;
	
	getline (input, line);
	int size = stoi(line.substr(line.find("<size>")+6, line.find("</size>")-line.find("<size>")+6));
	state_node = new BNode("s_t+0", size);
	state_node_t_1 = new BNode("s_t+1", size);
	
	stateSpaceSize = size;
	for (int count = 0; count < size; count++){
		getline (input, line);
		if(line.find("<state>") != std::string::npos){
			if(VERBOSE)
				cout<<"Node: "<<count<<endl;
			while(line.find("</state>") == std::string::npos){
				getline (input, line);
				if(line.find("name") != std::string::npos){
					observedStates[count] = line.substr(line.find("<name>")+6, line.find("</name>")-(line.find("<name>")+6));
				}else if(line.find("<value>") != std::string::npos){
					getline (input, line);
					string arguments;
					while(line.find("</value>") == std::string::npos){
						string element = line.substr(line.find("-")+1, -1);
						if(VERBOSE)
							cout<<"\tFound element in <state> <value>: "<<element<<endl;
						getline (input, line);
						
						arguments = arguments + element + " ";
					}
					valueToState[arguments] = count;
// 					cout<<arguments<<" ==> "<<count<<endl;
					stateToValue[count] = arguments;
					
				}else if(line.find("count") != std::string::npos){
					actionsObservationCount[count] = stoi(line.substr(line.find("<count>")+7, line.find("</count>")-(line.find("<count>")+7)));
				}
			}
		}else{
			count--;
		}
	}
}

void dbn::actionSpace(ifstream &input){
	string line;
	if(VERBOSE)
		cout<<endl<<"Loading action information: "<<endl;
	
	getline (input, line);
	int size = stoi(line.substr(line.find("<size>")+6, line.find("</size>")-line.find("<size>")+6));
	action_node = new BNode("a_t+0", size);
	action_node_t_1 = new BNode("a_t+1", size);
	
	actionSpaceSize = size;
	for (int count = 0; count < size; count++){
		getline (input, line);
		if(line.find("<action>") != std::string::npos){
			if(VERBOSE)
				cout<<"Action: "<<count<<endl;
			while(line.find("</action>") == std::string::npos){
				getline (input, line);
				if(line.find("name") != std::string::npos){
					//Add a value to the action space
					observedActions[count] = line.substr(line.find("<name>")+6, line.find("</name>")-(line.find("<name>")+6));
				}else if(line.find("<affect>") != std::string::npos){
					getline (input, line);
					string arguments;
					while(line.find("</affect>") == std::string::npos){
						string element = line.substr(line.find("-")+1, -1);
						if(VERBOSE)
							cout<<"\tFound element in <action> <affect>: "<<element<<endl;
						getline (input, line);
						
						arguments = arguments + element + " ";
					}
					affectToAction[arguments] = count;
					actiontoAffect[count] = arguments;
				}else if(line.find("primitive") != std::string::npos){
					//Name of the action value (action primitive)
					observedActionsPrimitive[count] = line.substr(line.find("<primitive>")+11, line.find("</primitive>")-(line.find("<primitive>")+11));
				}else if(line.find("count") != std::string::npos){
					//Add a value to the number of time the action value was observed
					statesObservationCount[count] = stoi(line.substr(line.find("<count>")+7, line.find("</count>")-(line.find("<count>")+7)));
				}
			}
		}else{
			count--;
		}				
	}
}

void dbn::edgeList(ifstream &input){
	string line;
	if(VERBOSE)
		cout<<endl<<"Loading edge information: "<<endl;
	
	getline (input, line);
	int size = stoi(line.substr(line.find("<size>")+6, line.find("</size>")-line.find("<size>")+6));
	for (int count = 0; count < size; count++){
		getline (input, line);
		if(line.find("<edge>") != std::string::npos){
			if(VERBOSE)
				cout<<"Node: "<<count<<endl;
			while(line.find("</edge>") == std::string::npos){
				getline (input, line);
				if(line.find("<connection>") != std::string::npos){
					getline (input, line);
					vector<string> elementList;
					string arguments;
					while(line.find("</connection>") == std::string::npos){
						string element = line.substr(line.find("-")+1, -1);
						if(VERBOSE)
							cout<<"\tFound element in <edge> <connection>: "<<element<<endl;
						elementList.push_back(element);
						
						getline (input, line);
						
						arguments = arguments + element + "-";
					}
					listOfEdges.push_back(elementList);
					
					if(elementList[0].find("s") != std::string::npos){
						if(elementList[1].find("a") != std::string::npos){
							BEdge* newEdge1 = new BEdge(state_node, action_node);
							BEdge* newEdge2 = new BEdge(state_node_t_1, action_node_t_1);
							observedEdgeList.push_back(newEdge1);
							observedEdgeList.push_back(newEdge2);
						}
						else if(elementList[1].find("s") != std::string::npos ){
							BEdge* newEdge = new BEdge(state_node, state_node_t_1);
							observedEdgeList.push_back(newEdge);
						}
						if(elementList[2].find("s") != std::string::npos ){
							BEdge* newEdge = new BEdge(state_node, state_node_t_1);
							observedEdgeList.push_back(newEdge);
						}
					}
					if(elementList[1].find("a") != std::string::npos){
						if(elementList[2].find("s") != std::string::npos ){
							BEdge* newEdge = new BEdge(action_node, state_node_t_1);
							observedEdgeList.push_back(newEdge);
						}
					}
					
				}else if(line.find("count") != std::string::npos){
					actionsObservationCount[count] = stoi(line.substr(line.find("<count>")+7, line.find("</count>")-(line.find("<count>")+7)));
				}
			}
		}else{
			count--;
		}				
	}
}

void dbn::estimateConditionalProbabilities(){
	std::vector<MatrixXf> listOfNodeConditionalProbabilities;
	//For every node
	for(int count = 0; count < staticBNet->getNumberOfNodes(); count++){
		int nbParent = staticBNet->getNumberOfParentsOf(staticBNet->getNodeId(count));
		int nbConditionalProb = 1;
		
		for(int c = 0; c < nbParent; c++){
			nbConditionalProb *= staticBNet->getParentNb(staticBNet->getNodeId(count), c)->getNumberOfStates();
		}
		//we create the probability matrix
		MatrixXf conditionalP(staticBNet->getNodeId(count)->getNumberOfStates(), nbConditionalProb);
		for(auto cCols = 0; cCols < conditionalP.cols(); cCols++){
			for(auto cRows = 0; cRows < conditionalP.rows(); cRows++){  
				conditionalP(cRows, cCols) = 0;
			}
		}
		//and every state of this node
		for(int c_state = 0; c_state < staticBNet->getNodeId(count)->getNumberOfStates(); c_state++){
			int countActivation = 0;
			int edgeIndex = 0;
			string nameState =  staticBNet->getNodeId(count)->getName().substr(0,1) + to_string(c_state);
			for(auto edge: listOfEdges){
				if(edge.size() >= nbParent && edge[nbParent] != "NULL" && edge[nbParent]==nameState){
					int indexChilds = 0;
					int stateNumber = 1;
					countActivation += actionsObservationCount[edgeIndex];
					//for all the parent of the node in the edge
					for(int c = 0; c < nbParent; c++){
						indexChilds += stoi(edge[c].substr(1, -1))*stateNumber;
						stateNumber = staticBNet->getParentNb(staticBNet->getNodeId(count), c)->getNumberOfStates();
					}
					conditionalP(c_state, indexChilds) += actionsObservationCount[edgeIndex];
				}
				edgeIndex++;
			}
		}
		for(auto cCols = 0; cCols < conditionalP.cols(); cCols++){
			double sum = 0;
			for(auto cRows = 0; cRows < conditionalP.rows(); cRows++){ 
				sum += conditionalP(cRows, cCols);
			}
			for(auto cRows = 0; cRows < conditionalP.rows(); cRows++){ 
				if(sum != 0){
					conditionalP(cRows, cCols) = conditionalP(cRows, cCols)/sum;
				}else{
					conditionalP(cRows, cCols) = 0;
				}
				staticBNet->setNodeProbability(staticBNet->getNodeId(count), cRows, cCols, conditionalP(cRows, cCols));
				if(VERBOSE){
					string line = "P(" + staticBNet->getNodeId(count)->getName().substr(0,1) + "=" + staticBNet->getNodeId(count)->getName().substr(0,1)+to_string(cRows) + ") = " + to_string((float)conditionalP(cRows, cCols)) + "; ";
					cout<<line;
				}
			}
			if(VERBOSE)
				cout<<endl;
		}
		if(VERBOSE)
			cout<<endl;
	}
	staticBNet->run();
	

}

void dbn::generateDynamicBNet(int stepNumber){
	*dynamicBNet = *staticBNet;
	
	for(int n = 1; n < stepNumber; n++){
		for(int countNode = 0; countNode < dynamicBNet->getNumberOfNodes(); countNode++){
			if(dynamicBNet->getNodeId(countNode)->getName().find("+"+to_string(n)) != std::string::npos){
				BNode* newNode = new BNode(dynamicBNet->getNodeId(countNode)->getName().substr(0, 1)+"_t+"+to_string(n+1), dynamicBNet->getNodeId(countNode)->getNumberOfStates());
				dynamicBNet->addNode(newNode);
			}
		}
		for(int countNode = 0; countNode < dynamicBNet->getNumberOfNodes(); countNode++){
			if(dynamicBNet->getNodeId(countNode)->getName().find("+"+to_string(n)) != std::string::npos){
				for(int c = 0; c < dynamicBNet->getNumberOfParentsOf(dynamicBNet->getNodeId(countNode)); c++){
					string nameParent = dynamicBNet->getParentNb(dynamicBNet->getNodeId(countNode), c)->getName();
					int newId = stoi(nameParent.substr(nameParent.find("+")+1, -1));
					
					string newParentName = dynamicBNet->getParentNb(dynamicBNet->getNodeId(countNode), c)->getName().substr(0, 1) + "_t+" + to_string(newId+1);
					BEdge* newEdge = new BEdge(dynamicBNet->getNodeNamed(newParentName), dynamicBNet->getNodeNamed(dynamicBNet->getNodeId(countNode)->getName().substr(0, 1)+"_t+"+to_string(n+1)));
					dynamicBNet->addEdge(newEdge);
				}
			}
		}
		for(int countNode = 0; countNode < dynamicBNet->getNumberOfNodes(); countNode++){
			if(dynamicBNet->getNodeId(countNode)->getName().find("+"+to_string(n+1)) != std::string::npos){
				string nodeName = (dynamicBNet->getNodeId(countNode)->getName().substr(0, 1)) + "_t+" + to_string(n);
				int idOfPreviousNode = dynamicBNet->getIdOfNode(dynamicBNet->getNodeNamed(nodeName)); 
				dynamicBNet->setConditionalProbOfNode(countNode, dynamicBNet->getConditionalProbOfNode(idOfPreviousNode));
			}
		}
				
	}
}

/* Called periodically every getPeriod() seconds */
bool dbn::loop() {
    
  //loop rate at 60Hz max
    ros::Rate loop_rate(30);
    
    while(ros::ok()){
      
	ros::spinOnce();
	
	loop_rate.sleep();
    }
    ros::shutdown();
	
    return true;
}

dbn::~dbn(){
	cout<<"\nShutdown"<<endl;
}