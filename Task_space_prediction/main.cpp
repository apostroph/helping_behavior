/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "dbn.h"

using namespace std;	

int main(int argc, char * argv[]) {
    /* initialize yarp network */ 
    Network yarp;

    /* prepare and configure the resource finder */
    ResourceFinder rf;
    rf.setVerbose(true); 
    rf.setDefaultConfigFile("Prediction.ini"); //overridden by --from parameter
    rf.setDefaultContext("Prediction/conf");   //overridden by --context parameter
    rf.configure(argc, argv);
    /* create your module */
    dbn module(rf); 
    /* run the module: runModule() calls configure first and, if successful, it then runs */
    module.runModule(rf);

    return 0;
}
