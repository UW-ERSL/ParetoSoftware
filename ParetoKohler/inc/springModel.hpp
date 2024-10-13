/*
 * gCodeReader.hpp

 * Author: aaditya Chandrasekhar
 */

#pragma once
#include <algorithm>
#include<iostream>
#include<math.h>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<string.h>
#include<time.h>
#include<limits>
#include <stdio.h>
#include <sstream>
#include <list>

using namespace std;

class springModel {
private:
    int sm_nNodes;
    double *sm_xNodes,*sm_yNodes,*sm_zNodes;

    int sm_nSprings;
    int *sm_startNode,*sm_endNode;




public:
            springModel();
    void    allocateNodeMemory(int nNodes);
            ~springModel();

};



