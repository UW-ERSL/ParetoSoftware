#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#define DIST3D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+(p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])))
using namespace std;
using namespace Eigen;

/**********************************************************************/
bool AFElasticity::InitializeDeflationForElasticity(void)
{
	if ((h_workSpaceInitialized) || (ws_deflationType == DEFLATION_NONE)) return true;
    if (h_deflation_verbose) writeMessageToParetoLog("AFElasticity: Attempting deflation with %d groups ",ws_nGroupsDesired);

	//create the groups
	CreateDeflationGroups();
	
	if (!computeSparseWKW()) {
		safeFree(ws_nodeGroupNumber);ws_nodeGroupNumber = NULL;
		safeFree(ws_groupSortedNodes);ws_groupSortedNodes = NULL;
		safeFree(h_CGPatch);h_CGPatch = NULL;
		ws_nGroups = 0;
		return false;// Compute WKW once, its LLT, etc
	}
	
	WorkSpaceTransferredToGPU = 0;
#ifdef COMPILE_GPUCODE
	if (usingGPU) {
		transferWorkSpaceToGPU();
	}
#endif

	h_workSpaceInitialized = true;
    if (h_deflation_verbose) writeMessageToParetoLog("AFElasticity: Deflation successful with %d groups ",ws_nGroupsDesired);
    //writeMessageToParetoLog("AFElasticity: Deflation successful with %d groups ",ws_nGroupsDesired);
	return true;
}
/**********************************************************************/
void AFElasticity::setNodeAssignmentMethod(int method)
{
	ws_nodeAssignmentMethod = method;
}

