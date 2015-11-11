/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#ifndef _DISPLAYNODE_H_
#define _DISPLAYNODE_H_

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>

#include <opencv/cv.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <unordered_map>

#include "BNode.h"
#include "BPVector.h"
#include "BNode.h"
#include "BNet.h"

using namespace std;
using namespace cv;


class displayNodes{

public:
    /** 
     * document your methods too.
     */
    displayNodes();
    
    void displayDBN(BNet* i_inputNet);
    
    ~displayNodes();


private:
    int width, height;
    
    unordered_map<string, Point> nodeToPosition;
    
    Mat canvas , canvasTop;
    
    void drawNodes(string name, Point position, int nbActivation = 0);
    void drawEdges(Point left, Point right);
 
};


#endif // _DISPLAYNODE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

