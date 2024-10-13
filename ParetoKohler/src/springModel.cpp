#include "springModel.hpp"
#include <iostream>
#include<math.h>
#include "ParetoApplication.h"



using namespace std;
extern ParetoApplication       topopt;
/*******************************************************/
springModel::springModel()
{
    sm_nNodes = 0;
    sm_xNodes = NULL;
    sm_yNodes = NULL;
    sm_zNodes = NULL;

    sm_nSprings = 0;
    sm_startNode = NULL;
    sm_endNode = NULL;

}
/*******************************************************/
void springModel::allocateNodeMemory(int nNodes)
{
    sm_nNodes = nNodes;
    sm_xNodes = (double *)malloc(nNodes*sizeof(double));
    sm_yNodes = (double *)malloc(nNodes*sizeof(double));
    sm_zNodes = (double *)malloc(nNodes*sizeof(double));

}
/*******************************************************/
springModel::~springModel(void)
{ 
    if (sm_xNodes) free(sm_xNodes);
    if (sm_yNodes) free(sm_yNodes);
    if (sm_zNodes) free(sm_zNodes);

    if (sm_startNode) free(sm_startNode);
    if (sm_endNode) free(sm_endNode);
}