/**********************************************************************/
void AFElasticity::enableDeflationVerbose(void)
{
    writeMessageToParetoLog("AFElasticity: Enabling Deflation Verbose");
	h_deflation_verbose = true;
}
/**********************************************************************/
void AFElasticity::disableDeflationVerbose(void)
{
    writeMessageToParetoLog("AFElasticity: Disabling Deflation Verbose");
	h_deflation_verbose = false;
}
/**********************************************************************/
bool AFElasticity::CreateDeflationGroups(void)
{
	Timer timer;
	if ((h_workSpaceInitialized) || (ws_deflationType == DEFLATION_NONE)) return true;
    if (h_deflation_verbose) writeMessageToParetoLog("AFElasticity: Assigning nodes to %d groups ",ws_nGroupsDesired);

	timer.start();
	if (ws_nGroups > 0) {
		safeFree(ws_nodeGroupNumber);
		ws_nodeGroupNumber = NULL;
		safeFree(ws_groupSortedNodes);
		ws_groupSortedNodes = NULL;
	}

	ws_nodeGroupNumber = (int *) malloc(h_nNodes*sizeof(int));// which group each node belongs to
	ws_groupSortedNodes = (int *) malloc(h_nNodes*sizeof(int));// all nodes belonging to group 0 are first, then the next set of nodes
#pragma omp parallel for
	for (int i = 0; i < h_nNodes; i++){
		ws_nodeGroupNumber[i] = -1;
		ws_groupSortedNodes[i] = -1;
	}
	assignNodesToGroupsBoxMethod();//first assign nodes based on a background box grid
	if (ws_maxNodesInAnyGroup == 0){
        writeMessageToParetoLog("Deflation: Warning, max nodes across group: %d ... exiting ",ws_maxNodesInAnyGroup);
		return false;
	}

    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for assigning nodes %.2f ",timer.getElapsedTimeInSec());

	
	if (h_neighborNodes == NULL)  findNeighboringNodesOfNodes();// need this to proceed
	int nGroups = ws_nGroups; // the number of groups might increase, so store as a local variable
	for (int group = 0; group < nGroups; group++)
		if (!isGroupConnected(group)) 
			splitGroupIfDisconnected(group);
	
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for splitting groups %.2f ",timer.getElapsedTimeInSec());

	//fillEmptyGroups();// fill all empty groups by splitting groups with large number of nodes
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for filling empty groups %.2f ",timer.getElapsedTimeInSec());

	findCGOfPatches();// find CG of all groups
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for finding CG %.2f ",timer.getElapsedTimeInSec());

	removeSmallGroups();// remove groups with less than 9 nodes, and recompute CG
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for removing small groups %.2f ",timer.getElapsedTimeInSec());

	sortGroupNodes(); // create auxilliary data structure for speed-up
    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for creating %d groups %.2f ",ws_nGroups,timer.getElapsedTimeInSec());
	timer.stop();
	return true;

}
/**********************************************************************/
void AFElasticity::assignNodesToGroupsBoxMethod(void)
{
	//depending on the aspect ratio
	double temp,boundingBox[3][2],xMin,yMin,zMin;
	double  xLength, yLength, zLength, sizeX, sizeY, sizeZ,alpha;
	int factorX, factorY, factorZ, node, groupId, nX, nY, nZ;

    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Assigning nodes to groups: grid method");
	getBoundingBox(boundingBox);
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Boundingbox min: (%g, %g, %g) ",boundingBox[0][0],boundingBox[1][0],boundingBox[2][0]);
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Boundingbox max: (%g, %g, %g) ",boundingBox[0][1],boundingBox[1][1],boundingBox[2][1]);

    //writeMessageToParetoLog("\n ws_nGroupsDesired = %d ",ws_nGroupsDesired);
	xMin = boundingBox[0][0];
	yMin = boundingBox[1][0];
	zMin = boundingBox[2][0];

	xLength = boundingBox[0][1] -boundingBox[0][0];
	yLength = boundingBox[1][1] -boundingBox[1][0];
	zLength = boundingBox[2][1] -boundingBox[2][0];

	// Find the number of groups in x, y and z
	temp = (xLength*yLength*zLength);
	alpha = pow((double) (1.0*ws_nGroupsDesired)/temp,(double) 1.0/3);
	if ( (xLength < yLength) && (xLength < zLength)){
		nX = (int) ceil(alpha*xLength);
		temp = (yLength*zLength*nX);
		alpha = sqrt(1.0*ws_nGroupsDesired/temp);
		if (yLength < zLength){
			nY = (int) ceil(alpha*yLength);
			nZ = ceil(1.0*ws_nGroupsDesired/(nX*nY));
		}
		else{
			nZ= (int) ceil(alpha*zLength);
			nY = ceil(1.0*ws_nGroupsDesired/(nX*nZ));
		}
	}
	else if ( (yLength < xLength) && (yLength < zLength)){
		nY = (int) ceil(alpha*yLength);
		temp = (xLength*zLength*nY);
		alpha = sqrt(1.0*ws_nGroupsDesired/temp);
		if (xLength < zLength){
			nX = (int) ceil(alpha*xLength);
			nZ = ceil(1.0*ws_nGroupsDesired/(nX*nY));
		}
		else{
			nZ= (int) ceil(alpha*zLength);
			nX = ceil(1.0*ws_nGroupsDesired/(nY*nZ));
		}
	}
	else {
		nZ = (int) ceil(alpha*zLength);
		temp = (xLength*yLength*nZ);
		alpha = sqrt(1.0*ws_nGroupsDesired/temp);
		if (xLength < yLength){
			nX = (int) ceil(alpha*xLength);
			nY = ceil(1.0*ws_nGroupsDesired/(nX*nZ));
		}
		else{
			nY= (int) ceil(alpha*yLength);
			nX = ceil(1.0*ws_nGroupsDesired/(nY*nZ));
		}
	}
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: deflation grid (%d,%d,%d) ",nX,nY,nZ);
	ws_nGroups = nX*nY*nZ;
	sizeX = xLength/nX; sizeY = yLength/nY; sizeZ = zLength/nZ;//average length of groups
	// assign nodes to a group
	for (node = 0; node < h_nNodes; node++) {
		if (doesNodeExist(node)) {
			factorX = MIN(floor((h_x[node] - xMin)/sizeX),nX-1);
			factorY = MIN(floor((h_y[node] - yMin)/sizeY),nY-1);
			factorZ = MIN(floor((h_z[node] - zMin)/sizeZ),nZ-1);
			groupId = factorX + nX*factorY + nX*nY*factorZ;
			ws_nodeGroupNumber[node] = groupId;
		}
	}

	if (ws_nGroups > 0) {
		safeFree(ws_groupCount);
		ws_groupCount = NULL;
		safeFree(ws_anyNodeInGroup);
		ws_anyNodeInGroup = NULL;
	}
	ws_groupCount = (int *) malloc(ws_nGroups*sizeof(int));// how many nodes in each group
	memset(ws_groupCount, 0, ws_nGroups*sizeof(int));
	
	ws_anyNodeInGroup = (int *) malloc(ws_nGroups*sizeof(int));
	for (int group = 0; group < ws_nGroups; group++)
		ws_anyNodeInGroup[group] = -1;

	for (int i = 0; i < h_nNodes; i++){
		if (doesNodeExist(i)) {
			groupId = ws_nodeGroupNumber[i];
			ws_groupCount[groupId]++;//first count how many nodes there are in each group
			if (ws_anyNodeInGroup[groupId] == -1)// not assigned as yet
				ws_anyNodeInGroup[groupId] = i;
		}
	}
	ws_nEmptyGroups = 0;
	ws_maxNodesInAnyGroup = 0;
	for (groupId= 0; groupId < ws_nGroups; groupId++){
		ws_maxNodesInAnyGroup = MAX(ws_maxNodesInAnyGroup,ws_groupCount[groupId]);
		if (ws_groupCount[groupId] == 0) 
			ws_nEmptyGroups++;
	}

	return;
}
/**********************************************************************/
bool AFElasticity::removeSmallGroups(void)
{
	int groupId,i;
	double pt1[3],pt2[3],distMax;
	int thisGroup,otherGroup,groupToMoveTo;
	bool groupRemoved;

    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Removing small groups");
	// we need at least 9 nodes per group, else LLT fails later on
	//  we move nodes from such groups to nearby groups
	int *groupMapping = (int *) malloc(ws_nGroups*sizeof(int));
	for (i = 0; i < ws_nGroups; i++)
		groupMapping[i] = i;// default

	groupRemoved = false;
	for (thisGroup = 0; thisGroup < ws_nGroups; thisGroup++){
		if ((ws_groupCount[thisGroup] > 0) && (ws_groupCount[thisGroup] < MIN_WORKSPACE_NODES)){// non-empty but insufficient
			groupRemoved = true;
			pt1[0] = h_CGPatch[3*thisGroup];
			pt1[1] = h_CGPatch[3*thisGroup+1];
			pt1[2] = h_CGPatch[3*thisGroup+2];
			distMax = HUGE_VAL;
			groupToMoveTo = -1;
			for (otherGroup = 0; otherGroup < ws_nGroups; otherGroup++){// find a nearby group
				if (otherGroup == thisGroup) continue;
				if (ws_groupCount[otherGroup] < MIN_WORKSPACE_NODES) continue;// not a good choice
				pt2[0] = h_CGPatch[3*otherGroup];
				pt2[1] = h_CGPatch[3*otherGroup+1];
				pt2[2] = h_CGPatch[3*otherGroup+2];
				if (DIST3D(pt1,pt2) < distMax){
					distMax = DIST3D(pt1,pt2);
					groupToMoveTo = otherGroup;
				}
			}
			if (groupToMoveTo >= 0){
				groupMapping[thisGroup] = groupToMoveTo;
                //writeMessageToParetoLog("Moving %d to %d ",thisGroup,groupToMoveTo);
			}
		}	
	}
	// now reassign all nodes
	if (groupRemoved){ 
		for (thisGroup = 0; thisGroup < ws_nGroups; thisGroup++)
			ws_groupCount[thisGroup] = 0;
		for (i = 0; i < h_nNodes; i++){
			if (doesNodeExist(i)) {
				groupId = ws_nodeGroupNumber[i];
				ws_nodeGroupNumber[i] = groupMapping[groupId];
				ws_groupCount[groupMapping[groupId]]++;
			}
		}
		findCGOfPatches();// redo this step
	}
	
	free(groupMapping);
	return (groupRemoved);
}
/**********************************************************************/
void AFElasticity::sortGroupNodes(void)
{
	int i,node,groupId,index,*groupCountTemp;

    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Sorting groups ");

	// We will need to put the nodes in a sorted list per group number for easy acccess
	// So, if group 0 has 10 nodes, then these 10 node numbers will first appear in ws_groupSortedNodes
	// Next the nodes of group 1 will appear and so on.
	if (ws_nGroups > 0) {
		safeFree(ws_groupEndNodeIndex);
		ws_groupEndNodeIndex = NULL;
	}
	
	ws_groupEndNodeIndex = (int *) malloc(ws_nGroups*sizeof(int));
	memset(ws_groupEndNodeIndex, 0, ws_nGroups*sizeof(int));

	// then find out the start and end location of the sorted list ... needed later on
	ws_groupEndNodeIndex[0] = ws_groupCount[0];// group 0 has that many nodes
	for (i = 1; i < ws_nGroups; i++)
		ws_groupEndNodeIndex[i] = ws_groupEndNodeIndex[i - 1] + ws_groupCount[i];//cumulative index


	groupCountTemp = (int *) malloc(ws_nGroups*sizeof(int));
	memset(groupCountTemp, 0, ws_nGroups*sizeof(int));
	for (node = 0; node < h_nNodes; node++){//now make the sorted list
		if (doesNodeExist(node)) {
			groupId = ws_nodeGroupNumber[node];
			if (groupId == 0)
				index = groupCountTemp[groupId];
			else
				index = groupCountTemp[groupId] + ws_groupEndNodeIndex[groupId - 1];
			ws_groupSortedNodes[index] = node;	//puts in the correct location
			groupCountTemp[groupId]++;
		}
	}
	free(groupCountTemp);
}
/**********************************************************************/
bool AFElasticity::isGroupConnected(int group)
{
	int startNode,nodeInFrontOfQ,neighborNode,nNodesInGroup;
	bool *nodeVisited;
	Queue Q;

    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Checking connectivity of group %d",group);

	if (ws_groupCount[group] <= 1)
		return true;// zero or 1 node in group

	if (ws_groupCount[group] >= ws_maxNodesInAnyGroup)
		return true;// with max nodes, there is no way the group is disconnected

	nodeVisited = (bool *)malloc(h_nNodes*sizeof(bool));
	memset(nodeVisited,false,h_nNodes*sizeof(bool));
	// start with any node in the group
	// use neighborhood to mark all nodes connected to it
	// count the nodes visited
	// compare against the total number of nodes in group

	//startNode = ws_anyNodeInGroup[group];
	
	for (int node = 0; node < h_nNodes; node++) {
		if (!doesNodeExist(node)) continue;
		if (ws_nodeGroupNumber[node] != group) continue;
		startNode = node;
		break;
	}

	Q = CreateQueue(h_nNodes);// worst case

	MakeEmpty(Q);
	Enqueue(startNode, Q); // add start node
	nodeVisited[startNode] = true;
	nNodesInGroup = 1;
	while (!IsEmpty(Q)) {
		nodeInFrontOfQ = Front(Q);
		Dequeue(Q);//remove the node from Q
		for (int i =0; i < 27; i++) {// consider all neighbors
			neighborNode = h_neighborNodes[i*h_nNodes + nodeInFrontOfQ];
			if ((neighborNode != -1) && (!nodeVisited[neighborNode]) &&  (ws_nodeGroupNumber[neighborNode] == group)){//not visited
				Enqueue(neighborNode, Q);
				nNodesInGroup++;
			}
			if (neighborNode != -1) 
				nodeVisited[neighborNode] = true;//mark as visitd
		}
	}

	free(nodeVisited);
	DisposeQueue(Q);

	if (nNodesInGroup == ws_groupCount[group])
		return true;
	else
		return false;
}
/**********************************************************************/
void AFElasticity::splitGroupIfDisconnected(int groupToSplit)
{
	int firstNodeInGroup,firstNodeInConnectedSet,neighborNode,currentGroup;
	Queue queueOfNodesInGroup,queueOfConnectedNodes;
	// all nodes in a group must be connected, if not they must be split (will improve CG performance)

	if (ws_groupCount[groupToSplit] <= 1) return;// zero or 1 node in group, need not split
	// we will need two groups
	queueOfNodesInGroup = CreateQueue(h_nNodes);// worst case
	queueOfConnectedNodes = CreateQueue(h_nNodes);// worst case
	
	for (int node = 0; node < h_nNodes; node++) {
		if (!doesNodeExist(node)) continue;
		if (ws_nodeGroupNumber[node] != groupToSplit) continue;
		Enqueue(node, queueOfNodesInGroup); // add node to the master list
		ws_nodeGroupNumber[node] = -1;// unassign group number
	}
	currentGroup = groupToSplit;// this is the current group
	bool firstPass = true;
	ws_groupCount[groupToSplit] = 0;
	while (!IsEmpty(queueOfNodesInGroup)) {// while there are no more nodes in group
		firstNodeInGroup = Front(queueOfNodesInGroup);
		Dequeue(queueOfNodesInGroup);//remove the node from group queue
		if (ws_nodeGroupNumber[firstNodeInGroup] >= 0) continue;// we have already classified this node
		if (!firstPass) {
			currentGroup = findAnEmptyGroup();
			if (currentGroup == -1){
				currentGroup = createAnEmptyGroup();// creates a new empty group
				ws_groupCount[currentGroup] = 0;
				ws_nEmptyGroups--;// we will add to this group
			}
		}
		MakeEmpty(queueOfConnectedNodes);// first empty the connected group
		Enqueue(firstNodeInGroup, queueOfConnectedNodes); // add node to this new group
		ws_nodeGroupNumber[firstNodeInGroup] = currentGroup;// assign the group number
		ws_groupCount[currentGroup]++;
		while (!IsEmpty(queueOfConnectedNodes)) {// while there are no more connected nodes
			firstNodeInConnectedSet = Front(queueOfConnectedNodes);
			Dequeue(queueOfConnectedNodes);//remove the node from group queue
			for (int i =0; i < 27; i++) {// consider all neighbors
				neighborNode = h_neighborNodes[i*h_nNodes + firstNodeInConnectedSet];
				if ((neighborNode != -1) && (ws_nodeGroupNumber[neighborNode] < 0) ){//not visited
					Enqueue(neighborNode, queueOfConnectedNodes);
					ws_nodeGroupNumber[neighborNode] = currentGroup;
					ws_groupCount[currentGroup]++;
				}
			}
		}
		firstPass = false;
	}
	DisposeQueue(queueOfNodesInGroup);
	DisposeQueue(queueOfConnectedNodes);
}
/**********************************************************************/
void AFElasticity::fillEmptyGroups(void)
{
	double *groupCount;
	int group,*sortIndex,numEmptyGrps;


    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Filling empty groups");
	groupCount = (double *)malloc(ws_nGroups*sizeof(double));
	memset(groupCount,0,ws_nGroups*sizeof(double));
	sortIndex =(int *)malloc(ws_nGroups*sizeof(int));
	numEmptyGrps = 0;
	for (int i = 0; i < ws_nGroups; i++){
		groupCount[i] = (1.0)*ws_groupCount[i];
		sortIndex[i] = i;
		if (groupCount[i] == 0) numEmptyGrps++;	
	}

	bubbleSortIncreasing(groupCount,sortIndex,ws_nGroups);

	for (int index = ws_nGroups-1; index >= ws_nGroups-numEmptyGrps-1 ; index--) {
		group = sortIndex[index];
		splitAGroup(group,-1);
	}
	ws_nEmptyGroups = 0;
	for (int i = 0; i < ws_nGroups; i++)
		if (ws_groupCount[i] == 0) 
			ws_nEmptyGroups++;	

	free(groupCount);
	free(sortIndex);

}
/**********************************************************************/
int	AFElasticity::findAnEmptyGroup(void)
{
	// look for an empty group
	for (int group = 0; group < ws_nGroups;group++){
		if (ws_groupCount[group] == 0) 
			return group;
	}
	//else
	return -1;
}
/**********************************************************************/
int	AFElasticity::createAnEmptyGroup(void)
{
    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Creating a new group");

	ws_groupCount = (int *) realloc(ws_groupCount,(ws_nGroups+1)*sizeof(int));
	ws_nGroups++;
	ws_groupCount[ws_nGroups-1] = 0;
	ws_nEmptyGroups++;
	return ws_nGroups-1;
}
/**********************************************************************/
bool	AFElasticity::splitAGroup(int groupToSplit,int splitDirection)
{
	// create new groups

	double boundingBoxGroup[3][2],range[3],midpt[3];
	int newGroupId;

	if (ws_groupCount[groupToSplit] == 0) return false;

	newGroupId = findAnEmptyGroup();
	if (newGroupId == -1) return false;

	// find the direction to split
	for (int i = 0; i < 3; i++){
		boundingBoxGroup[i][0] = HUGE_VAL;
		boundingBoxGroup[i][1] = -HUGE_VAL;
	}
	for (int node = 0; node < h_nNodes; node++){
		if (ws_nodeGroupNumber[node] != groupToSplit) continue;
		boundingBoxGroup[0][0] = MIN(boundingBoxGroup[0][0],h_x[node]);
		boundingBoxGroup[0][1] = MAX(boundingBoxGroup[0][1],h_x[node]);

		boundingBoxGroup[1][0] = MIN(boundingBoxGroup[1][0],h_y[node]);
		boundingBoxGroup[1][1] = MAX(boundingBoxGroup[1][1],h_y[node]);

		boundingBoxGroup[2][0] = MIN(boundingBoxGroup[2][0],h_z[node]);
		boundingBoxGroup[2][1] = MAX(boundingBoxGroup[2][1],h_z[node]);
	}
	for (int i = 0; i < 3; i++){
		range[i] = boundingBoxGroup[i][1] - boundingBoxGroup[i][0];
		midpt[i] = (boundingBoxGroup[i][1] + boundingBoxGroup[i][0])/2.0;
	}

	if (splitDirection == -1) {// no preferred direction
		splitDirection = 0;
		if ( (range[1] > range[0]) && (range[1] > range[0])) splitDirection = 1;
		if ( (range[2] > range[0]) && (range[2] > range[1])) splitDirection = 2;
	}

	ws_nEmptyGroups--;//decrease nEmptyGroups
	ws_groupCount[newGroupId] = 0;
	// since there is an empty group, we don't need to increase ws_nGroups
	// walk thru all nodes of in groupToSplit
	for (int node = 0; node < h_nNodes; node++){
		if (ws_nodeGroupNumber[node] != groupToSplit) continue;
		if ( ((splitDirection == 0) && (h_x[node] > midpt[0])) ||
			 ((splitDirection == 1) && (h_y[node] > midpt[1])) ||
			 ((splitDirection == 2) && (h_z[node] > midpt[2])) ){// split by hhalf plane
			ws_nodeGroupNumber[node] = newGroupId;
			ws_groupCount[newGroupId]++;
			ws_groupCount[groupToSplit]--;
		}
	}
	return true;
}
/**********************************************************************/
void AFElasticity::findCGOfPatches(void)
{
	int groupId;


    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Finding CG of patches");
	if (ws_nGroups > 0) {
		safeFree(h_CGPatch);
		h_CGPatch = NULL;
	}
	h_CGPatch = (double *)malloc(3*ws_nGroups*sizeof(double));
	memset(h_CGPatch,0,3*ws_nGroups*sizeof(double));
	
	for (int i = 0; i < h_nNodes; i++){
		if (doesNodeExist(i)) {
			groupId = ws_nodeGroupNumber[i];
			h_CGPatch[3*groupId] += h_x[i];
			h_CGPatch[3*groupId+1] += h_y[i];
			h_CGPatch[3*groupId+2] += h_z[i];
		}
	}
	for (groupId= 0; groupId < ws_nGroups; groupId++){
		h_CGPatch[3*groupId] /=ws_groupCount[groupId];
		h_CGPatch[3*groupId+1] /=ws_groupCount[groupId];
		h_CGPatch[3*groupId+2] /=ws_groupCount[groupId];
	}

}
/**********************************************************************/
void AFElasticity::printWorkSpaceGroupData(void)
{
	for (int groupId= 0; groupId < ws_nGroups; groupId++)
        writeMessageToParetoLog("groupId: %d, #Nodes = %d ",groupId,ws_groupCount[groupId]);

    writeMessageToParetoLog("---------------------");
}
/**********************************************************************/
int AFElasticity::getWorkspaceGroup(int node) 
{
	if (doesNodeExist(node))
		return ws_nodeGroupNumber[node];
	else 
		return 0;
}
/**********************************************************************/
int AFElasticity::getNumNodesInGroup(int group)
{
	return ws_groupCount[group];
}
/**********************************************************************/
int AFElasticity::getNumEmptyGroups(void) 
{
	return ws_nEmptyGroups;
}
/**********************************************************************/
int AFElasticity::getNumWorkspaceGroups(void) 
{
	return ws_nGroups;
}
/**********************************************************************/
//Allocate double* WKW, and check whether ws_nodeGroupNumber is in place??
bool AFElasticity::computeSparseWKW()// Neumann condition assumed, diriclet applied after K*u
{

    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Constructing dense WKW ");
	Timer timer;
	timer.start();
	
	if (ws_deflationType == DEFLATION_NONE) return true;
	if (ws_deflationType != DEFLATION_RIGID) return false;

	// a dense matrix denseWKW is created to save on time
	// then a sparse matrix sparseKTilde is extracted from it.
	KreductionRBM();// rigid body

    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for constructing WKW is %.2f (secs) ",timer.getElapsedTimeInSec());


	sparseWKW = denseWKW.sparseView();
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Sparse KTilde; #rows = %d; nnz = %d ",sparseWKW.rows(),sparseWKW.nonZeros());
    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for sparse view is %.2f (secs) ",timer.getElapsedTimeInSec());
	
	sparseSolver.compute(sparseWKW);
    //if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time for sparse LLT %.2f (secs) ",timer.getElapsedTimeInSec());

	if (sparseSolver.info() != 0) {
        if (h_deflation_verbose) writeMessageToParetoLog("\nDeflation: Sparse LLT decomposition failed with %d groups ",ws_nGroups);
		return false;
	}
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: SparseWKW constructed");

	sparseWKW_Lower = sparseSolver.matrixL();// this is permuted
	MatrixXd sparsePermute = sparseSolver.permutationP();

	sparsePermuteVector.resize(sparseWKW.rows());
	for (int row = 0; row < sparsePermute.rows(); row++){
		for (int col = 0; col < sparsePermute.cols(); col++){
			if (sparsePermute(row,col) > 0.5){// 0 or 1
				sparsePermuteVector(row) = col;
				break;
			}
		}
	}
	denseWKW.resize(1,1);// release the dense matrix 
	timer.stop();
	return true;
}

/******************************************************************************/
void AFElasticity::KreductionRBM()
{

	denseWKW.resize(6*ws_nGroups,6*ws_nGroups);// Initialize WKW (Here 'denseWKW')
	denseWKW.setZero(6*ws_nGroups,6*ws_nGroups);

	for ( int k = 0; k < h_nElems; k++) {
		if (!h_simpMode && h_pseudoDensity[k] <0.5) continue; // element exists but is void (example: during topology optimization)
		elementWKW(k);
	}
	int rowStart,rowEnd;
	double maxValue = denseWKW.maxCoeff();
	for (int g = 0; g <ws_nGroups;g++) {
		if (ws_groupCount[g] == 0){
			rowStart = 6*g;
			rowEnd = 6*g + 6;
			for (int i = rowStart; i < rowEnd;i++) 
				denseWKW(i,i) = maxValue;
		}
	}
}
/******************************************************************************/
void AFElasticity::elementWKW(int elem) 
{
	int freeEncoded, u1, u2, v1, v2, w1, w2, g2, g1, n1, n2;
	int node_i, node_j, elemID;
	double K_uu, K_uv, K_uw, K_vu, K_vv, K_vw, K_wu, K_wv, K_ww,x1,y1,z1,x2,y2,z2,EScale;
	
	elemID = h_templateID[elem];
	EScale = pow(h_pseudoDensity[elem],h_Eexponent);// E = E0*(rho)^n;
	for (node_i = 0; node_i < 8; node_i++) {
		//node of a single voxel // d-o-f of node (u, v, w)
		n1 = h_elems[node_i*h_nElems + elem]; 
		g1 = ws_nodeGroupNumber[n1]; // Group number on mesh

		freeEncoded = h_free[n1];
		w1 = freeEncoded/4;
		v1 = (freeEncoded - w1*4)/2;
		u1 = (freeEncoded - w1*4 -v1*2);
			
		if ( (u1 == 0) && (v1 == 0) && (w1 == 0)) 
			continue;
		
		x1 = (h_x[n1] - h_CGPatch[3*g1 + 0]);
		y1 = (h_y[n1] - h_CGPatch[3*g1 + 1]);
		z1 = (h_z[n1] - h_CGPatch[3*g1 + 2]);
		

		for (node_j = 0; node_j < 8; node_j++) { 
			n2 = h_elems[node_j*h_nElems + elem]; 
			g2 = ws_nodeGroupNumber[n2];
								
			freeEncoded = h_free[n2];
			w2 = freeEncoded/4;
			v2 = (freeEncoded - w2*4)/2;
			u2 = (freeEncoded - w2*4 -v2*2);

			if ( (u2 == 0) && (v2 == 0) && (w2 == 0)) 
				continue;
			
			x2 = (h_x[n2] - h_CGPatch[3*g2 + 0]);
			y2 = (h_y[n2] - h_CGPatch[3*g2 + 1]);
			z2 = (h_z[n2] - h_CGPatch[3*g2 + 2]);
		
			// For 9 cases use K_val and index to populate WKW using only required node number and nodeNumber information
			K_uu = h_KETemplates[(24*(3*node_i) + 3*node_j)*(h_nTemplateElements) + elemID]*u1*u2*EScale;
			K_uv = h_KETemplates[(24*(3*node_i) + 3*node_j + 1)*(h_nTemplateElements) + elemID]*u1*v2*EScale;
			K_uw = h_KETemplates[(24*(3*node_i) + 3*node_j + 2)*(h_nTemplateElements) + elemID]*u1*w2*EScale;
			K_vu = h_KETemplates[(24*(3*node_i + 1) + 3*node_j)*(h_nTemplateElements) + elemID]*v1*u2*EScale;
			K_vv = h_KETemplates[(24*(3*node_i + 1) + 3*node_j + 1)*(h_nTemplateElements) + elemID]*v1*v2*EScale;
			K_vw = h_KETemplates[(24*(3*node_i + 1) + 3*node_j + 2)*(h_nTemplateElements) + elemID]*v1*w2*EScale;
			K_wu = h_KETemplates[(24*(3*node_i + 2) + 3*node_j)*(h_nTemplateElements) + elemID]*w1*u2*EScale; 
			K_wv = h_KETemplates[(24*(3*node_i + 2) + 3*node_j + 1)*(h_nTemplateElements) + elemID]*w1*v2*EScale; 
			K_ww = h_KETemplates[(24*(3*node_i + 2) + 3*node_j + 2)*(h_nTemplateElements) + elemID]*w1*w2*EScale;

			denseWKW(6*g1,6*g2) += K_uu;
			denseWKW(6*g1,6*g2 + 1) += K_uv;
			denseWKW(6*g1,6*g2 + 2) += K_uw;//[2,0)
			denseWKW(6*g1,6*g2 + 3) += (K_uw*y2 - K_uv*z2);
			denseWKW(6*g1,6*g2 + 4) += (K_uu*z2 - K_uw*x2);
			denseWKW(6*g1,6*g2 + 5) += (K_uv*x2 - K_uu*y2);

			denseWKW(6*g1 + 1,6*g2) += K_vu;
			denseWKW(6*g1 + 1,6*g2 + 1) += K_vv;
			denseWKW(6*g1 + 1,6*g2 + 2) += K_vw;//[1,2)
			denseWKW(6*g1 + 1,6*g2 + 3) += (K_vw*y2 - K_vv*z2);
			denseWKW(6*g1 + 1,6*g2 + 4) += (K_vu*z2 - K_vw*x2);//[1,4)
			denseWKW(6*g1 + 1,6*g2 + 5) += (K_vv*x2 - K_vu*y2);

			denseWKW(6*g1 + 2,6*g2) += K_wu;//[0,2)
			denseWKW(6*g1 + 2,6*g2 + 1) += K_wv;//[2,1)
			denseWKW(6*g1 + 2,6*g2 + 2) += K_ww;
			denseWKW(6*g1 + 2,6*g2 + 3) += (K_ww*y2 - K_wv*z2);
			denseWKW(6*g1 + 2,6*g2 + 4) += (K_wu*z2 - K_ww*x2);
			denseWKW(6*g1 + 2,6*g2 + 5) += (K_wv*x2 - K_wu*y2);//[2,5)

			denseWKW(6*g1 + 3,6*g2) += (K_wu*y1 - K_vu*z1);//[3,0)
			denseWKW(6*g1 + 3,6*g2 + 1) += (K_wv*y1 - K_vv*z1);
			denseWKW(6*g1 + 3,6*g2 + 2) += (K_ww*y1 - K_vw*z1);
			denseWKW(6*g1 + 3,6*g2 + 3) += (y2*(K_ww*y1 - K_vw*z1) - z2*(K_wv*y1 - K_vv*z1));
			denseWKW(6*g1 + 3,6*g2 + 4) += (z2*(K_wu*y1 - K_vu*z1) - x2*(K_ww*y1 - K_vw*z1));
			denseWKW(6*g1 + 3,6*g2 + 5) += (x2*(K_wv*y1 - K_vv*z1) - y2*(K_wu*y1 - K_vu*z1));

			denseWKW(6*g1 + 4,6*g2) += (K_uu*z1 - K_wu*x1);
			denseWKW(6*g1 + 4,6*g2 + 1) += (K_uv*z1 - K_wv*x1);//[4,1)
			denseWKW(6*g1 + 4,6*g2 + 2) += (K_uw*z1 - K_ww*x1);
			denseWKW(6*g1 + 4,6*g2 + 3) += (z2*(K_wv*x1 - K_uv*z1) - y2*(K_ww*x1 - K_uw*z1));
			denseWKW(6*g1 + 4,6*g2 + 4) += (x2*(K_ww*x1 - K_uw*z1) - z2*(K_wu*x1 - K_uu*z1));
			denseWKW(6*g1 + 4,6*g2 + 5) += (y2*(K_wu*x1 - K_uu*z1) - x2*(K_wv*x1 - K_uv*z1));

			denseWKW(6*g1 + 5,6*g2) += (K_vu*x1 - K_uu*y1);
			denseWKW(6*g1 + 5,6*g2 + 1) += (K_vv*x1 - K_uv*y1);
			denseWKW(6*g1 + 5,6*g2 + 2) += (K_vw*x1 - K_uw*y1);//[5,2)
			denseWKW(6*g1 + 5,6*g2 + 3) += (y2*(K_vw*x1 - K_uw*y1) - z2*(K_vv*x1 - K_uv*y1));
			denseWKW(6*g1 + 5,6*g2 + 4) += (z2*(K_vu*x1 - K_uu*y1) - x2*(K_vw*x1 - K_uw*y1));
			denseWKW(6*g1 + 5,6*g2 + 5) += (x2*(K_vv*x1 - K_uv*y1) - y2*(K_vu*x1 - K_uu*y1));
		}
	}
}
/**********************************************************************/
void AFElasticity::getWorkSpaceVector(double* inVector, VectorXd &outVector)
/* Function to transform full vector in to a workspace vector
	Input: int allDof (nodes * dof/node), int nDof (free degrees of freedom)
		pre allocated pointers for
		- coordinates (3 x allDof)
		- array of freeDOFs (1 x nDof)
		- input Vector in solution space (1 x nDof)
		- output Vector of workspace (1 x 6*#groups)
		*/
{
	int group, node, start = 0, stop, index;
	double gX, gY, gZ, xnode,ynode,znode, nu = 0.28;

	start = 0;
	if (ws_deflationType == DEFLATION_RIGID) {
		for (int i =0; i < 6*ws_nGroups; i++)
			outVector(i) = 0.0;
		//#pragma omp parallel for private(group,gX,gY,gZ,start,stop,node,index,xnode,ynode,znode)
		for (group = 0; group < ws_nGroups; group++){
			gX = h_CGPatch[group*3];
			gY = h_CGPatch[group*3 + 1];
			gZ = h_CGPatch[group*3 + 2];
			if (group != 0) 
				start = ws_groupEndNodeIndex[group - 1];
			else
				start = 0;
			stop = ws_groupEndNodeIndex[group];
			for (index = start; index < stop; index ++){
				node = ws_groupSortedNodes[index];
				if(doesNodeExist(node)) {
					xnode = (h_x[node] - gX);
					ynode = (h_y[node] - gY);
					znode = (h_z[node] - gZ);

					outVector(6*group)		+= *(inVector + 3*node);
					outVector(6*group + 4)	+= *(inVector + 3*node) * znode;
					outVector(6*group + 5)	-= *(inVector + 3*node) * ynode;

					outVector(6*group + 1)	+= *(inVector + 3*node + 1) ;
					outVector(6*group + 3)	-= *(inVector + 3*node + 1) * znode;
					outVector(6*group + 5)	+= *(inVector + 3*node + 1) * xnode;

					outVector(6*group + 2)	+= *(inVector + 3*node + 2) ;
					outVector(6*group + 3)	+= *(inVector + 3*node + 2) * ynode;
					outVector(6*group + 4)	-= *(inVector + 3*node + 2) * xnode;
				}
			}
		}
	}
	else if (ws_deflationType == DEFLATION_TRANSLATION) {
		for (int i =0; i < 3*ws_nGroups; i++)
			outVector(i) = 0.0;
		#pragma omp parallel for private(group,start,stop,node,index)
		for (group = 0; group < ws_nGroups; group++){

			if (group != 0) start = ws_groupEndNodeIndex[group - 1];
			stop = ws_groupEndNodeIndex[group];
			for (index = start; index < stop; index ++){
				node = ws_groupSortedNodes[index];
				if(doesNodeExist(node)) {
					
					outVector(3*group)		+= *(inVector + 3*node);
					outVector(3*group + 1)	+= *(inVector + 3*node + 1);
					outVector(3*group + 2)	+= *(inVector + 3*node + 2);

				}
			}
		}
	}
	else if (ws_deflationType == DEFLATION_BEAM) {
		for (int i =0; i < 7*ws_nGroups; i++)
			outVector(i) = 0.0;
		#pragma omp parallel for private(group,gX,gY,gZ,start,stop,node,index,xnode,ynode,znode)
		for (group = 0; group < ws_nGroups; group++){
			gX = h_CGPatch[group*3];
			gY = h_CGPatch[group*3 + 1];
			gZ = h_CGPatch[group*3 + 2];
			if (group != 0) start = ws_groupEndNodeIndex[group - 1];
			stop = ws_groupEndNodeIndex[group];
			for (index = start; index < stop; index ++){
				node = ws_groupSortedNodes[index];
				if(doesNodeExist(node))
				{
					xnode = (h_x[node] - gX);
					ynode = (h_y[node] - gY);
					znode = (h_z[node] - gZ);

					outVector[7*group]		+= *(inVector + 3*node);
					outVector[7*group + 1]	+= *(inVector + 3*node + 1);
					outVector[7*group + 2]	+= *(inVector + 3*node + 2);
					outVector[7*group + 3]	+= *(inVector + 3*node + 2) * ynode - *(inVector + 3*node + 1) * znode;
					outVector[7*group + 4]	+= *(inVector + 3*node) * znode - *(inVector + 3*node + 2) * xnode;
					outVector[7*group + 5]	+= *(inVector + 3*node + 1) * xnode - *(inVector + 3*node) * ynode;
					outVector[7*group + 6]	+= (*(inVector + 3*node + 2) * xnode/2 - *(inVector + 3*node) * znode) * xnode;

				}
			}
		}
	}
	else if (ws_deflationType == DEFLATION_PLANE_STRESS){
		for (int i =0; i < 3*ws_nGroups; i++)
			outVector(i) = 0.0;
		#pragma omp parallel for private(group,gX,gY,gZ,start,stop,node,index,xnode,ynode,znode)
		for (group = 0; group < ws_nGroups; group++){
			gX = h_CGPatch[group*3];
			gY = h_CGPatch[group*3 + 1];
			gZ = h_CGPatch[group*3 + 2];
			if (group != 0) start = ws_groupEndNodeIndex[group - 1];
			stop = ws_groupEndNodeIndex[group];
			for (index = start; index < stop; index ++){
				node = ws_groupSortedNodes[index];
				if(doesNodeExist(node)) {
					xnode = (h_x[node] - gX);
					ynode = (h_y[node] - gY);
					znode = (h_z[node] - gZ);

					outVector(3*group)		+= *(inVector + 3*node);
					outVector(3*group + 2)	-= *(inVector + 3*node) * ynode;

					outVector(3*group + 1)	+= *(inVector + 3*node + 1) ;
					outVector(3*group + 2)	+= *(inVector + 3*node+1) * xnode;

				}
			}
		}
	}
	else {
		for (int i =0; i < 9*ws_nGroups; i++)
			outVector(i) = 0.0;
		#pragma omp parallel for private(group,gX,gY,gZ,start,stop,node,index,xnode,ynode,znode)
		for (group = 0; group < ws_nGroups; group++){
			gX = h_CGPatch[group*3];
			gY = h_CGPatch[group*3 + 1];
			gZ = h_CGPatch[group*3 + 2];
			if (group != 0) start = ws_groupEndNodeIndex[group - 1];
			stop = ws_groupEndNodeIndex[group];
			for (index = start; index < stop; index ++){
				node = ws_groupSortedNodes[index];
				if(doesNodeExist(node))
				{
					xnode = (h_x[node] - gX);
					ynode = (h_y[node] - gY);
					znode = (h_z[node] - gZ);

					outVector[9*group]		+= *(inVector + 3*node);
					outVector[9*group + 1]	+= *(inVector + 3*node + 1);
					outVector[9*group + 2]	+= *(inVector + 3*node + 2);
					outVector[9*group + 3]	+= *(inVector + 3*node + 2) * ynode - *(inVector + 3*node + 1) * znode;
					outVector[9*group + 4]	+= *(inVector + 3*node) * znode - *(inVector + 3*node + 2) * xnode;
					outVector[9*group + 5]	+= *(inVector + 3*node + 1) * xnode - *(inVector + 3*node) * ynode;
					outVector[9*group + 6]	+= (*(inVector + 3*node + 2) * xnode/2 - *(inVector + 3*node) * znode) * xnode;
					outVector[9*group + 7]	+= (*(inVector + 3*node + 2) * ynode/2 - *(inVector + 3*node + 1) * znode) * ynode;
					outVector[9*group + 8]	+= *(inVector + 3*node + 2) * xnode * ynode - *(inVector + 3*node + 1) * znode * xnode
						- *(inVector + 3*node) * znode * ynode; 
				}
			}
		}
	}

}

/**********************************************************************/
void AFElasticity::getSolutionSpaceVector(VectorXd &inVector, double* outVector)
/* Function to transform workspace vector in to a full vector
	Input: int allDof (nodes * dof/node), int nDof (free degrees of freedom)
		pre allocated pointers for
		- coordinates (3 x allDof)
		- group array or ws_nodeGroupNumber (1 x allDof)
		- array of freeDOFs (1 x nDof)
		- input Vector of workspace (1 x 6*#groups)
		- output Vector in solution space (1 x nDof)
		*/
{
	int group, node;
	double xnode,ynode,znode, nu = 0.28;

	memset(outVector, 0, 3*h_nNodes*sizeof(double));
	
	if (ws_deflationType == DEFLATION_RIGID){
		#pragma omp parallel for private(node,xnode,ynode,znode,group)
		for (node = 0; node < h_nNodes; node++)	{
			if (!doesNodeExist(node)) continue;
			group = ws_nodeGroupNumber[node];
			xnode = (h_x[node] - h_CGPatch[3*group + 0]);
			ynode = (h_y[node] - h_CGPatch[3*group + 1]);
			znode = (h_z[node] - h_CGPatch[3*group + 2]);
			if (doesNodeExist (node)){
				*(outVector + 3*node) = inVector(6*group) + inVector(6*group + 4) * znode
					- inVector(6*group + 5) * ynode;
				*(outVector + 3*node + 1) = inVector(6*group + 1) - inVector(6*group + 3) * znode 
					+ inVector(6*group + 5) * xnode;
				*(outVector + 3*node + 2) = inVector(6*group + 2) + inVector(6*group + 3) * ynode
					- inVector(6*group + 4) * xnode;
			}
		}
	}
	else if (ws_deflationType == DEFLATION_TRANSLATION) {
		#pragma omp parallel for private(node,group)
		for (node = 0; node < h_nNodes; node++)	{
			if (!doesNodeExist(node)) continue;
			group = ws_nodeGroupNumber[node];
			if (doesNodeExist (node)){
				*(outVector + 3*node) = inVector(3*group);
				*(outVector + 3*node + 1) = inVector(3*group + 1);
				*(outVector + 3*node + 2) = inVector(3*group + 2);
			}
		}
	}
	else if (ws_deflationType == DEFLATION_BEAM) {
		#pragma omp parallel for private(node,xnode,ynode,znode,group, nu)
		for (node = 0; node < h_nNodes; node++) {
			if (!doesNodeExist(node)) continue;
			nu = 0.28;
			group = ws_nodeGroupNumber[node];
			xnode = (h_x[node] - h_CGPatch[3*group + 0]);
			ynode = (h_y[node] - h_CGPatch[3*group + 1]);
			znode = (h_z[node] - h_CGPatch[3*group + 2]);
			if (doesNodeExist (node)){
			outVector[3*node] = inVector[7*group] + inVector[7*group + 4] * znode - inVector[7*group + 5] * ynode
							- inVector[7*group + 6] * znode * xnode;

			outVector[3*node + 1] = inVector[7*group + 1] - inVector[7*group + 3] * znode + inVector[7*group + 5] * xnode;

			outVector[3*node + 2] = inVector[7*group + 2] + inVector[7*group + 3] * ynode - inVector[7*group + 4] * xnode
									+ inVector[7*group + 6] * xnode * xnode/2;
			}
		}
	}
	else if (ws_deflationType == DEFLATION_PLANE_STRESS){
		#pragma omp parallel for private(node,xnode,ynode,znode,group)
		for (node = 0; node < h_nNodes; node++)	{
			if (!doesNodeExist(node)) continue;
			group = ws_nodeGroupNumber[node];
			xnode = (h_x[node] - h_CGPatch[3*group + 0]);
			ynode = (h_y[node] - h_CGPatch[3*group + 1]);
			znode = (h_z[node] - h_CGPatch[3*group + 2]);
			if (doesNodeExist (node)){
				*(outVector + 3*node) = inVector(3*group) - inVector(3*group + 2) * ynode;
				*(outVector + 3*node + 1) = inVector(3*group + 1) + inVector(3*group + 2) * xnode;
			}
		}
	}
	else {
		#pragma omp parallel for private(node,xnode,ynode,znode,group)
		for (node = 0; node < h_nNodes; node++) {
			if (!doesNodeExist(node)) continue;
			group = ws_nodeGroupNumber[node];
			xnode = (h_x[node] - h_CGPatch[3*group + 0]);
			ynode = (h_y[node] - h_CGPatch[3*group + 1]);
			znode = (h_z[node] - h_CGPatch[3*group + 2]);
			if (doesNodeExist (node)){
			outVector[3*node] = inVector[9*group] + inVector[9*group + 4] * znode - inVector[9*group + 5] * ynode
							- inVector[9*group + 6] * znode * xnode - inVector[9*group + 8] * znode * ynode;

			outVector[3*node + 1] = inVector[9*group + 1] - inVector[9*group + 3] * znode + inVector[9*group + 5] * xnode
									- inVector[9*group + 7] * znode * ynode - inVector[9*group + 8] * znode * xnode;

			outVector[3*node + 2] = inVector[9*group + 2] + inVector[9*group + 3] * ynode - inVector[9*group + 4] * xnode
									+ inVector[9*group + 6] * xnode * xnode/2 + inVector[9*group + 7] * ynode * ynode/2 
									+ inVector[9*group + 8] * xnode * ynode;
			}
		}
	}
}
/**********************************************************************/
void AFElasticity::printKTilde(void)
{
	FILE *fptr;

	fptr = fopen(".//result.txt","w");
	double maxVal,a;
	maxVal = fabs((double)denseWKW(0,0));

	for (int i = 0; i < denseWKW.rows(); i++) {
		for (int j = 0; j < denseWKW.cols(); j++) {
			a = fabs((double)denseWKW(i,j));
			maxVal = (a > maxVal)? a:maxVal;
		}
	}
    fprintf(fptr,"MaxVal = %e\n",maxVal);
	for (int i = 0; i < denseWKW.rows(); i++) {
		for (int j = 0; j < denseWKW.cols(); j++) {
            fprintf(fptr,"%.4f\t",(double)  denseWKW(i,j)/maxVal);
		}
        fprintf(fptr,"\n");
	}
    fprintf(fptr,"-------------------------\n\n");
	fclose(fptr);
}
/**********************************************************************/
bool	AFElasticity::performCGW(int *niter )
{
	// Solves Ku = f
	// Assumes K and f have been defined, and Dirichlet conditions have been set
	// Also assumes CG iterations and tolerances have been defined.
	// Solution is returned in h_sol
	Timer timer,totalTimer;
	double fractionCompleted,tempDouble;
	
	
	timer.start();
	totalTimer.start();
	#ifdef COMPILE_GPUCODE
	if (usingGPU) {
		*niter = performCGWOnGPU();
		return true;
	}
	#endif

	double rho,alpha,beta,rho_new,normr,normr0;
	double *r,*p,*a,*d, *t;

    if (h_deflation_verbose)writeMessageToParetoLog("Deflatiom: Performing deflated CG ");

	// use temporary CPU variables
	r = (double *) malloc(3*h_nNodes*sizeof(double));
	p = (double *) malloc(3*h_nNodes*sizeof(double));
	a = (double *) malloc(3*h_nNodes*sizeof(double));
	d = (double *) malloc(3*h_nNodes*sizeof(double));
	t = (double *) malloc(3*h_nNodes*sizeof(double));

	double *temp_force = (double *) malloc(3*h_nNodes*sizeof(double));
	memcpy(temp_force,h_f,3*h_nNodes*sizeof(double));
	for (int node = 0; node < h_nNodes; node++) {
		if (!h_nodeExists[node]) {// make sure that there is no force on hanging nodes (body force case)
			temp_force[3*node] = 0;
			temp_force[3*node+1] = 0;
			temp_force[3*node+2] = 0;
		}
	}


	safeFree(h_cgNorm);
	h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
	
	VectorXd mu,result;
	if (ws_deflationType == DEFLATION_RIGID)	mu.resize(6*ws_nGroups);
	else if (ws_deflationType == DEFLATION_TRANSLATION) mu.resize(3*ws_nGroups);
	else if (ws_deflationType == DEFLATION_BEAM) mu.resize(7*ws_nGroups);
	else if (ws_deflationType == DEFLATION_PLANE_STRESS) mu.resize(3*ws_nGroups);
	else mu.resize(9*ws_nGroups);
	// Performing conjugate gradient here with residual start
	KuCPU(h_sol,a);// a = K*sol
	zeaxpy(-1,1,r,a,temp_force);// r = -a + f
	getWorkSpaceVector(r, mu); // mu = W'*h_f
	//Compute [W'*K*W]*result = W'*r = mu (from above)
	//result = denseWKW_LLT.solve(mu);
	result = sparseSolver.solve(mu);
	getSolutionSpaceVector(result, t);// h_sol = W*result


	enforceDirichlet0(t);
	axpy(1,t,h_sol);

	KuCPU(h_sol,a);// a = K*sol
	zeaxpy(-1,1,r,a,temp_force);// r = -a + f
	normr0 = dotprod(r,r);
	normr = 10*normr0;
	//********************************************************	
	KuCPU(r,t);
	getWorkSpaceVector(t, mu);	
	result = sparseSolver.solve(mu);
	getSolutionSpaceVector(result, t);
	
    //writeMessageToParetoLog("Residual after Deflation: %e ",Norm(t));
	
	enforceDirichlet0(t);
	zeaxpy(-1,1,p,t,r);// p = r - t
	//********************************************
	rho = dotprod(r,r);
	*niter = 1;     // Init counter for number of iterations

	fractionCompleted = 0;
	bool success = true;
	while (normr > h_CGRelativeResErr*normr0)   {
		tempDouble = ((h_CGRelativeResErr*normr0)/normr);
		fractionCompleted = MAX(0,1+0.1*log(tempDouble));// no negative values, at start, fractionCompleted =0 and then converges to 1
		h_cgNorm[*niter-1] = log10(normr/normr0);
		KuCPU(p,a);// a = K*p;
		beta = dotprod(a,p);
		alpha = rho/beta;
		axpy(alpha, p,h_sol);//u = u + alpha*p;
		if ((*niter)%500 ==0)	{ // restart CG with recomputing residual
            if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Restarting CG ");
			KuCPU(h_sol,a);// a = K*sol
			zeaxpy(-1,1,r,a,temp_force);// r = -a + f
		}
		else
			axpy(-alpha,a,r);//r = r - alpha*a;

		rho_new =  dotprod(r,r);//rho = a'*r
		normr = (rho_new);
		KuCPU(r,t);
		getWorkSpaceVector(t, mu);		// mu = W'*t
		result = sparseSolver.solve(mu);
		getSolutionSpaceVector(result, t); // t = W*result
		enforceDirichlet0(t);
		zeaxpy(1,(rho_new/rho),p,r,p);//p = a + rho_new/rho * p;
		zeaxpy(-1,1,p,t,p);//p = -t + p
		rho = rho_new;
		(*niter)++;
		h_nCGItersActual = *niter;

		if (timer.getElapsedTimeInSec() > 120) {//update every 2 mins or so.
            writeMessageToParetoLog("Deflation: Iter: %d	 Rel-err: %.2e ",*niter,normr/normr0);
			timer.stop();
			timer.start();
		}
		
        if ((h_deflation_verbose) && ((*niter)%50 ==0)) writeMessageToParetoLog("Deflation: Iter: %d	 Rel-err: %.2e ",*niter,normr/normr0);
		if (normr >1e3*normr0)  {  
			success = false;
			break;
		}

		if (*niter >= ws_DCGMaxIter)  { 
            if (h_deflation_verbose) writeMessageToParetoLog("Deflation: DCG did not converge in %d iter, resErr = %e ",*niter,normr/normr0);
			success = false; 
			break;
		}
	}
	memcpy(h_residual,r,3*h_nNodes*sizeof(double));//can be used for adaptive splitting ?
	
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Norm of solution is = %0.3e\n; relative res = %.3e",Norm(h_sol),normr/normr0);
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time taken for iterations = %0.3f (s)",totalTimer.getElapsedTimeInSec());
    if (h_deflation_verbose)  writeMessageToParetoLog("Deflation: CG on CPU; nIter = %d, resErr = %e ",*niter,normr/normr0);
    //writeMessageToParetoLog("\nDeflation: CG on CPU; nIter = %d, resErr = %e ",*niter,normr/normr0);
	computeDeformationMax();
	free(a);
	free(p);
	free(r);
	free(d);
	free(t); 
	free(temp_force);
	return success;
}

/**********************************************************************/
bool	AFElasticity::performPCGW(int *niter )
{
    // it can do both CGW as well as preconditioned CGW
	// Solves Ku = f
	// Assumes K and f have been defined, and Dirichlet conditions have been set
	// Also assumes CG iterations and tolerances have been defined.
	// Solution is returned in h_sol
	Timer timer,totalTimer;
	double fractionCompleted,tempDouble;
	
	timer.start();
	totalTimer.start();
	#ifdef COMPILE_GPUCODE
	if (usingGPU) {
		*niter = performCGWOnGPU();
		return true;
	}
	#endif

	double rho,alpha,beta,rho_new,normr,normr0;
	double *r,*p,*a, *t, *z,*preconditionerJacobi;	
	
    if (h_deflation_verbose)writeMessageToParetoLog("Deflation: Performing preconditioned deflated CG ");

	// use temporary CPU variables
	z = (double *) malloc(3*h_nNodes*sizeof(double)); 
	r = (double *) malloc(3*h_nNodes*sizeof(double));
	p = (double *) malloc(3*h_nNodes*sizeof(double));
	a = (double *) malloc(3*h_nNodes*sizeof(double));
	t = (double *) malloc(3*h_nNodes*sizeof(double));

	double *temp_force = (double *) malloc(3*h_nNodes*sizeof(double));
	memcpy(temp_force,h_f,3*h_nNodes*sizeof(double));
	for (int node = 0; node < h_nNodes; node++) {
		if (!h_nodeExists[node]) {// make sure that there is no force on hanging nodes (body force case)
			temp_force[3*node] = 0;
			temp_force[3*node+1] = 0;
			temp_force[3*node+2] = 0;
			continue;
		}	
	}

	safeFree(h_cgNorm);
	h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
	
	VectorXd mu,result;
	if (ws_deflationType == DEFLATION_RIGID)	mu.resize(6*ws_nGroups);
	else if (ws_deflationType == DEFLATION_TRANSLATION) mu.resize(3*ws_nGroups);
	else if (ws_deflationType == DEFLATION_BEAM) mu.resize(7*ws_nGroups);
	else if (ws_deflationType == DEFLATION_PLANE_STRESS) mu.resize(3*ws_nGroups);
	else mu.resize(9*ws_nGroups);
	// Performing conjugate gradient here with residual start
	KuCPU(h_sol,a);// a = K*sol
	zeaxpy(-1,1,r,a,temp_force);// r = -a + f  //r_-1
	
	getWorkSpaceVector(r, mu); // mu = W'*h_f   
	//Compute [W'*K*W]*result = W'*r = mu (from above)
	
	//result = denseWKW_LLT.solve(mu);
	result = sparseSolver.solve(mu);
    getSolutionSpaceVector(result, t);// h_sol = W*result  //tej: this is now the d0 = t
	enforceDirichlet0(t);
	axpy(1,t,h_sol);  //here h_sol is our x0
	KuCPU(h_sol,a);// a = K*sol
	zeaxpy(-1,1,r,a,temp_force);// r = -a + f  //r = r0
	if(h_usingPreconditioner)	{	
		//Jacobi Preconditioning
		preconditionerJacobi= (double*) malloc(3*h_nNodes*sizeof(double));	
        computeJacobiPreconditioner(preconditionerJacobi);
		ScaleByVector(z,r,preconditionerJacobi);	//here z is z0
	}
	else
		memcpy(z,r,3*h_nNodes*sizeof(double));
	

	normr0 = dotprod(z,r);
	normr = 10*normr0;
    //step3 of algo starts below
	//********************************************************	
	KuCPU(z,t);
	getWorkSpaceVector(t, mu);	
	//result = denseWKW_LLT.solve(mu);
	result = sparseSolver.solve(mu);
	getSolutionSpaceVector(result, t);  
	
	enforceDirichlet0(t);
	zeaxpy(-1,1,p,t,z);// p = r - t
    //tej: step 4 of the algo starts below
	//********************************************
	rho = dotprod(r,z);
	*niter = 1;     // Init counter for number of iterations

	fractionCompleted = 0;
	bool success = true;
    while (normr > h_CGRelativeResErr*normr0)   {
        tempDouble = (h_CGRelativeResErr*normr0/normr);
		fractionCompleted = MAX(0,1+0.1*log(tempDouble));// no negative values
		h_cgNorm[*niter-1] = log10(normr/normr0);
		KuCPU(p,a);// a = K*p;
		beta = dotprod(a,p);
		alpha = rho/beta;
		axpy(alpha, p,h_sol);//u = u + alpha*p;
		if ((*niter)%500 ==0)	{ // restart CG with recomputing residual
            if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Restarting CG ");
			KuCPU(h_sol,a);// a = K*sol
			zeaxpy(-1,1,r,a,temp_force);// r = -a + f
		}
		else
			axpy(-alpha,a,r);//r = r - alpha*a;
			
		if(h_usingPreconditioner)		
			ScaleByVector(z,r,preconditionerJacobi);	//here z is z0
		else
			memcpy(z,r,3*h_nNodes*sizeof(double));  //dividing by each component by 1

		rho_new =  dotprod(r,z);//rho = a'*r
		normr = (rho_new);
		KuCPU(z,t);
		getWorkSpaceVector(t, mu);		// mu = W'*t  
		//result = denseWKW_LLT.solve(mu);
		result = sparseSolver.solve(mu);
		getSolutionSpaceVector(result, t); // t = W*result  
		enforceDirichlet0(t);
		zeaxpy(1,(rho_new/rho),p,z,p);//p = a + rho_new/rho * p;
		zeaxpy(-1,1,p,t,p);//p = -t + p
		rho = rho_new;
		(*niter)++;
		h_nCGItersActual = *niter;

		if (timer.getElapsedTimeInSec() > 120) {//update every 2 mins or so.
            writeMessageToParetoLog("Deflation: Iter: %d	 Rel-err: %.2e ",*niter,normr/normr0);
			timer.stop();
			timer.start();
		}
		
		if ((h_deflation_verbose) && ((*niter)%100 ==0)) {
            writeMessageToParetoLog("Deflation: Iter: %d	 Rel-err: %.2e ",*niter,normr/normr0);
		}
		if (normr >1e3*normr0)  {  
            if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Diverging at %d iter, resErr = %e ",*niter,normr);
			success = false;
			break;
		}

		if (*niter >= ws_DCGMaxIter)  { 
            if (h_deflation_verbose) writeMessageToParetoLog("Deflation: DCG did not converge in %d iter, resErr = %e ",*niter,normr/normr0);
			success = false;
			break;
		}
	}
	memcpy(h_residual,r,3*h_nNodes*sizeof(double));//can be used for adaptive splitting ?
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Norm of solution is = %0.3e;",Norm(h_sol));
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Time taken for iterations = %0.3f (s)",totalTimer.getElapsedTimeInSec());
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: CG on CPU; nIter = %d, resErr = %e ",*niter,normr/normr0);
	computeDeformationMax();
	if(h_usingPreconditioner)	
		free(preconditionerJacobi);
	free(a);
	free(p);
	free(r);
	free(t); 
	free(z);
	free(temp_force);
	return success;
}

/**********************************************************************/
void	AFElasticity::computeJacobiPreconditioner(double *preconditionerJacobi)
{
    // function to compute the JacobiPreconditioner
	int node,codedElem,nodeIndex,elem,elemID,maxElem,node2,i,j,KEindex;  //additional variables for initializing preconditioning matrix
	double EScale,row,KE;
		
    if(h_deflation_verbose) writeMessageToParetoLog("Deflation: Computing Jacobi Preconditioner");

	memset(preconditionerJacobi,0,3*h_nNodes*sizeof(double));
	for ( node = 0; node < h_nNodes; node++) {
		if (!doesNodeExist(node)){
			for (j = 0; j < 3;j++)
				preconditionerJacobi[3*node+j] = 1;//diagonal of 1
			continue;
		}
		maxElem = h_nNeighboringElems[node];// how many elements connected to node
		for (j =0; j < maxElem; j++) {//
			codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
			nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
			elem = codedElem - nodeIndex*MAXELEMS;
			EScale = pow(h_pseudoDensity[elem],h_Eexponent);// E = E0*(rho)^n;
			elemID = h_templateID[elem];
			row = 24*3*nodeIndex;
			for (i = 0; i < 8; i++) {// find this node in the neighboring element
				node2 = h_elems[(h_nElems)*i + elem];
				if (node == node2) {
					KEindex = (row + 3*i)*(h_nTemplateElements) + elemID;
					KE = EScale*h_KETemplates[KEindex];
					preconditionerJacobi[3*node] += KE;

					KEindex = (row + 24+3*i+1)*(h_nTemplateElements) + elemID;
					KE = EScale*h_KETemplates[KEindex];
					preconditionerJacobi[3*node+1] += KE;

					KEindex = (row + 48 + 3*i+2)*(h_nTemplateElements) + elemID;
					KE = EScale*h_KETemplates[KEindex];
					preconditionerJacobi[3*node+2] += KE;	
				}
			}
		}	
	}
}
/**********************************************************************/
void AFElasticity::ReleaseWorkSpace()
{

	if (!h_workSpaceInitialized) return;
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Releasing Workspace");
	safeFree(ws_nodeGroupNumber); ws_nodeGroupNumber= NULL;
	safeFree(ws_groupSortedNodes); ws_groupSortedNodes= NULL;
	safeFree(ws_groupEndNodeIndex); ws_groupEndNodeIndex= NULL;
	safeFree(ws_groupCount);ws_groupCount = NULL;
	safeFree(ws_anyNodeInGroup);ws_anyNodeInGroup = NULL;

	
	#ifdef COMPILE_GPUCODE
	if (WorkSpaceTransferredToGPU) {
		freeGPUWorkSpaceData(&gpuWS);
		WorkSpaceTransferredToGPU = 0;
	}
	#endif
	h_workSpaceInitialized = false;
}

#ifdef COMPILE_GPUCODE
/**********************************************************************/
void	AFElasticity::transferWorkSpaceToGPU(void)
{// just a wrapper

	// Transfer WorkSpace Data to GPU
	if (WorkSpaceTransferredToGPU == 0) {
        if (h_deflation_verbose)  writeMessageToParetoLog("Deflation: Initializing workspace on GPU ");
		InitializeDeflationForElasticityOnGPU(h_nNodes, ws_nGroups, ws_deflationType, ws_nodeGroupNumber, ws_groupSortedNodes,
					ws_groupEndNodeIndex, h_x, h_y, h_z, h_CGPatch, sparseWKW_Lower.data(),sparsePermuteVector.data(), &gpuWS);
		WorkSpaceTransferredToGPU = 1;
	}
}
/**********************************************************************/
int	AFElasticity::performCGWOnGPU(void)
{
	// Assumes that
	// (1) forces are initialized in h_gpuData->d_fu, ...
	// (2) Templates are computed and d_KETemplates stored in h_gpuData
	// The solution is stored in
	// h_gpuData->d_u, ...

	// If converged, returns the number of CG iterations
	// If divergence, returns -1
	int nIter;
	
	//transferAFElasticityToGPU();
	if (WorkSpaceTransferredToGPU == 0) {
		transferWorkSpaceToGPU();
		WorkSpaceTransferredToGPU = 1;
	}
    if (h_deflation_verbose) writeMessageToParetoLog("Deflation: Performing Deflated CG on GPU");
	nIter = WorkSpaceCGOnGPU(&h_gpuData, &gpuWS, ws_DCGMaxIter,h_CGRelativeResErr,h_nodeBasedAssembly);
	transferSolutionFromGPU(&h_gpuData,h_sol);
	computeDeformationMax();
	return nIter;
}
#endif
