#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string> 
#include <fstream> 
#include <iostream> 
#ifndef __APPLE__
#include <omp.h>
#endif
#include <algorithm>    // std::sort
#include "cmatrix.h"
#include "TopOpt.h"
#include "Timer.h"
#include <IntegerSet.h>


using namespace std; 
typedef unsigned char byte; 

#pragma warning(disable:4996)
#pragma warning( disable : 4101 )//disable warning with openmp


/**********************************************************************/
TopOpt::TopOpt()
{

	ts_verbose = false;
	ts_pNormIndex = 6;// for pNorm vonmises stress approximation
	ts_maxAttempts = 5;
	ts_topSensElem = NULL;
    ts_topSensElemUnscaled = NULL;
	ts_topSensNode = NULL;
	for (int i = 0; i < MAX_ISO_SAVES; i++){
		ts_topSensNodeSave[i] = NULL;
		ts_elemExists[i] = NULL;
	}
	ts_currentIsoCounter =-1;//
    ts_saveEveryNIterations = 2;// save isosurface every such N iterations
	
    ts_min_topoptIterations = 1;
	ts_max_topoptIterations = 4;
	ts_stressAdjoint = NULL;
	ts_stressAdjointForce = NULL;
	ts_compliance0 = 0;
	ts_displacement0 = 0;
	ts_vonMisesStress0 = 0;
	ts_outputcompliance0 = 0;
	ts_compliance = 0;
	ts_outputcompliance = 0;
	ts_inputDisplacement = 0;
	ts_outputDisplacement = 0;

    ts_includeThermalEffects = false;
	TSComplianceNorm0 = 1.0;
	TSOutputComplianceNorm0 = 1.0;
	TSMutualComplianceNorm0 = 1.0;
	TSStressNorm0 = 1.0;
	TSBucklingNorm0 = 1.0;
    ts_isTSMinNegative = false;


	ts_minAllowableVolumeFraction = 0.0099;
	ts_maxDisplacementAllowed = HUGE_VAL;

    ts_maxFeatureSize = -1;

	// saved data initialization
	ts_backupData.sol = NULL;
	ts_backupData.pseudoDensity = NULL;
	ts_backupData.nodeExists = NULL;
	ts_backupData.topSensElem = NULL;
	ts_backupData.topSensNode = NULL;
	ts_backupData.neighborNodes = NULL;
	ts_backupData.nodeSignature = NULL;

	ts_topoptQuantity = COMPLIANCE;//default
	ts_topoptMode = TOPOPT_MODE_ELASTIC;// default
    ts_nSmoothingIterations = 1;

    ts_relativeJIncrementForStepping = 0.05;// for adaptive stepping
	ts_ensureIsoSurfaceConnectivity = false;// if set to true, use the mesh pseudo densiuty to create isosurface (coarser), else use the TS field (smoother)

	ts_useEigenConstraint = false;
	ts_minEigenValueHz = 1000;//Hz
    ts_useTemperatureConstraint = false;
    ts_maxTemperatureLimit = 1000; // typically Kelvin
	ts_useAllLoads = false;
    ts_keepTopologyConnected = true;
    strcpy(ts_lastTopOptErrorMessage,"No TopOpt error");

}
/**********************************************************************/
void TopOpt::enableTSVerbose(void)
{
    writeMessageToParetoLog("TopOpt: Enabling TopOpt Verbose");
	ts_verbose = true;
}
/**********************************************************************/
void TopOpt::disableTSVerbose(void)
{
    writeMessageToParetoLog("TopOpt: Disabling TopOpt Verbose");
	ts_verbose = false;
}
/**********************************************************************/
void TopOpt::getLastTopoptErrorMessage(char msg[])
{
	strcpy(msg,ts_lastTopOptErrorMessage);
}
/**********************************************************************/
void TopOpt::setMinAllowableVolFraction(double volFrac)
{
	ts_minAllowableVolumeFraction = volFrac;
}
/**********************************************************************/
void TopOpt::setThermalEffects(bool thermalEffects)
{
    ts_includeThermalEffects = thermalEffects;
}
/**********************************************************************/
double TopOpt::getMinAllowableVolFraction(void)
{
	return ts_minAllowableVolumeFraction;
}
/**********************************************************************/
void TopOpt::setIterationCountForSaving(int iter)
{
	ts_saveEveryNIterations = iter;
}
/**********************************************************************/
void TopOpt::setQuantityOfInterest(TOPOPT_QUANTITY quantity)
{
	ts_topoptQuantity = quantity;
	if (ts_topoptQuantity == VONMISES) 
		ts_complianceWt = 0;
}
/**********************************************************************/
void TopOpt::setMaxTemperatureLimit( double maxTemperatureAllowed)
{
    if (maxTemperatureAllowed > 0)
        ts_useTemperatureConstraint = true;
    else
        ts_useTemperatureConstraint = false;

    ts_maxTemperatureLimit = maxTemperatureAllowed;
}
/**********************************************************************/
void TopOpt::setEigenConstraint( double minEigenValueHz)
{
    if (minEigenValueHz > 0)
        ts_useEigenConstraint = true;
    else
        ts_useEigenConstraint = false;

	ts_minEigenValueHz = minEigenValueHz;
}
/**********************************************************************/
bool TopOpt::isEigenConstraintActive(void)
{
	return ts_useEigenConstraint;
}
/**********************************************************************/
void TopOpt::setMaxAttempts(int attempts)
{
	ts_maxAttempts = attempts;
}
/**********************************************************************/
void TopOpt::setDrawDirection(int drawDirection)
{
    if ((drawDirection < 0) || (drawDirection > 3)) return;
	ts_drawDirection = drawDirection;
}
/**********************************************************************/
void TopOpt::setMinFeatureSize(double minFeatureSize)
{
    if (minFeatureSize < 0)  return;
    ts_minFeatureSize = minFeatureSize;
}
/**********************************************************************/
void TopOpt::setMaxFeatureSize(double maxFeatureSize)
{
    if (maxFeatureSize < 0)  return;
    ts_maxFeatureSize = maxFeatureSize;
}
/**********************************************************************/
void TopOpt::setExtrudeDirection(int extrudeDirection)
{
    if ((extrudeDirection < 0) || (extrudeDirection > 3)) return;
    ts_extrudeDirection = extrudeDirection;
}
/**********************************************************************/
void TopOpt::setXGridPattern(int xGrid)
{
   if (xGrid < 2) return;
   ts_gridPattern[0] = xGrid;
}
/**********************************************************************/
void TopOpt::setYGridPattern(int yGrid)
{
    if (yGrid < 2) return;
   ts_gridPattern[1] = yGrid;
}
/**********************************************************************/
void TopOpt::setZGridPattern(int zGrid)
{
    if (zGrid < 2) return;
   ts_gridPattern[2] = zGrid;
}
/**********************************************************************/
void TopOpt::setSymmetryIndex(int symmetryIndex)
{
    if ((symmetryIndex < 0) || (symmetryIndex > 7)) return;
	ts_symmetryIndex = symmetryIndex;
}
/**********************************************************************/
void TopOpt::setStressSafetyFactor(double safetyFactor)
{
	ts_stressSafetyFactor = safetyFactor;
}
/**********************************************************************/
void TopOpt::setTopoptMode(TOPOPT_MODE mode)
{
	ts_topoptMode = mode;
}
/**********************************************************************/
void TopOpt::setAxiSymmetry(int axiSymmetry)
{
	ts_axiSymmetryIndex = axiSymmetry;
}
/**********************************************************************/
void TopOpt::setKeepTopologyConnected(bool connected)
{
    ts_keepTopologyConnected = connected;
}
/**********************************************************************/
int TopOpt::InitializeTopOpt(int BCmode)
// BCmode = 0 implies no additional constraint (rarely used, unless body force exists)
// BCmode = 1 implies all elements next to Neumann BCs are retained during optimization, but not Dirichlet
// BCmode = 2 implies all elements next to Dirichlet BCs are retained during optimization, but not Neumann (NEVER USED in practice)
// BCmode = 3 implies all elements next to Neumann and Dirichlet BCs are retained during optimization
{
	int j,elem,node,maxElem,codedElem,nodeIndex,nModes,partId;

    if (ts_verbose) writeMessageToParetoLog("TopOpt: Initializing Topopt");

    strcpy(ts_lastTopOptErrorMessage,"No TopOpt error");
	initializeSolution();
	ts_maxAttempts = 5;

	ts_maxDisplacementAllowed = HUGE_VAL;
	
	ts_relativeJTolerance = 0.015;//default
	ts_relativeJConvergence = 100.0;// default
	computeFMatrix();// needed for p-norm stress

	ts_useAnalyticalTS = true;
	ts_restartTopology = true;// 'true' means elements may reappear
	ts_scaleTSField = true;// 'true' means scale between 0 and 1
	ts_complianceWt = 1.00;//  not used for compliance

    safeFree(ts_topSensElem);
    safeFree(ts_topSensElemUnscaled);
    safeFree(ts_topSensNode);

	for (int i = 0; i < MAX_ISO_SAVES; i++){
		safeFree(ts_topSensNodeSave[i]);
		safeFree(ts_elemExists[i]);
	}

	ts_topSensElem = (double *)malloc(h_nElems*sizeof(double));
    ts_topSensElemUnscaled = (double *)malloc(h_nElems*sizeof(double));
	ts_topSensNode = (double *)malloc(h_nNodes*sizeof(double));
	memset(ts_topSensElem,0,h_nElems*sizeof(double));
    memset(ts_topSensElemUnscaled,0,h_nElems*sizeof(double));
	memset(ts_topSensNode,0,h_nNodes*sizeof(double));

	for (int i = 0; i < MAX_ISO_SAVES; i++){
		ts_topSensNodeSave[i] = (double *)malloc(h_nNodes*sizeof(double));
		memset(ts_topSensNodeSave[i],0,h_nNodes*sizeof(double));

		ts_elemExists[i] = (bool *)malloc(h_nElems*sizeof(bool));
		memset(ts_elemExists[i],true,h_nElems*sizeof(bool));

		ts_maxDisplacementSave[i] = 0;
        for (int loadSet = 0; loadSet < MAX_BC_SET; loadSet++){
            ts_complianceSave[loadSet][i] = 0;
            ts_maxTemperatureSave[loadSet][i] = 0;
            for (int part  = 0; part < getNumberOfParts(); part++)
                ts_maxStressSave[loadSet][i][part] = 0;
        }

		ts_firstModeSave[i] = 0;

	}
     if (ts_verbose) writeMessageToParetoLog("TopOpt: Allocated Topopt memory");

	ts_volFracSave[0] = 1.0;
	ts_currentIsoCounter = -1;//

	safeFree(ts_stressAdjoint);ts_stressAdjoint = NULL;
	safeFree(ts_stressAdjointForce);ts_stressAdjointForce = NULL;
 
	
	ts_volErrorFraction = MAX(0.001,10.0/h_nElems); // acceptable error in volume fraction
	
	ts_stressAdjoint = (double *)malloc(3*h_nNodes*sizeof(double));
	ts_stressAdjointForce = (double *)malloc(3*h_nNodes*sizeof(double));
	memset(ts_stressAdjoint,0,3*h_nNodes*sizeof(double));
	memset(ts_stressAdjointForce,0,3*h_nNodes*sizeof(double));

	
	initializets_backupData();
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Initialized backup ");

	computeGrad_H8(0,0,0,gradNxiCenter,gradNetaCenter,gradNmuCenter); //compute for later on

    resetTopOptConstraints();

	nModes = 1;
	// Force all elements next to 'retain' nodes to keep
	for (node = 0; node < h_nNodes; node++) {
		if (h_retain[node]) {
			maxElem = h_nNeighboringElems[node];// how many elements connected to node
			for (j =0; j < maxElem; j++) {//
				codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
				nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
				elem = codedElem - nodeIndex*MAXELEMS;
				h_keepElem[elem] = true;
			}
		}
	}

	// if a part is not to be optimized ... keep the corresponding elements
	for (elem = 0; elem < h_nElems; elem++) {
		partId = getPartIdOfElem(elem);
		if (!shouldPartBeOptimized(partId)){
			h_keepElem[elem] = true;
		}
	}

	if (BCmode == 0)
		BCmode = 0;//nothing to do
	if ((BCmode == 1) || (BCmode == 3)) {
		// force all elements next to Neumann BC and output (for complaint mechanism) to keep
		for (node = 0; node < h_nNodes; node++) {
			//if (!isNodeOnBoundary(node)) continue;// only keep elements on boundary 
			if ( isNodeRetained(node) ){// retain applies also to nodes with forces
				maxElem = h_nNeighboringElems[node];// how many elements connected to node
				for (j =0; j < maxElem; j++) {//
					codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
					nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
					elem = codedElem - nodeIndex*MAXELEMS;
					h_keepElem[elem] = true;

				}
			}
		}
	}

	if ((BCmode == 2)|| (BCmode == 3)) {
		//  force all elements next to Dirichlet BC to keep
		for (node = 0; node < h_nNodes; node++) {
			if (!isNodeOnBoundary(node)) continue;// only worry about boundary nodes
			if (h_free[node] <7) {
				maxElem = h_nNeighboringElems[node];// how many elements connected to node
				for (j =0; j < maxElem; j++) {//
					codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
					nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
					elem = codedElem - nodeIndex*MAXELEMS;
					h_keepElem[elem] = true;

				}
			}
		}
	}

	int nElemsToKeep = 0;
	for (int elem = 0; elem < h_nElems; elem++)
		if (h_keepElem[elem])
			nElemsToKeep++;

    if (ts_verbose) writeMessageToParetoLog("TopOpt: TS smoothening iterations = %d ",ts_nSmoothingIterations);
	return SUCCESS;
}
/**********************************************************************/
void TopOpt:: resetTopOptConstraints(void)
{
    ts_extrudeDirection = -1;
    ts_drawDirection = -1;
    ts_axiSymmetryIndex = -1;
    ts_symmetryIndex = -1;
    ts_gridPattern[0] = -1;
    ts_gridPattern[1] = -1;
    ts_gridPattern[2] = -1;
    ts_minFeatureSize = -1;
    ts_maxFeatureSize = -1;
    ts_maxDisplacementAllowed = HUGE_VAL;
    ts_stressSafetyFactor = 1.0;
    ts_minEigenValueHz = -1;
    ts_useEigenConstraint = false;
    ts_AMBuildDirection = 0;
}
/**********************************************************************/
void TopOpt:: setTSScaleToUnity(bool scaleToUnity)
{
	ts_scaleTSField = scaleToUnity;
}
/**********************************************************************/
bool TopOpt::getKeepElem(int elem)
{
	return h_keepElem[elem];
}
/**********************************************************************/
void TopOpt::setTStoAnalytical(bool trueOrFalse)
{
	ts_useAnalyticalTS = trueOrFalse;
}
/**********************************************************************/
void TopOpt::setMaxDisplacement(double disp)
{
	ts_maxDisplacementAllowed = disp;
}
/**********************************************************************/
void TopOpt::initializets_backupData(void)
{
	// saved data initialization
	safeFree(ts_backupData.sol);
	safeFree(ts_backupData.pseudoDensity);
	safeFree(ts_backupData.nodeExists);
	safeFree(ts_backupData.topSensElem);
	safeFree(ts_backupData.topSensNode);
	safeFree(ts_backupData.neighborNodes);
	safeFree(ts_backupData.nodeSignature);

	ts_backupData.sol = (double *)malloc(3*h_nNodes*sizeof(double));
	ts_backupData.pseudoDensity = (double *)malloc(h_nElems*sizeof(double));
	ts_backupData.nodeExists = (bool *)malloc(h_nNodes*sizeof(bool));
	ts_backupData.topSensElem = (double *)malloc(h_nElems*sizeof(double));
	ts_backupData.topSensNode = (double *)malloc(h_nNodes*sizeof(double));
	ts_backupData.neighborNodes = (int *)malloc(27*h_nNodes*sizeof(int));
	ts_backupData.nodeSignature = (int *)malloc(h_nNodes*sizeof(int));

    if (ts_verbose) writeMessageToParetoLog("Topopt: Initialized saved data ");
}
/**********************************************************************/
void TopOpt::pushToSavedData(void)
{
	if (ts_verbose) cout << "Topopt: Saving back up for recovery " << endl;


	getSolutionVector(ts_backupData.sol);
	getPseudoDensity(ts_backupData.pseudoDensity);
	getNodeExists(ts_backupData.nodeExists);

	memcpy(ts_backupData.topSensElem,ts_topSensElem,h_nElems*sizeof(double));
	memcpy(ts_backupData.topSensNode,ts_topSensNode,h_nNodes*sizeof(double));

	if (isSpMVMethodNodeBased()) {
		getNeighborNodes(ts_backupData.neighborNodes);
		getNodeSignatures(ts_backupData.nodeSignature);
	}

	// there is only one load here
	for (int part = 0; part < getNumberOfParts(); part++)
		ts_backupData.maxStress[0][part] = getMaxStressInPart(part);
	ts_backupData.maxDisp[0] = getDeformationMax();
    ts_backupData.firstEigenMode = getFirstEigenValue();
}
/**********************************************************************/
void TopOpt::retrieveFromSavedData(void)
{
	setSolutionAtAllNodes(ts_backupData.sol);
	setPseudoDensity(ts_backupData.pseudoDensity);
	setNodeExists(ts_backupData.nodeExists);

	memcpy(ts_topSensElem,ts_backupData.topSensElem,h_nElems*sizeof(double));
	memcpy(ts_topSensNode,ts_backupData.topSensNode,h_nNodes*sizeof(double));

    // copy the unscaled values for adaptive stepping
    memcpy(ts_topSensElemUnscaled,ts_topSensElem,h_nElems*sizeof(double));
	if (isSpMVMethodNodeBased()) {
		setNeighborNodes(ts_backupData.neighborNodes);
		setNodeSignatures(ts_backupData.nodeSignature);
	}
    if (ts_verbose) writeMessageToParetoLog("Topopt: Retrieved saved data ");
}
/**********************************************************************/
void TopOpt::setNumSmoothingOperations(int nSmoothing)
{
	ts_nSmoothingIterations = MAX(1,nSmoothing);
} 
/**********************************************************************/
void TopOpt::getMaxMinTopSens(double *TSMax,double *TSMin)
{
	*TSMax = ts_TSMax;
	*TSMin = ts_TSMin;
}
/**********************************************************************/
void TopOpt::getAdjoint(double *adjoint)
{
	memcpy(adjoint,ts_stressAdjoint,3*h_nNodes*sizeof(double));
}
/**********************************************************************/
void TopOpt::setAdjoint(double *adjoint)
{
	memcpy(ts_stressAdjoint,adjoint,3*h_nNodes*sizeof(double));
}
/**********************************************************************/
void TopOpt::getElemAdjoint(int elem,double adjoint[24])
{
	int node;
	for (int m = 0; m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		for (int i = 0; i < 3; i++)
			adjoint[3*m+i] = ts_stressAdjoint[3*node+i];
	}
}
/**********************************************************************/
void TopOpt::getElemTopOptSens(int elem,double *ts)
{
	*ts = ts_topSensElem[elem];
}
/**********************************************************************/
void TopOpt::getElemNodesTopOptSens(int elem,double ts[8])
{
	int node;
	for (int m = 0;m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		ts[m] = ts_topSensNode[node];
	}
}
/**********************************************************************/
bool TopOpt::doesTSExist(void)
{
	if (ts_topSensNode == NULL)
		return false;
	else
		return true;
}
/**********************************************************************/
void  TopOpt::computeFMatrix(void)
{
	double xi,eta,mu;
	double gradNxi[8],gradNeta[8],gradNmu[8];
	double dx,dy,dz;
	double d[36];
	xi = 0; eta = 0; mu = 0; // evaluate at center

	dx = h_dx;
	dy = h_dy;
	dz = h_dz;
	computeGrad_H8(xi,eta,mu,gradNxi,gradNeta,gradNmu);
	for (int materialID = 0;materialID<getNumMaterials();materialID++){
		for (int i=0;i<6;i++){
			for (int j=0;j<24;j++)
				Fe[materialID][i][j] = 0.;
		}
	}

	for (int materialID = 0;materialID<getNumMaterials();materialID++){
		getDMatrix(materialID,d);

		Fe[materialID][0][0] = (d[0]*gradNxi[0]*2.0)/dx;
		Fe[materialID][0][1] = (d[1]*gradNeta[0]*2.0)/dy;
		Fe[materialID][0][2] = (d[2]*gradNmu[0]*2.0)/dz;
		Fe[materialID][0][3] = (d[0]*gradNxi[1]*2.0)/dx;
		Fe[materialID][0][4] = (d[1]*gradNeta[1]*2.0)/dy;
		Fe[materialID][0][5] = (d[2]*gradNmu[1]*2.0)/dz;
		Fe[materialID][0][6] = (d[0]*gradNxi[2]*2.0)/dx;
		Fe[materialID][0][7] = (d[1]*gradNeta[2]*2.0)/dy;
		Fe[materialID][0][8] = (d[2]*gradNmu[2]*2.0)/dz;
		Fe[materialID][0][9] = (d[0]*gradNxi[3]*2.0)/dx;
		Fe[materialID][0][10] = (d[1]*gradNeta[3]*2.0)/dy;
		Fe[materialID][0][11] = (d[2]*gradNmu[3]*2.0)/dz;
		Fe[materialID][0][12] = (d[0]*gradNxi[4]*2.0)/dx;
		Fe[materialID][0][13] = (d[1]*gradNeta[4]*2.0)/dy;
		Fe[materialID][0][14] = (d[2]*gradNmu[4]*2.0)/dz;
		Fe[materialID][0][15] = (d[0]*gradNxi[5]*2.0)/dx;
		Fe[materialID][0][16] = (d[1]*gradNeta[5]*2.0)/dy;
		Fe[materialID][0][17] = (d[2]*gradNmu[5]*2.0)/dz;
		Fe[materialID][0][18] = (d[0]*gradNxi[6]*2.0)/dx;
		Fe[materialID][0][19] = (d[1]*gradNeta[6]*2.0)/dy;
		Fe[materialID][0][20] = (d[2]*gradNmu[6]*2.0)/dz;
		Fe[materialID][0][21] = (d[0]*gradNxi[7]*2.0)/dx;
		Fe[materialID][0][22] = (d[1]*gradNeta[7]*2.0)/dy;
		Fe[materialID][0][23] = (d[2]*gradNmu[7]*2.0)/dz;
		Fe[materialID][1][0] = (d[6]*gradNxi[0]*2.0)/dx;
		Fe[materialID][1][1] = (d[7]*gradNeta[0]*2.0)/dy;
		Fe[materialID][1][2] = (d[8]*gradNmu[0]*2.0)/dz;
		Fe[materialID][1][3] = (d[6]*gradNxi[1]*2.0)/dx;
		Fe[materialID][1][4] = (d[7]*gradNeta[1]*2.0)/dy;
		Fe[materialID][1][5] = (d[8]*gradNmu[1]*2.0)/dz;
		Fe[materialID][1][6] = (d[6]*gradNxi[2]*2.0)/dx;
		Fe[materialID][1][7] = (d[7]*gradNeta[2]*2.0)/dy;
		Fe[materialID][1][8] = (d[8]*gradNmu[2]*2.0)/dz;
		Fe[materialID][1][9] = (d[6]*gradNxi[3]*2.0)/dx;
		Fe[materialID][1][10] = (d[7]*gradNeta[3]*2.0)/dy;
		Fe[materialID][1][11] = (d[8]*gradNmu[3]*2.0)/dz;
		Fe[materialID][1][12] = (d[6]*gradNxi[4]*2.0)/dx;
		Fe[materialID][1][13] = (d[7]*gradNeta[4]*2.0)/dy;
		Fe[materialID][1][14] = (d[8]*gradNmu[4]*2.0)/dz;
		Fe[materialID][1][15] = (d[6]*gradNxi[5]*2.0)/dx;
		Fe[materialID][1][16] = (d[7]*gradNeta[5]*2.0)/dy;
		Fe[materialID][1][17] = (d[8]*gradNmu[5]*2.0)/dz;
		Fe[materialID][1][18] = (d[6]*gradNxi[6]*2.0)/dx;
		Fe[materialID][1][19] = (d[7]*gradNeta[6]*2.0)/dy;
		Fe[materialID][1][20] = (d[8]*gradNmu[6]*2.0)/dz;
		Fe[materialID][1][21] = (d[6]*gradNxi[7]*2.0)/dx;
		Fe[materialID][1][22] = (d[7]*gradNeta[7]*2.0)/dy;
		Fe[materialID][1][23] = (d[8]*gradNmu[7]*2.0)/dz;
		Fe[materialID][2][0] = (d[12]*gradNxi[0]*2.0)/dx;
		Fe[materialID][2][1] = (d[13]*gradNeta[0]*2.0)/dy;
		Fe[materialID][2][2] = (d[14]*gradNmu[0]*2.0)/dz;
		Fe[materialID][2][3] = (d[12]*gradNxi[1]*2.0)/dx;
		Fe[materialID][2][4] = (d[13]*gradNeta[1]*2.0)/dy;
		Fe[materialID][2][5] = (d[14]*gradNmu[1]*2.0)/dz;
		Fe[materialID][2][6] = (d[12]*gradNxi[2]*2.0)/dx;
		Fe[materialID][2][7] = (d[13]*gradNeta[2]*2.0)/dy;
		Fe[materialID][2][8] = (d[14]*gradNmu[2]*2.0)/dz;
		Fe[materialID][2][9] = (d[12]*gradNxi[3]*2.0)/dx;
		Fe[materialID][2][10] = (d[13]*gradNeta[3]*2.0)/dy;
		Fe[materialID][2][11] = (d[14]*gradNmu[3]*2.0)/dz;
		Fe[materialID][2][12] = (d[12]*gradNxi[4]*2.0)/dx;
		Fe[materialID][2][13] = (d[13]*gradNeta[4]*2.0)/dy;
		Fe[materialID][2][14] = (d[14]*gradNmu[4]*2.0)/dz;
		Fe[materialID][2][15] = (d[12]*gradNxi[5]*2.0)/dx;
		Fe[materialID][2][16] = (d[13]*gradNeta[5]*2.0)/dy;
		Fe[materialID][2][17] = (d[14]*gradNmu[5]*2.0)/dz;
		Fe[materialID][2][18] = (d[12]*gradNxi[6]*2.0)/dx;
		Fe[materialID][2][19] = (d[13]*gradNeta[6]*2.0)/dy;
		Fe[materialID][2][20] = (d[14]*gradNmu[6]*2.0)/dz;
		Fe[materialID][2][21] = (d[12]*gradNxi[7]*2.0)/dx;
		Fe[materialID][2][22] = (d[13]*gradNeta[7]*2.0)/dy;
		Fe[materialID][2][23] = (d[14]*gradNmu[7]*2.0)/dz;
		Fe[materialID][3][0] = (d[21]*gradNeta[0]*2.0)/dy;
		Fe[materialID][3][1] = (d[21]*gradNxi[0]*2.0)/dx;
		Fe[materialID][3][3] = (d[21]*gradNeta[1]*2.0)/dy;
		Fe[materialID][3][4] = (d[21]*gradNxi[1]*2.0)/dx;
		Fe[materialID][3][6] = (d[21]*gradNeta[2]*2.0)/dy;
		Fe[materialID][3][7] = (d[21]*gradNxi[2]*2.0)/dx;
		Fe[materialID][3][9] = (d[21]*gradNeta[3]*2.0)/dy;
		Fe[materialID][3][10] = (d[21]*gradNxi[3]*2.0)/dx;
		Fe[materialID][3][12] = (d[21]*gradNeta[4]*2.0)/dy;
		Fe[materialID][3][13] = (d[21]*gradNxi[4]*2.0)/dx;
		Fe[materialID][3][15] = (d[21]*gradNeta[5]*2.0)/dy;
		Fe[materialID][3][16] = (d[21]*gradNxi[5]*2.0)/dx;
		Fe[materialID][3][18] = (d[21]*gradNeta[6]*2.0)/dy;
		Fe[materialID][3][19] = (d[21]*gradNxi[6]*2.0)/dx;
		Fe[materialID][3][21] = (d[21]*gradNeta[7]*2.0)/dy;
		Fe[materialID][3][22] = (d[21]*gradNxi[7]*2.0)/dx;
		Fe[materialID][4][0] = (d[28]*gradNmu[0]*2.0)/dz;
		Fe[materialID][4][2] = (d[28]*gradNxi[0]*2.0)/dx;
		Fe[materialID][4][3] = (d[28]*gradNmu[1]*2.0)/dz;
		Fe[materialID][4][5] = (d[28]*gradNxi[1]*2.0)/dx;
		Fe[materialID][4][6] = (d[28]*gradNmu[2]*2.0)/dz;
		Fe[materialID][4][8] = (d[28]*gradNxi[2]*2.0)/dx;
		Fe[materialID][4][9] = (d[28]*gradNmu[3]*2.0)/dz;
		Fe[materialID][4][11] = (d[28]*gradNxi[3]*2.0)/dx;
		Fe[materialID][4][12] = (d[28]*gradNmu[4]*2.0)/dz;
		Fe[materialID][4][14] = (d[28]*gradNxi[4]*2.0)/dx;
		Fe[materialID][4][15] = (d[28]*gradNmu[5]*2.0)/dz;
		Fe[materialID][4][17] = (d[28]*gradNxi[5]*2.0)/dx;
		Fe[materialID][4][18] = (d[28]*gradNmu[6]*2.0)/dz;
		Fe[materialID][4][20] = (d[28]*gradNxi[6]*2.0)/dx;
		Fe[materialID][4][21] = (d[28]*gradNmu[7]*2.0)/dz;
		Fe[materialID][4][23] = (d[28]*gradNxi[7]*2.0)/dx;
		Fe[materialID][5][1] = (d[35]*gradNmu[0]*2.0)/dz;
		Fe[materialID][5][2] = (d[35]*gradNeta[0]*2.0)/dy;
		Fe[materialID][5][4] = (d[35]*gradNmu[1]*2.0)/dz;
		Fe[materialID][5][5] = (d[35]*gradNeta[1]*2.0)/dy;
		Fe[materialID][5][7] = (d[35]*gradNmu[2]*2.0)/dz;
		Fe[materialID][5][8] = (d[35]*gradNeta[2]*2.0)/dy;
		Fe[materialID][5][10] = (d[35]*gradNmu[3]*2.0)/dz;
		Fe[materialID][5][11] = (d[35]*gradNeta[3]*2.0)/dy;
		Fe[materialID][5][13] = (d[35]*gradNmu[4]*2.0)/dz;
		Fe[materialID][5][14] = (d[35]*gradNeta[4]*2.0)/dy;
		Fe[materialID][5][16] = (d[35]*gradNmu[5]*2.0)/dz;
		Fe[materialID][5][17] = (d[35]*gradNeta[5]*2.0)/dy;
		Fe[materialID][5][19] = (d[35]*gradNmu[6]*2.0)/dz;
		Fe[materialID][5][20] = (d[35]*gradNeta[6]*2.0)/dy;
		Fe[materialID][5][22] = (d[35]*gradNmu[7]*2.0)/dz;
		Fe[materialID][5][23] = (d[35]*gradNeta[7]*2.0)/dy;
	}
}

/**********************************************************************/
void TopOpt::getFMatrix(double FMatrix[10][6][24])
{
	for (int matl=0;matl<10;matl++)
		for (int i=0;i<6;i++)
			for (int j=0;j<24;j++)
				FMatrix[matl][i][j] = Fe[matl][i][j];
}
/**********************************************************************/
void TopOpt::getStressAdjointSolution(int elem, double adjointSol[24])
{
	int node;
	// gather adjoint solution for that element
	for (int m = 0;m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		adjointSol[3*m] = ts_stressAdjoint[3*node];
		adjointSol[3*m+1] = ts_stressAdjoint[3*node+1];
		adjointSol[3*m+2] = ts_stressAdjoint[3*node+2];
	}
}
/**********************************************************************/
void TopOpt::computeAdjointStrainAtElem(int elem, double strains[6])
{
	int i,m,node;
	double u[8],v[8],w[8];
	double J00, J11,J22,T1, T2, T3;

	for (i = 0; i < 6; i++)
		strains[i] = 0;

	if (getElemPseudoDensity(elem) <0.5) return;

	// gather adjoint solution for that element
	for (m = 0;m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		u[m] = ts_stressAdjoint[3*node];
		v[m] = ts_stressAdjoint[3*node+1];
		w[m] = ts_stressAdjoint[3*node+2];
	}
	J00 = 2./h_dx;//Jinv
	J11 = 2./h_dy;
	J22 = 2./h_dz;

	
	for (i = 0; i < 8; i++) {// contribution from all 8 nodes
		T1 = J00*gradNxiCenter[i];
		T2 = J11*gradNetaCenter[i] ;
		T3 = J22*gradNmuCenter[i];
		strains[0] += T1*u[i];// ux
		strains[1] += T2*v[i];// vy
		strains[2] += T3*w[i];// wz
		strains[3] += (T2*u[i] + T1*v[i]);// uy + vx
		strains[4] += T3*u[i] + T1*w[i];// uz + wx
		strains[5] += T3*v[i] + T2*w[i];// vz + wy
	}
}
/**********************************************************************/
double TopOpt::computeBodyForceSensitivity(int elem)
	// Compute fBodyForce(elem)*u2
{
	int i,m;
	double result,force[3],nodeForce[24],u[24];

	force[0] = h_bodyForce[3*elem]/8;// shared by 8 nodes
	force[1] = h_bodyForce[3*elem+1]/8;// shared by 8 nodes
	force[2] = h_bodyForce[3*elem+2]/8;// shared by 8 nodes
	if (NORM3D(force) < TOL) return 0;

	for (m =0; m < 8; m++) {// first compute the nodal force due to the element body force
		nodeForce[3*m] = force[0];
		nodeForce[3*m+1] = force[1];
		nodeForce[3*m+2] = force[2];
	}
	getElemSolution(elem,u);
	result = 0;
	for (i =0; i < 24; i++)	
		result += nodeForce[i]*u[i];

	//vol = getElemVolume(elem);
	//result /= vol;
	return result;
}
/**********************************************************************/
void  TopOpt::computeTopSensInitial(void)
{
	// Compute Topological sensitivity at the center of each element
	int		elem;
    double TSCompliance = 0,TSAdjoint= 0;
	double TSComplianceMin,TSComplianceMax,TSStressMin,TSStressMax,TSEigenMax,TSEigenMin,
			TSOutputComplianceMin,TSOutputComplianceMax,TSMutualComplianceMin,TSMutualComplianceMax;
	double elemSol[24],adjointSol[24];
    double HdeltaT[24],correction;


	if (ts_topSensElem == NULL) {
        writeMessageToParetoLog("TopOpt: Initialize Topopt!");
		return;
	}
	if (ts_useAnalyticalTS) return; // no need for norms
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Computing Norm of topological sensitivity");

	TSComplianceMin =  HUGE_VAL;
	TSComplianceMax =  -HUGE_VAL;

    if (ts_topoptMode == TOPOPT_MODE_THERMAL)  {
        double thermalSol[8];
        for (elem = 0; elem < h_nElems;elem++) {
            if (getElemPseudoDensity(elem) < 0.5) continue;// cannot update TS of elements that are outside
            getElemNodesTemperature(elem,thermalSol);
            TSCompliance = computeElemInnerProductThermal(elem,thermalSol,thermalSol);

            ts_topSensElem[elem] = TSCompliance;
            TSComplianceMin = min(TSComplianceMin,TSCompliance);
            TSComplianceMax = max(TSComplianceMax,TSCompliance);
        }
        TSComplianceNorm0 = TSComplianceMax - TSComplianceMin;
        ts_compliance0 = computeCompliance_thermal();
    }

	if (ts_topoptQuantity == VONMISES){
		TSStressMin = HUGE_VAL;
		TSStressMax = -HUGE_VAL;
	}
	else if (ts_topoptQuantity == EIGENVALUE){
		TSEigenMin = 0.0;
		TSEigenMax = 1.0;
	}
	else if (ts_topoptQuantity == OUTPUT_COMPLIANCE){ 
		TSOutputComplianceMin = HUGE_VAL;
		TSOutputComplianceMax = -HUGE_VAL;

	}
	else if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE){ 
		TSMutualComplianceMin = HUGE_VAL;
		TSMutualComplianceMax = -HUGE_VAL;
		ts_compliance0 = ts_compliance;
		ts_outputcompliance0 = ts_outputcompliance;
	}
	for (elem = 0; elem < h_nElems;elem++) {
		getElemSolution(elem,elemSol); 

		TSCompliance = computeElemInnerProduct(elem,elemSol,elemSol);
        if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC){//need a correction
            /*See Eqn 3.29 in "Stress constrained thermo-elastic topology optimization " Structural and Multidisciplinary Optimization archive
              Volume 56 Issue 6, December 2017 Pages 1413-1427 */

            getHdeltaT(elem, HdeltaT);
            correction = 0;
            for (int i = 0; i < 24; i++)
                correction += elemSol[i]*HdeltaT[i];

            //cout << elem << ": TSCompliance " << TSCompliance << "; correction " << correction << endl;
            //TSCompliance -= correction;
            // We are still missing one term from Eqn 3.29
        }
		TSComplianceMin = min(TSComplianceMin,TSCompliance);
		TSComplianceMax = max(TSComplianceMax,TSCompliance);
		if (ts_topoptQuantity == VONMISES) {
			getStressAdjointSolution(elem,adjointSol); 
			TSAdjoint = computeElemInnerProduct(elem,elemSol,adjointSol);
			TSStressMin = min(TSStressMin,TSAdjoint);
			TSStressMax = max(TSStressMax,TSAdjoint);
		}
		else if (ts_topoptQuantity == EIGENVALUE){ 
			int materialID = getElemMaterialID(elem);
			double E,nu,strains[6],stresses[6],strainEnergy;
			int desiredEigenMode = 1;//lowest eigen-value
			int *elementNodes,elementDOF[24];
			elementNodes = (int *)malloc(8*sizeof(int));
			double temp3 = 0.0,temp2 = 0.0;
			double TSEigen = 0.0;

			getElasticMaterialProperties(materialID,&E,&nu);
			for (int i = 0;i < 6; i++) {
				stresses[i] = h_elemstresses[i][elem];
				strains[i] = h_elemstrains[i][elem];
			}
			strainEnergy = 0;//element strain energy
			for (int i =0; i < 6; i++) 
				strainEnergy += stresses[i]*strains[i];//sum(sum(stressTensor.*strainTensor));

			getElemNodes(elem,elementNodes);
			for(int i =0; i < 8; i++){
				elementDOF[3*i] = elementNodes[i]*3;
				elementDOF[3*i+1] = elementNodes[i]*3+1;
				elementDOF[3*i+2] = elementNodes[i]*3+2;
			}
			for (int j = 0; j < 24; j++)
				temp3 += h_eigenModes[desiredEigenMode-1][elementDOF[j]]*h_eigenModes[desiredEigenMode-1][elementDOF[j]];
			temp2 = h_eigenValues[desiredEigenMode-1]*h_eigenValues[desiredEigenMode-1]*temp3*nu;
			TSEigen = strainEnergy-temp2;

			ts_topSensElem[elem] = TSEigen;
			free(elementNodes); elementNodes = NULL;

			TSEigenMin = min(TSEigenMin,TSEigen);
			TSEigenMax = max(TSEigenMax,TSEigen);

		}
		else if (ts_topoptQuantity == OUTPUT_COMPLIANCE) { 
			getStressAdjointSolution(elem,adjointSol); 
			TSAdjoint = computeElemInnerProduct(elem,adjointSol,adjointSol);
			TSOutputComplianceMin = min(TSOutputComplianceMin,TSAdjoint);
			TSOutputComplianceMax = max(TSOutputComplianceMax,TSAdjoint);
		}
		else if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) { 
			getStressAdjointSolution(elem,adjointSol); 
			TSAdjoint = computeElemInnerProduct(elem,elemSol,adjointSol);
			TSMutualComplianceMin = min(TSMutualComplianceMin,TSAdjoint);
			TSMutualComplianceMax = max(TSMutualComplianceMax,TSAdjoint);
		}

		TSComplianceNorm0 = TSComplianceMax - TSComplianceMin;
		if (ts_topoptQuantity == VONMISES)
			TSStressNorm0 = TSStressMax - TSStressMin;
		if (ts_topoptQuantity == EIGENVALUE)
			TSEigenNorm0 = TSEigenMax - TSEigenMin;
		if (ts_topoptQuantity == OUTPUT_COMPLIANCE)
			TSOutputComplianceNorm0 = TSOutputComplianceMax - TSOutputComplianceMin;
		if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) {
			TSMutualComplianceNorm0 = TSMutualComplianceMax - TSMutualComplianceMin;
		}
		ts_compliance0 = computeCompliance();
	}
    writeMessageToParetoLog("Compliance Norm: %e ",TSComplianceNorm0);
}
/**********************************************************************/
void  TopOpt::computeTopSensAtElems(void)
{
	if (ts_useAnalyticalTS)
		computeTopSensAtElemsAnalytical();
	else
		computeTopSensAtElemsApproximate();

    if (ts_verbose) writeMessageToParetoLog("Topopt: TSMin = %e TSMax = %e",ts_TSMin,ts_TSMax );
}
/**********************************************************************/
void  TopOpt::computeTopSensAtElemsAnalytical(void)
{
	// Compute Topological sensitivity at the center of each element
	int		elem,i;
	double	strains[6],adj_strains[6],stresses[6],E,nu;
	double	strainEnergy,temp2,TSCompliance = 0, TSObjective= 0;

	
    if (ts_verbose) writeMessageToParetoLog("TopOpt: In computeTopSensAtElemsAnalytical");

	if (ts_topSensElem == NULL) {
        writeMessageToParetoLog("TopOpt: Initialize Topopt!");
		return;
	}
	int materialID;
	
	// find the topsensitivity at all elements
	ts_TSMin = HUGE_VAL;
	ts_TSMax = -HUGE_VAL;

    double HPrime[24][8],elemSol[24];
    if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC)
        getHPrime(HPrime);
#pragma omp parallel for private(elem, materialID,E,nu,i,stresses,strains,strainEnergy,temp2,TSCompliance,adj_strains,TSObjective)
	for (elem = 0; elem < h_nElems;elem++) {
		if (getElemPseudoDensity(elem) <0.5) continue;// cannot explicitly update TS of elements that are outside ... will use smoothening
		materialID = getElemMaterialID(elem);

		getElasticMaterialProperties(materialID,&E,&nu);
		for (i = 0;i < 6; i++) {
			stresses[i] = h_elemstresses[i][elem];
			strains[i] = h_elemstrains[i][elem];
		}
		strainEnergy = 0;//element strain energy
		for (i =0; i < 6; i++) 
			strainEnergy += stresses[i]*strains[i];//sum(sum(stressTensor.*strainTensor));
		
		temp2 = (stresses[0]+stresses[1]+stresses[2])*(strains[0]+strains[1]+strains[2]); // trace(stressTensor)*trace(strainTensor)
		TSCompliance = 4.0/(1+nu)*strainEnergy-(1-3*nu)/(1-nu*nu)*temp2;
        TSCompliance -=computeBodyForceSensitivity(elem);

        if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC){//need a correction
            /*See Eqn 3.29 in "Stress constrained thermo-elastic topology optimization " Structural and Multidisciplinary Optimization archive
              Volume 56 Issue 6, December 2017 Pages 1413-1427 */
            double HdeltaT[24],correction,nodeTemp[8];
            getElemNodesTemperature(elem, nodeTemp);
            for(int i = 0; i < 24 ; i++) {
                HdeltaT[i] = 0;
                for (int j = 0; j < 8; j++){
                    HdeltaT[i] += HPrime[i][j]*(nodeTemp[j]-h_thermoElasticReferenceTemperature);
                }
            }

            getElemSolution(elem,elemSol);//displacements
            correction = 0;
            for (int i = 0; i < 24; i++)
                correction += elemSol[i]*HdeltaT[i];

            TSCompliance *= h_dx*h_dy*h_dz;
            //cout << TSCompliance << " " << correction << endl;
            TSCompliance -= correction;// correction due to Bubble function
        }

		TSCompliance /= TSComplianceNorm0;
		if ( (ts_topoptQuantity == COMPLIANCE) || (ts_topoptQuantity == VOLUME) )
			ts_topSensElem[elem] = TSCompliance;

		else if (ts_topoptQuantity == VONMISES) {
			computeAdjointStrainAtElem(elem,adj_strains);
			strainEnergy = 0;//element strain energy
			for (i =0; i < 6; i++) 
				strainEnergy += stresses[i]*adj_strains[i];//sum(sum(stressTensor.*strainTensor));
			temp2 = (stresses[0]+stresses[1]+stresses[2])*(adj_strains[0]+adj_strains[1]+adj_strains[2]); // trace(stressTensor)*trace(strainTensor)
			TSObjective = 4.0/(1+nu)*strainEnergy-(1-3*nu)/(1-nu*nu)*temp2;
			TSObjective /= TSStressNorm0;
			ts_topSensElem[elem] = ts_complianceWt*TSCompliance + (1-ts_complianceWt)*TSObjective;
		}

#pragma omp critical
		ts_TSMin = (ts_topSensElem[elem] < ts_TSMin)? ts_topSensElem[elem]: ts_TSMin;
		ts_TSMax = (ts_topSensElem[elem] > ts_TSMax)? ts_topSensElem[elem]: ts_TSMax;
	}
	modifyTSField();

	if (ts_verbose) cout << "Topopt: TSMin = " << ts_TSMin << " TSMax = " << ts_TSMax << endl;

	if (ts_TSMin < 0)
		ts_isTSMinNegative = true;
	else
		ts_isTSMinNegative = false;
    // copy the unscaled values for adaptive stepping
    memcpy(ts_topSensElemUnscaled,ts_topSensElem,h_nElems*sizeof(double));

    if (ts_scaleTSField) {
#pragma omp parallel for private(elem)
		for (elem = 0; elem < h_nElems;elem++) {
			ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin);	
			if (h_keepElem[elem])
				ts_topSensElem[elem] = 1.0;
            if (getElemPseudoDensity(elem)< 0.5) ts_topSensElem[elem] = 0;
		}
		ts_TSMax = 1.0; ts_TSMin = 0;
	}
	else {
#pragma omp parallel for private(elem)
		for (elem = 0; elem < h_nElems;elem++) {
			if (h_keepElem[elem])
				ts_topSensElem[elem] = ts_TSMax;
		}
	}

    //if (ts_verbose) writeMessageToParetoLog("Topopt: TSMin = %e TSMax = %e",ts_TSMin,ts_TSMax );
}
/**********************************************************************/
void  TopOpt::computeTopSensAtElemsApproximate(void)
{
	// Compute Topological sensitivity at the center of each element
	int		elem;
    double	TSCompliance = 0, TSObjective= 0,TSOutputCompliance=0,TSMutualCompliance=0;
	double elemSol[24],adjointSol[24],thermalSol[8];

	
    if (ts_verbose) writeMessageToParetoLog("TopOpt: In computeTopSensAtElemsApproximate");


	if (ts_topSensElem == NULL) {
        writeMessageToParetoLog("TopOpt: Initialize Topopt!");
		return;
	}
	
	// find the topsensitivity at all elements
	ts_TSMin = HUGE_VAL;
	ts_TSMax = -HUGE_VAL;

    if (ts_topoptMode == TOPOPT_MODE_THERMAL)  {
		for (elem = 0; elem < h_nElems;elem++) {
			if (getElemPseudoDensity(elem) < 0.5) continue;// cannot update TS of elements that are outside
            getElemNodesTemperature(elem,thermalSol);
            TSCompliance = computeElemInnerProductThermal(elem,thermalSol,thermalSol);
            getTemperatureInElem(elem);
            TSCompliance /= TSComplianceNorm0;
           // TSCompliance=  (getTemperatureInElem(elem)-300)/100000;
            ts_topSensElem[elem] = TSCompliance;
			ts_TSMin = (ts_topSensElem[elem] < ts_TSMin)? ts_topSensElem[elem]: ts_TSMin;
			ts_TSMax = (ts_topSensElem[elem] > ts_TSMax)? ts_topSensElem[elem]: ts_TSMax;
		}
	}
	double Sin,Sout,Smut,eta;
	double Win,Wout,Wmut,denom;
    double HPrime[24][8];
	if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) {
		Sin = 0;
		Sout = 0;
		Smut = 0;
		eta = 0.5;
		for (elem = 0; elem < h_nElems;elem++) {
			getElemSolution(elem,elemSol); 
			Sin += computeElemInnerProduct(elem,elemSol,elemSol);
			getStressAdjointSolution(elem,adjointSol); 
			Sout += computeElemInnerProduct(elem,adjointSol,adjointSol);
			Smut += computeElemInnerProduct(elem,elemSol,adjointSol);
		}
		denom = 2*eta*Sin + 2*(1-eta)*Sout;
		Win = 2*eta*Smut/(denom*denom);
		Wout = 2*(1-eta)*Smut/(denom*denom);
		Wmut = 0.5/denom;
		denom = Win + Wout+Wmut;
	
		Win /=denom;
		Wout /=denom;
		Wmut /=denom;

		Win = MIN(1,MAX(0.01,Win));
		Wout = MIN(1,MAX(0.01,Wout));
		Wmut = MIN(1,MAX(0.01,Wmut));

	}
    if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC)
        getHPrime(HPrime);
    if ((ts_topoptMode == TOPOPT_MODE_ELASTIC)  ||
        (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC) ) {
    //#pragma omp parallel for private(elem,elemSol,adjointSol,TSCompliance,TSObjective)
		for (elem = 0; elem < h_nElems;elem++) {
			if (getElemPseudoDensity(elem) < 0.5) continue;// cannot compute TS of elements that are outside; use filtering later on
			getElemSolution(elem,elemSol); 
			TSCompliance = computeElemInnerProduct(elem,elemSol,elemSol);
            TSCompliance += computeBodyForceSensitivity(elem);
            if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC){//need a correction
                /*See Eqn 3.29 in "Stress constrained thermo-elastic topology optimization " Structural and Multidisciplinary Optimization archive
                  Volume 56 Issue 6, December 2017 Pages 1413-1427 */
                double HdeltaT[24],correction,nodeTemp[8];
                getElemNodesTemperature(elem, nodeTemp);
                for(int i = 0; i < 24 ; i++) {
                    HdeltaT[i] = 0;
                    for (int j = 0; j < 8; j++){
                        HdeltaT[i] += HPrime[i][j]*(nodeTemp[j]-h_thermoElasticReferenceTemperature);
                    }
                }

                correction = 0;
                for (int i = 0; i < 24; i++)
                    correction += elemSol[i]*HdeltaT[i];

                //cout << TSCompliance << " " << correction << endl;
                TSCompliance -= 2*correction;// correction due to Bubble function
            }

            TSCompliance /= TSComplianceNorm0;

			if (ts_topoptQuantity == VOLUME) 
				ts_topSensElem[elem] = -1;// we may need to include compliance to avoid singularity
            else if (ts_topoptQuantity == COMPLIANCE) {
				ts_topSensElem[elem] = TSCompliance ;

            }
			else if (ts_topoptQuantity == VONMISES) {
				getStressAdjointSolution(elem,adjointSol); 
				TSObjective = computeElemInnerProduct(elem,elemSol,adjointSol);
				TSObjective /= TSStressNorm0;
				ts_topSensElem[elem] = ts_complianceWt*TSCompliance + (1-ts_complianceWt)*TSObjective;
			}
			else if (ts_topoptQuantity == EIGENVALUE) { 
				double E,nu,strains[6],stresses[6],strainEnergy;
				int desiredEigenMode = 0;//lowest eigen-value
				int elementNodes[8],elementDOF[24];

				double temp3 = 0.0,temp2 = 0.0,rho,K,alpha,specificHeat;
				double TSEigen = 0.0;
				char materialName[256];

				getMaterialOfPart(0,&E,&nu,&rho,&K, &alpha,&specificHeat,materialName);
				for (int i = 0;i < 6; i++) {
					stresses[i] = h_elemstresses[i][elem];
					strains[i] = h_elemstrains[i][elem];
				}
				strainEnergy = 0;//element strain energy
				for (int i =0; i < 6; i++) 
					strainEnergy += stresses[i]*strains[i];//sum(sum(stressTensor.*strainTensor));

				getElemNodes(elem,elementNodes);
				for(int i =0; i < 8; i++){
					elementDOF[3*i] = elementNodes[i]*3;
					elementDOF[3*i+1] = elementNodes[i]*3+1;
					elementDOF[3*i+2] = elementNodes[i]*3+2;
				}
				for (int j = 0; j < 24; j++)
					temp3 += h_eigenModes[desiredEigenMode][elementDOF[j]]*h_eigenModes[desiredEigenMode][elementDOF[j]];
				temp2 = h_eigenValues[desiredEigenMode]*h_eigenValues[desiredEigenMode]*temp3*rho;
				TSEigen = strainEnergy-temp2;
				TSEigen = TSEigen/TSEigenNorm0;
				ts_topSensElem[elem] = ts_complianceWt*TSCompliance + (1 - ts_complianceWt)* TSEigen;
			}
			else if (ts_topoptQuantity == OUTPUT_COMPLIANCE) {
				getStressAdjointSolution(elem,adjointSol); 
				TSOutputCompliance = computeElemInnerProduct(elem,adjointSol,adjointSol);
				TSOutputCompliance /= TSOutputComplianceNorm0;
				ts_topSensElem[elem] = TSOutputCompliance;
			}
			else if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) {
				getStressAdjointSolution(elem,adjointSol); 
				TSOutputCompliance = computeElemInnerProduct(elem,adjointSol,adjointSol);
				TSMutualCompliance = -computeElemInnerProduct(elem,elemSol,adjointSol);
				ts_topSensElem[elem] = TSMutualCompliance;//Wmut*TSMutualCompliance+Win*TSCompliance + Wout*TSOutputCompliance;
			}
#pragma omp critical
			ts_TSMin = (ts_topSensElem[elem] < ts_TSMin)? ts_topSensElem[elem]: ts_TSMin;
			ts_TSMax = (ts_topSensElem[elem] > ts_TSMax)? ts_topSensElem[elem]: ts_TSMax;
		}
	}
	if (ts_topoptQuantity == VOLUME) 
		return;// don't scale since the field is a constant

    if (ts_verbose) writeMessageToParetoLog("TopOpt: TSMin = %e TSMax = %e",ts_TSMin,ts_TSMax );
	modifyTSField();


	if (ts_scaleTSField) {
        if (ts_verbose) writeMessageToParetoLog("Scaling TS field");
#pragma omp parallel for private(elem)
		for (elem = 0; elem < h_nElems;elem++) {
			ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin);
			if (h_keepElem[elem])
				ts_topSensElem[elem] = 1.0;
            if (getElemPseudoDensity(elem)< 0.5) ts_topSensElem[elem] = 0;
		}
		ts_TSMin = 0.0;
		ts_TSMax = 1.0;
	}
	else {
#pragma omp parallel for private(elem)
		for (elem = 0; elem < h_nElems;elem++) {
			if (h_keepElem[elem])
				ts_topSensElem[elem] = ts_TSMax;
		}
	}

}
/**********************************************************************/
void  TopOpt::kernelTSSmoothener()
{
	int elem;
	kernelSmoothener(ts_topSensElem);

	computeNodalTSField();
	imposeDrawDirection(ts_drawDirection);
	imposeSymmetry(ts_symmetryIndex);
	imposeCyclicSymmetry(ts_axiSymmetryIndex);


#pragma omp parallel for private(elem)
	for (elem = 0; elem < h_nElems;elem++) {
		if (h_keepElem[elem])// imposing draw direction may have affected nodes TS
			ts_topSensElem[elem] =ts_TSMax;	
	}
}
/**********************************************************************/
void  TopOpt::computeNodalTSField()
{
	int node,j,maxElem,codedElemNum,nodeIndex,elem,count;

#pragma omp parallel for private(node,maxElem,count,j,codedElemNum,nodeIndex,elem)
	for (node = 0; node < h_nNodes;node++) {
		ts_topSensNode[node] = 0;
		maxElem = h_nNeighboringElems[node];// how many elements connected to node
		count = 0;
		for (j =0; j < maxElem; j++) {//
			codedElemNum = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
			nodeIndex = codedElemNum/MAXELEMS;
			elem = codedElemNum - nodeIndex*MAXELEMS;
			//partId = getPartIdOfElem(elem);
			//if (!shouldPartBeOptimized(partId)) continue;// don't consider TS values from non-design parts

            //if (getElemPseudoDensity(elem) < 0.5) continue;
			// else even if the element is outside, use the previously computed TS value
			count++;
			ts_topSensNode[node] += ts_topSensElem[elem];	
		}
		count = MAX(1,count);
		ts_topSensNode[node] /= count;
        //cout << ts_topSensNode[node] << endl;
	}
}
/**********************************************************************/
void  TopOpt::TSSmoothener()
{
	connectivityTSSmoothener();
}
/**********************************************************************/
double TopOpt::getBucklingLoad(void)
{
	return h_bucklingLoadValue;
}
/**********************************************************************/
bool	 TopOpt::setAMBuildDirection(int buildDirection)
{
    ts_AMBuildDirection = 0;
	if (fabs(1.*buildDirection) > 3) return false;
	ts_AMBuildDirection = buildDirection;// 0 for no build direction, +/-1 for x, +/-2 for y, +/-3 for z

	return true;
}

/**********************************************************************/
void  TopOpt::connectivityTSSmoothener()
{
	// Compute TS at each node as average of each neighboring element
	// Then compute TS at each elem as average of 8 nodes
	// Finally recompute TS at node as average of each neighboring element
	int node,elem,m;

    if (ts_verbose) writeMessageToParetoLog("TopOpt: connectivityTSSmoothener");
	for (int iter = 0; iter < ts_nSmoothingIterations; iter++) {
		computeNodalTSField();
        imposeSupportStructureConstraintOnNodes();
		imposeDrawDirection(ts_drawDirection);
		imposeSymmetry(ts_symmetryIndex);
		imposeCyclicSymmetry(ts_axiSymmetryIndex);
        imposeGridPattern(ts_gridPattern);

#pragma omp parallel for private(elem,m,node)
		for (elem = 0; elem < h_nElems;elem++) {
			if (h_keepElem[elem]){// imposing constraints may have affected nodes TS
				ts_topSensElem[elem] = ts_TSMax;
				continue;
			}
			//else
			ts_topSensElem[elem] = 0;
			for (m = 0; m < 8; m++) {
				node = h_elems[(h_nElems)*m + elem];
				ts_topSensElem[elem] += ts_topSensNode[node];
			}
			ts_topSensElem[elem] /= 8;		
		}
        imposeExtrudeOnElements(ts_extrudeDirection);
	}
   /* ts_TSMin = HUGE;
    ts_TSMax = -HUGE;
    for (int elem = 0; elem < h_nElems; elem++){
        ts_TSMin = (ts_topSensElem[elem] < ts_TSMin)? ts_topSensElem[elem]: ts_TSMin;
        ts_TSMax = (ts_topSensElem[elem] > ts_TSMax)? ts_topSensElem[elem]: ts_TSMax;
    }*/
}
/**********************************************************************/
void TopOpt::fixNodalTSValues(int isoCounter, bool invert)
{
	int elem,nodes[8];
	bool plotNode;
	int elems[50],nElems,nPartsOverlapping,partId;
	bool overLappingParts[MAX_PARTS];
	double TSCutoff;
	

    if (isoCounter == -1)
        isoCounter = ts_currentIsoCounter;


    TSCutoff = ts_isoValueSave[isoCounter];

	// elements belonging to non-design parts  isosurface treatment
	if (getNumberOfParts() > 1) {
		for (int node = 0; node < h_nNodes;node++) {
			nElems = getNeighboringElemsOfNode(node,elems);
			plotNode = false;// if false, this node will need to be removed from isosurface
			for (int i = 0; i < nElems; i++){
				elem = elems[i];
				partId = getPartIdOfElem(elem);
				if (shouldPartBeOptimized(partId)){
					plotNode = true;// no need to fix this node
					break;
				}
			}
			if (!plotNode)
                ts_topSensNodeSave[isoCounter][node] = TSCutoff-fabs(0.0001*TSCutoff);

			// we also need to check if an element overlaps two different parts that are being optimized
			for (int part = 0; part < getNumberOfParts(); part++)
				overLappingParts[part] = false;

			for (int i = 0; i < nElems; i++){
				elem = elems[i];
				partId = getPartIdOfElem(elem);
				if (shouldPartBeOptimized(partId))
					overLappingParts[partId] = true;
			}
			nPartsOverlapping = 0;
			for (int part = 0; part < getNumberOfParts(); part++)
				if (overLappingParts[part]) 
					nPartsOverlapping++;
			if (nPartsOverlapping > 1)
                ts_topSensNodeSave[isoCounter][node] = TSCutoff-fabs(0.001*TSCutoff);
		}
	}

	//do we want to create the positive isosurface or the negative isosurface
	
	if (!invert){//positive surface
		// This isosurface is consistent with mesh
		for (elem = 0; elem < h_nElems;elem++) {
			getElemNodes(elem,nodes);
			if (ts_ensureIsoSurfaceConnectivity) {
				if ((getElemPseudoDensity(elem) > 0.5)){// element exists in the FEA solution
					for (int m = 0; m < 8; m++)
                        ts_topSensNodeSave[isoCounter][nodes[m]] =  MAX(ts_topSensNodeSave[isoCounter][nodes[m]],TSCutoff+fabs(0.0001*TSCutoff));// for plotting purposes, force the TS values to be higher than cutoff
				}
			}
			// for all elements
			for (int m = 0; m < 8; m++) {
				if  (isNodeOnBoundary(nodes[m]))// for the boundary nodes
                    ts_topSensNodeSave[isoCounter][nodes[m]] = MIN(ts_topSensNodeSave[isoCounter][nodes[m]],TSCutoff-fabs(0.01*TSCutoff));// for plotting purposes
			}
		}
	}
	else {//inverted surface
		double a,b;
		a = TSCutoff/(TSCutoff-1);
		b = -a;
		for (int node = 0; node < h_nNodes;node++) {
            ts_topSensNodeSave[isoCounter][node] = a*ts_topSensNodeSave[isoCounter][node] +b;
			if  (isNodeOnBoundary(node))// for the boundary nodes
                ts_topSensNodeSave[isoCounter][node] = MIN(ts_topSensNodeSave[isoCounter][node],TSCutoff-fabs(0.01*TSCutoff));// for plotting purposes
		}
	}
	
}
/**********************************************************************/
double  TopOpt::getTSOfElem(int elem)
{
	if (elem < 0 || elem > h_nElems-1){
        writeMessageToParetoLog("TopOpt: Invalid elem %d in getTSOfElem",elem);
		return ts_TSMin;
	}
	if (getElemPseudoDensity(elem) < 0.5)
		return ts_TSMin;
	return (ts_topSensElem[elem]);
}

/**********************************************************************/
void  TopOpt::getTSFieldinElems(double *tsField)
	// it is assumed that the memory has been allocated
{
	memcpy(tsField,ts_topSensElem,h_nElems*sizeof(double));
}
/**********************************************************************/
void  TopOpt::getTopSensAtElemNodes(int elem, double TS[8])
{
	int node;
	for (int m = 0;m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		TS[m] = ts_topSensNode[node];
	}
}
/**********************************************************************/
void  TopOpt::resetIsoCounter(void)
{
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Resetting iso counter");
	ts_currentIsoCounter = -1;
}
/**********************************************************************/
double  TopOpt::getCurrentTSCutoff(void)
{
	if (ts_currentIsoCounter == -1) return ts_TSMin;
	return ts_isoValueSave[ts_currentIsoCounter];
}
/**********************************************************************/
int  TopOpt::getCurrentIsoCounter(void)
{
	return ts_currentIsoCounter;
}
/**********************************************************************/
void  TopOpt::setTopOptFieldAtNodes(double *nodalValues)
	// can be used by external function
{
	#pragma omp parallel for
	for (int node = 0;node < h_nNodes; node++) 
		 ts_topSensNode[node] = nodalValues[node];
}
/**********************************************************************/
bool  TopOpt::saveTopOptField(double volFrac,double isoValue,bool forceSave)
{
	double volfracPrev;
	// isosurfaces are saved under the following 3 conditions
	// 1. forced by user
	// 2. more than x% change in volume since last save
	// 3. the change in compliance is more than 25%
	static int saveCounter = 0;


	saveCounter++;
	if ((!forceSave)  &&  (saveCounter < ts_saveEveryNIterations)) {// check if we must avoid saving
		return false;
	}
	volfracPrev = -1;
	if (ts_currentIsoCounter >= 0)
		volfracPrev = ts_volFracSave[ts_currentIsoCounter];
	
	if  (fabs(volfracPrev  - volFrac) < 0.001)
		return false;
	saveCounter = 0;
	
	ts_currentIsoCounter = MIN(ts_currentIsoCounter,MAX_ISO_SAVES-2);
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Saving at isovalue %d, vol Frac %.3f ",ts_currentIsoCounter+1,volFrac);
	ts_isoValueSave[ts_currentIsoCounter+1] = isoValue;
	ts_volFracSave[ts_currentIsoCounter+1] = volFrac;
	ts_maxDisplacementSave[ts_currentIsoCounter+1] = getDeformationMax();
    ts_complianceSave[0][ts_currentIsoCounter+1] = computeCompliance();
    ts_maxTemperatureSave[0][ts_currentIsoCounter+1] = getTemperatureMax();

	for (int part  = 0; part < getNumberOfParts(); part++)
        ts_maxStressSave[0][ts_currentIsoCounter+1][part] = getMaxStressInPart(part);

	ts_timingSave[ts_currentIsoCounter+1] = ts_timer.getElapsedTimeInSec();

	if (ts_useEigenConstraint) {
		if (h_eigenValues != NULL)
			ts_firstModeSave[ts_currentIsoCounter + 1] = h_eigenValues[0];

	}
	#pragma omp parallel for
	for (int node = 0;node < h_nNodes; node++) 
		 ts_topSensNodeSave[ts_currentIsoCounter+1][node] = ts_topSensNode[node];

	#pragma omp parallel for
	for (int elem = 0;elem < h_nElems; elem++) 
		 ts_elemExists[ts_currentIsoCounter+1][elem] = (getElemPseudoDensity(elem) > 0.5);

	ts_currentIsoCounter++;// don't update until the end since a different thread will query the latest counter
	//ts_currentIsoCounter = MIN(ts_currentIsoCounter,MAX_ISO_SAVES-2);
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Saved ts_currentIsoCounter %d with isoValue = %.3e ",ts_currentIsoCounter,isoValue);
	return true;
}
/**********************************************************************/
void  TopOpt::getAllVolFracs(double *vf)
{
	for (int i =0; i <= ts_currentIsoCounter;i++)
		vf[i] = ts_volFracSave[i]/ts_volFracSave[0];
}
/**********************************************************************/
void  TopOpt::getMaxTemperatureAtAllVolFracs(double *vf,double maxTemperature[MAX_BC_SET][MAX_ISO_SAVES])
{
    for (int i =0; i <= ts_currentIsoCounter;i++){
        vf[i] = ts_volFracSave[i];
        for (int loadSet = 0; loadSet < MAX_BC_SET; loadSet++){
            maxTemperature[loadSet][i] = ts_maxTemperatureSave[loadSet][i];
        }
    }
}
/**********************************************************************/
void  TopOpt::getResultsAtAllVolFracs(double *vf, double *maxDisp, double maxStress[MAX_BC_SET][MAX_ISO_SAVES],double compliance[MAX_BC_SET][MAX_ISO_SAVES], double *timeTaken)
{

	for (int i =0; i <= ts_currentIsoCounter;i++){
		vf[i] = ts_volFracSave[i];
		maxDisp[i] = ts_maxDisplacementSave[i];
        for (int loadSet = 0; loadSet < MAX_BC_SET; loadSet++){
            compliance[loadSet][i] = ts_complianceSave[loadSet][i];
            maxStress[loadSet][i] =ts_maxStressSave[loadSet][i][0];
        }
		timeTaken[i] = ts_timingSave[i];
	}
}
/**********************************************************************/
void	 TopOpt::getEigenResultsAtAllVolFracs(double *vf, double *firstEigenMode)
{
	for (int i = 0; i <= ts_currentIsoCounter; i++) {
		vf[i] = ts_volFracSave[i];
		firstEigenMode[i] = ts_firstModeSave[i];
	}
}
/**********************************************************************/
double  TopOpt::getInitialDisplacement(void)
{
	if (ts_currentIsoCounter == -1)
		return getDeformationMax();
	else
		return ts_displacement0;
}
/**********************************************************************/
double  TopOpt::getInitialMaxStressInPart(int part)
{
    int loadSet,isoCounter;

    if (part >= MAX_PARTS)
        return 0;
    loadSet = 0;
    isoCounter = 0;
    if (ts_currentIsoCounter == -1)
        return h_vonmisesMaxInPart[part];
    else
        return ts_maxStressSave[loadSet][isoCounter][part];
}
/**********************************************************************/
double  TopOpt::getInitialVonMisesStress(void)
{
	if (ts_currentIsoCounter == -1)
		return getMaxVonMisesStress();
	else
		return ts_vonMisesStress0;
}
/**********************************************************************/
double  TopOpt::getMaxDispAtCounter(int isoCounter)
{

	if (isoCounter == -1) return 0;
	if (isoCounter > ts_currentIsoCounter) return 0;
	return ts_maxDisplacementSave[isoCounter];
}
/**********************************************************************/
double  TopOpt::getFirstEigenValueAtCounter(int isoCounter)
{
	if (isoCounter == -1) return 0;
	if (isoCounter > ts_currentIsoCounter) return 0;
	return ts_firstModeSave[isoCounter];
}
/**********************************************************************/
double  TopOpt::getMaxStressInPartAtCounter(int isoCounter,int part)
{

	if (isoCounter == -1) 0;
	if (isoCounter > ts_currentIsoCounter) 0;
    return ts_maxStressSave[0][isoCounter][part];
}
/**********************************************************************/
int  TopOpt::getClosestCounterAtVolFraction(double volFrac)
{
	int counter,bestCounter;
	double maxErr=1e8;

	if (ts_currentIsoCounter == -1)
		return -1;
	bestCounter = 0;
	maxErr=1e8;
	for (counter = 0; counter <= ts_currentIsoCounter;counter++){
		if (fabs(ts_volFracSave[counter] - volFrac) <= maxErr){
			bestCounter = counter;
			maxErr = fabs(ts_volFracSave[bestCounter] - volFrac) ;
		}
	}
    //if (ts_verbose) writeMessageToParetoLog("Topopt: Retrieving counter %d at vol fraction %0.4f, isoCutoff = %e ",bestCounter);
	return bestCounter;
}
/**********************************************************************/
int  TopOpt::getClosestCounterAtVolFraction(double volFrac,double *actualVolFrac,double *isoCutoff, double *timeInSecs)
{
	int counter,bestCounter;
	double maxErr=1e8;

	*actualVolFrac = 1.0;
	if (ts_currentIsoCounter == -1)
		return -1;
	bestCounter = 0;
	maxErr=1e8; 
	for (counter = 0; counter <= ts_currentIsoCounter;counter++){
		if (fabs(ts_volFracSave[counter] - volFrac) <= maxErr){
			bestCounter = counter;
			maxErr = fabs(ts_volFracSave[bestCounter] - volFrac) ;
		}
	}
	*actualVolFrac = ts_volFracSave[bestCounter];
	*isoCutoff = ts_isoValueSave[bestCounter];

	*timeInSecs = ts_timingSave[bestCounter];
    //if (ts_verbose) writeMessageToParetoLog("Topopt: Retrieving counter %d at vol fraction %0.4f, isoCutoff = %e \n",bestCounter,*actualVolFrac,*isoCutoff);
	return bestCounter;
}
/**********************************************************************/
bool  TopOpt::doesElemExistsAtTSCounter(int elem, int counter)
{
	if ((elem < 0) || (elem >= h_nElems)) return false;
	if (ts_currentIsoCounter == -1) return true;// no topopt has been executed, all elements exist
	if (counter < 0) return false;
	if (ts_elemExists[counter] == NULL) {// topopt not initialized ... use pseudodensity
		return (getElemPseudoDensity(elem) > 0.5);
	}
	return ts_elemExists[counter][elem];
}
/**********************************************************************/
double  TopOpt::getTopSensAtElemAtCounter(int elem, int counter)
{
	if ((elem < 0) || (elem >= h_nElems)) return 0;
	if (counter < 0) return 0;
	int nodes[8];
	double tsValue;
	getElemNodes(elem,nodes);
	tsValue = 0;
	for (int i = 0; i < 8;i++)
		tsValue += getTopSensAtNodeAtCounter(nodes[i],counter);
	return tsValue/8;
}
/**********************************************************************/
double  TopOpt::getTopSensAtNodeAtCounter(int node, int counter)
{
	if ((node < 0) || (node >= h_nNodes)) return 0;
	if (counter < 0) return 0;
	return ts_topSensNodeSave[counter][node];
}
/**********************************************************************/
void  TopOpt::getTopSensAtElemNodesAtCounter(int elem, int counter, double TS[8])
{
	
	if ((elem < 0) || (elem >= h_nElems)) return;
	if (counter < 0) {
		for (int m = 0;m < 8; m++) 
			TS[m] = 1.0;
		return;
	}
	int node;
	for (int m = 0;m < 8; m++) {
		node = h_elems[(h_nElems)*m + elem];
		TS[m] = ts_topSensNodeSave[counter][node];
	}
}
/**********************************************************************/
double  TopOpt::get_TSMax(void)
{
	return ts_TSMax;
}
/**********************************************************************/
double  TopOpt::get_TSMin(void)
{
	return ts_TSMin;
}
/**********************************************************************/
double	TopOpt::findVolFractionBelowCutoff(double cutoff)
{
	int nElems;
    if (ts_verbose) writeMessageToParetoLog("Topopt:In findNumElementsBelowCutoff " );
	
	nElems  = 0;
	for (int elem = 0; elem < h_nElems; elem++) {
		if (ts_topSensElem[elem] < cutoff)
			nElems++;
	}
	return (1.0*nElems/h_nElems);
}
/**********************************************************************/
double  TopOpt::computeAllowableVolumeDecrement(double allowableRelativeChangeInCompliance)
{
    // given a relative allowable change in compliance, predict an allowable decrement in volume
    double currentCompliance,deltaJ,allowableDeltaJ,elemVolume,tsElem;
    std::vector<double> myvector(ts_topSensElemUnscaled, ts_topSensElemUnscaled+h_nElems);// increasing order
    std::sort(myvector.begin(), myvector.end());
    int elem,nElemsOut;

    elemVolume = h_dx*h_dy*h_dz;
    currentCompliance = computeCompliance();
    allowableDeltaJ = allowableRelativeChangeInCompliance*currentCompliance;
    deltaJ = 0;
    nElemsOut = (1-getCurrentVolumeFraction())*h_nElems;
    for (elem = 0; elem < h_nElems; elem++){
        tsElem = myvector[elem]*elemVolume;
        deltaJ += tsElem;
        if (deltaJ > allowableDeltaJ)
            break;
    }
    return (1.0*(elem-nElemsOut)/h_nElems);
}
/**********************************************************************/
int  TopOpt::computeIsovalue(double volFractionDesired, double *isoCutoff, int *nElemsToRemove)
{
	/// Find the cutting place for the desired volume fraction
	double *temp;
	int nElementsBeingOptimized;

	if (getNumberOfParts() > 1)
        nElementsBeingOptimized = getNumberOfElementsInOptimizedParts();
	else
		nElementsBeingOptimized = h_nElems;


	// find the isoValue corresponding to volFractionDesired
	*nElemsToRemove = (int) ceil((1.0 -volFractionDesired)*nElementsBeingOptimized +1);


	// there is a problem in isosurface if too few elements are removed initially
	*nElemsToRemove = MAX(*nElemsToRemove,5);

	int rem = *nElemsToRemove % 2;
	*nElemsToRemove -= rem;// make it even

    if (ts_verbose) writeMessageToParetoLog("Topopt: Computing Iso Value; nElemsToRemove  = %d",*nElemsToRemove );
	int method = 0;

	if (method == 0) {
		std::vector<double> myvector(ts_topSensElem, ts_topSensElem+h_nElems);// the entire vector is used
		std::sort(myvector.begin(), myvector.end());
		*isoCutoff = myvector[*nElemsToRemove];
		vector<double>().swap(myvector);//release
	}
	else if (method == 1) {// may be buggy
		temp = (double *)malloc(h_nElems*sizeof(double));
		memcpy(temp,ts_topSensElem,h_nElems*sizeof(double));// so as to not overwrite
		//quickSortNonrecursive(temp,h_nElems-1);
		quickSort(temp,0,h_nElems-1);

		*isoCutoff = temp[*nElemsToRemove];
		free(temp);
	}
	else  {
		cout <<"Invalid method \n";
		return FAILURE;
	}
	if (ts_symmetryIndex == 7){//multiples of 8
		int rem = *nElemsToRemove%8;
		*nElemsToRemove -= rem;
	}
    if (ts_verbose) writeMessageToParetoLog("Topopt: Isovalue = %e ",*isoCutoff );

	return SUCCESS;
}
/**********************************************************************/
double TopOpt::computeMutualCompliance(void)
{
	int i;
	double J=0;

#pragma omp parallel for reduction (+: J )
	for (i =0; i < h_nNodes; i++) 
		J += (ts_stressAdjoint[3*i]*h_f[3*i] + ts_stressAdjoint[3*i+1]*h_f[3*i+1] + ts_stressAdjoint[3*i+2]*h_f[3*i+2]);
#pragma omp barrier 	
	return J;
}
/**********************************************************************/
double TopOpt::computeOutputCompliance(void)
{
	int i;
	double J=0;

#pragma omp parallel for reduction (+: J )
	for (i =0; i < h_nNodes; i++) 
		J += (ts_stressAdjoint[3*i]*h_output[3*i] + ts_stressAdjoint[3*i+1]*h_output[3*i+1] + ts_stressAdjoint[3*i+2]*h_output[3*i+2]);
#pragma omp barrier 
	return J;
}
/**********************************************************************/
double TopOpt::getPNormStress(void)
{
	int elem;
	double J=0,vonMises;
	
	for (elem = 0; elem < h_nElems; elem++) {
		vonMises =  getElemVonMisesStress(elem);
		J += pow(vonMises,ts_pNormIndex);
	}	
	J = pow(J,1.0/ts_pNormIndex);

	return J;
}
/**********************************************************************/
double TopOpt::computeJ(void)
{
    if (ts_topoptMode == TOPOPT_MODE_THERMAL)
         return computeCompliance_thermal();
    else
        return computeCompliance();
}
/**********************************************************************/
void TopOpt::setVonMisesAdjointForce(void)
{
	int elem,m,nodes[8],j,freeEncoded,ufree,vfree,wfree,whichNode,materialID;
	double stresses[6],vonMises,val,ce,vonMisesMax,scalingFactor;
	memset(ts_stressAdjointForce,0,3*h_nNodes*sizeof(double));// set the force vector to zero
	vonMisesMax = getMaxVonMisesStress(&whichNode); 
	scalingFactor = 0;
	for (elem = 0; elem < h_nElems;elem++) {
		vonMises = getElemVonMisesStress(elem);
		//if (vonMises < 0.1*vonMisesMax) continue;
		if (getElemPseudoDensity(elem) <0.5) continue;
		getElemNodes(elem,nodes);
		getElemStresses( elem,stresses); //get stresses in the middle of element
		ce = pow(vonMises,ts_pNormIndex-2)/sqrt(2.0);

		scalingFactor += pow(vonMises,ts_pNormIndex);
		materialID = getElemMaterialID(elem);
		for (m = 0; m < 8; m++) {
			for (j = 0; j < 3;j++) {
				val  = 0;
				val += (stresses[0]-stresses[1])*(Fe[materialID][0][3*m+j]-Fe[materialID][1][3*m+j]);
				val += (stresses[0]-stresses[2])*(Fe[materialID][0][3*m+j]-Fe[materialID][2][3*m+j]);
				val += (stresses[1]-stresses[2])*(Fe[materialID][1][3*m+j]-Fe[materialID][2][3*m+j]);
				val += 6*stresses[3]*Fe[materialID][3][3*m+j];
				val += 6*stresses[4]*Fe[materialID][4][3*m+j];
				val += 6*stresses[5]*Fe[materialID][5][3*m+j];
				ts_stressAdjointForce[3*nodes[m]+j] += ce*val;
			}
		}
	}
	scalingFactor = pow(scalingFactor,1./ts_pNormIndex -1);
	for (m = 0; m < h_nNodes; m++) {
		ts_stressAdjointForce[3*m]  *= scalingFactor;
		ts_stressAdjointForce[3*m+1] *= scalingFactor;
		ts_stressAdjointForce[3*m+2] *= scalingFactor;
	}
	// Remove forces on fixed dof
	for (m = 0; m < h_nNodes; m++) {
		freeEncoded = h_free[m];
		wfree = freeEncoded/4;
		vfree = (freeEncoded - wfree*4)/2;
		ufree = (freeEncoded - wfree*4 -vfree*2);
		if (ufree==0)
			ts_stressAdjointForce[3*m] = 0;
		if (vfree==0)
			ts_stressAdjointForce[3*m+1] = 0;
		if (wfree==0)
			ts_stressAdjointForce[3*m+2] = 0;
	}
}
/**********************************************************************/
void TopOpt::setPNorm(double pNorm)
{
	ts_pNormIndex = pNorm;
}
/**********************************************************************/
void TopOpt::setOutputAdjointForce(void)
// For compliant mechanism
{
	int node,j;
	for (node = 0; node < h_nNodes;node++) {
		for (j = 0; j < 3;j++)
			ts_stressAdjointForce[3*node+j] = h_output[3*node+j];
	}
}
/**********************************************************************/
void TopOpt::saveFEAandSolveEigen(int *nFEA)
{
	double *h_solSave, *h_fSave, maxStressSave;
	int nNodes = h_nNodes, nEigenModes, eigRelAccuracyExponent = 2;
	double eigRelativeAccuracy;
	double *eigenValues, **eigenModes;
	float maxTimeAllowed = 3600.0;

	nEigenModes = 1; // solve for only one eigen mode
	maxStressSave = h_vonmisesMax;
	h_solSave = (double *)malloc(3 * h_nNodes * sizeof(double));
	h_fSave = (double *)malloc(3 * h_nNodes * sizeof(double));
	getSolutionVector(h_solSave);// save the primary solution
	copy(h_f, h_fSave);// save the primary force

	// solve eigen-mode problem
    //initializeSolution();
    //releaseEigen();
	eigenModes = (double **)malloc(MAX_EIGEN_MODES * sizeof(double *));
	eigenValues = (double *)malloc(MAX_EIGEN_MODES * sizeof(double));
	for (int i = 0; i < MAX_EIGEN_MODES; i++)
		eigenModes[i] = (double *)malloc(3 * nNodes * sizeof(double));

	if (ts_verbose)
        writeMessageToParetoLog("TopOpt: Solving EigenMode problem");
	eigRelativeAccuracy = pow(10.0, -eigRelAccuracyExponent);
	bool success = solveElasticityModal(nEigenModes, eigenValues, eigenModes, eigRelativeAccuracy, maxTimeAllowed);
	*nFEA += 3* nEigenModes;// equivalent
	if (!success)
        writeMessageToParetoLog("TopOpt: EigenMode Error");
    else
        populateEigen(nEigenModes, eigenValues, eigenModes);

    if (ts_verbose) {
		for (int i = 0; i < nEigenModes; i++)
            writeMessageToParetoLog("Eigenmode %d: %.4f (Hz) ", i + 1, sqrt(eigenValues[i]) / (2 * 3.1415));
	}
	safeFree(eigenValues); eigenValues = NULL;
	for (int i = 0; i < MAX_EIGEN_MODES; i++) {
		safeFree(eigenModes[i]);
		eigenModes[i] = NULL;
	}
	safeFree(eigenModes); eigenModes = NULL;

	setSolutionAtAllNodes(h_solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary
	setForceVector(h_fSave);// // copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
	computeDeformationMax();
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Completed EigenMode computing");
	safeFree(h_solSave);
	safeFree(h_fSave);
	h_vonmisesMax = maxStressSave;
}
/**********************************************************************/
int TopOpt::solvePrimaryAndAdjointProblems(int *nFEA)
{
    double *h_solSave,*h_fSave,maxStressSave,*h_fBeforeThermal;
    double maxTemperature;
    int node;

     if (ts_topoptMode == TOPOPT_MODE_THERMAL)  {
         removeNonManifoldElements();
         removeDisconnectedElements();
         disableDeflation();
         clearThermalConvectionLoad();
         applyConvectionOnAllExposedNodes();
         for (int node = 0; node < h_nNodes; node++) {
             if (!isNodeOnEvolvingBoundary(node)) {
                 h_f_thermal[node] = 0;// remove load.
                 h_convection[node] = 0;//remove convection coefficient.
             }
         }
         solveThermalProblem(false);// false means don't reset the temperatures to zero ... use previous node temperatures
         return SUCCESS;
     }

	if (!isStaticFEAValid()) {// static results invalid
		h_solSave = (double *)malloc(3*h_nNodes*sizeof(double));// just in case solution fails, store previous solution
		getSolutionVector(h_solSave);// save the primary solution
		initializeSolution();
        if (ts_includeThermalEffects){
            h_fBeforeThermal = (double *)malloc(3*getNumNodes()*sizeof(double));// needed below
            disableDeflation();
            solveThermalProblem(false);

            if (ts_useTemperatureConstraint){
                maxTemperature = getTemperatureMax();
                if (maxTemperature > ts_maxTemperatureLimit){
                    writeMessageToGUIWindow("Temperature limit of %.2f exceeded ",ts_maxTemperatureLimit);
                    if (ts_verbose) writeMessageToParetoLog("Temperature limit exceeded");
                    return FAILURE;
                }

            }
            enableDeflation();
            getForceVector(h_fBeforeThermal);// the elasticity force is saved
            addThermalForce();
        }

		*nFEA +=1;
        if (ts_verbose) cout << "TopOpt: Solving primary " << endl;
		if (!solveElasticityStatic()){
            if (ts_verbose) writeMessageToParetoLog("Primary FEA solving error");
			setSolutionAtAllNodes(h_solSave);
			safeFree(h_solSave);
            if (ts_includeThermalEffects){// bring back the structural force
                setForceVector(h_fBeforeThermal);
                safeFree(h_fBeforeThermal);
            }
			return FAILURE; 
		}
		safeFree(h_solSave);
		ts_compliance= computeCompliance();
		computeDeformationMax();
		postprocess();
        if (ts_includeThermalEffects){// bring back the structural force
            setForceVector(h_fBeforeThermal);
            safeFree(h_fBeforeThermal);
        }
        if (ts_verbose) cout << "TopOpt: Done solving primary " << endl;
	}
	if (ts_topoptQuantity == VONMISES) {
		// assumes that the primary problem has been solved above
		maxStressSave = h_vonmisesMax;
		h_solSave = (double *)malloc(3*h_nNodes*sizeof(double));
		h_fSave = (double *)malloc(3*h_nNodes*sizeof(double));
		getSolutionVector(h_solSave);// save the primary solution
		copy(h_f,h_fSave);// save the primary force
		setVonMisesAdjointForce();
		setForceVector(ts_stressAdjointForce);// don't copy directly since AFElasticity takes care pushing to GPU if necessary
		initializeSolution();
		*nFEA +=1;
        if (ts_verbose) writeMessageToParetoLog("TopOpt: Solving adjoint problem ");
		if (!solveElasticityStatic()){
            writeMessageToParetoLog("\nvon Mises FEA solving Error");
			return FAILURE;  
		}
		// don't post-process for adjoint field, stresses will be messed-up
		getSolutionVector(ts_stressAdjoint);// copy the solution to adjoint
		setSolutionAtAllNodes(h_solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary

		setForceVector(h_fSave);// // copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
        if (ts_verbose) writeMessageToParetoLog("TopOpt: Completed primary and adjoint ");
		safeFree(h_solSave);
		safeFree(h_fSave);
		h_vonmisesMax = maxStressSave;
		//deleteStaticFEAResults();
	}
	else if (ts_topoptQuantity == EIGENVALUE) {
		saveFEAandSolveEigen(nFEA);
	}
	if (ts_topoptQuantity == BUCKLING) {
		//primary problem
		;
	}
	else if (ts_topoptQuantity == OUTPUT_COMPLIANCE) { 
		h_solSave = (double *)malloc(3*h_nNodes*sizeof(double));
		h_fSave = (double *)malloc(3*h_nNodes*sizeof(double));
		getSolutionVector(h_solSave);// save the primary solution
		copy(h_f,h_fSave);// save the primary force
		setOutputAdjointForce();
		setForceVector(ts_stressAdjointForce);// // don't copy directly since AFElasticity takes care pushing to GPU if necessary
		initializeSolution();
		*nFEA +=1;
		if (!solveElasticityStatic()){
			safeFree(h_solSave);
			safeFree(h_fSave);
			return FAILURE;  
		}
		ts_outputcompliance= computeOutputCompliance();
		// don't post-process for adjoint field, stresses will be messed-up
		getSolutionVector(ts_stressAdjoint); // copy the solution to adjoint
		setSolutionAtAllNodes(h_solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary
		setForceVector(h_fSave);// // copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
		computeDeformationMax();
		safeFree(h_solSave);
		safeFree(h_fSave);
	}
	else if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) { 
		int counter1,counter2;

		h_solSave = (double *)malloc(3*h_nNodes*sizeof(double));
		h_fSave = (double *)malloc(3*h_nNodes*sizeof(double));
		getSolutionVector(h_solSave);// save the primary solution
		copy(h_f,h_fSave);// save the primary force
		ts_inputDisplacement = 0;
		counter1= 0;
		counter2= 0;
		ts_outputDisplacement= 0;
		for (int node =0; node < h_nNodes;node++){
			if (isThereForceOnNode(node)){
				ts_inputDisplacement+= h_solSave[3*node]*h_f[3*node] + h_solSave[3*node+1]*h_f[3*node+1] + h_solSave[3*node+2]*h_f[3*node+2];
				counter1++;
			}
			if (isNodeOutput(node)){
				ts_outputDisplacement += h_solSave[3*node]*h_output[3*node] + h_solSave[3*node+1]*h_output[3*node+1] + h_solSave[3*node+2]*h_output[3*node+2];
				counter2++;
			}
		}
		ts_inputDisplacement /=counter1;
		ts_outputDisplacement /=counter2;
		ts_geometricAdvantage = ts_outputDisplacement/ts_inputDisplacement;//for a given input force, output displacement/input displacement

		setOutputAdjointForce();
		setForceVector(ts_stressAdjointForce);//  // don't copy directly since AFElasticity takes care pushing to GPU if necessary
		initializeSolution();
		*nFEA +=1;
        if (ts_verbose) writeMessageToParetoLog("\nTopOpt: Solving mutual ");
		if (!solveElasticityStatic()){
            if (ts_verbose) writeMessageToParetoLog("\nMutual compliance FEA solving Error");
			setSolutionAtAllNodes(h_solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary
			setForceVector(h_fSave);// // copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
			computeDeformationMax();
			safeFree(h_solSave);
			safeFree(h_fSave);
			return FAILURE;  
		}
		getSolutionVector(ts_stressAdjoint);//  // copy the solution to adjoint
		ts_outputcompliance = computeOutputCompliance();
		// don't post-process for adjoint field, stresses will be messed-up	
		setSolutionAtAllNodes(h_solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary
		setForceVector(h_fSave);// // copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
		computeDeformationMax();
		safeFree(h_solSave);
		safeFree(h_fSave);
	}
	return SUCCESS;
}
/**********************************************************************/
void  TopOpt::imposeCyclicSymmetry(int axiSymmetry)
{
	// axiSymmetry = 2, 180 deg (360/2)
	// axiSymmetry = 3, 120 deg (360/3)
	// axiSymmetry = 4, 90 deg
	//....

	// assume it z axis and origin.
	int node;
	double relativeError, sectorAngle,maxRadius,pi,x,y,center[3],boundingBox[3][2];
	double tsNode, tsNodeMax,tsNodeMin,tsNodeAvg,dist,deltaAngle;
	bool *nodeVisited;
	IntegerSet nodeSet;
	pi = 3.14159265359;
	// assumes z axis for axi symmetry
    if (axiSymmetry < 2) return;
    sectorAngle = 2*pi/axiSymmetry;

	getBoundingBox(boundingBox);
	maxRadius = (boundingBox[0][1]-boundingBox[0][0])/2;

	center[0] = (boundingBox[0][1]+boundingBox[0][0])/2;
	center[1] = (boundingBox[1][1]+boundingBox[1][0])/2;
	center[2] = (boundingBox[2][1]+boundingBox[2][0])/2;
	nodeVisited = (bool *)malloc(h_nNodes*sizeof(bool));
	memset(nodeVisited,false,h_nNodes*sizeof(bool));

	for (double r = h_dx; r <= maxRadius; r += h_dx){// walk along the radial line in discrete steps
		// walk along the circle in steps of specified angle, and pick up closest nodes
		deltaAngle = 0.5*h_dx/r;// in radians
		for (double startAngle = 0; startAngle < sectorAngle; startAngle += deltaAngle){// different start position
            // take the average of the TS values of all these node and assign to each of them
			nodeSet.clear();
			for (double z = boundingBox[2][0]-0.1*h_dz; z <= boundingBox[2][1]+0.1*h_dz; z += h_dz){// walk through every node layer in z
                for (double theta = startAngle; theta < 2*pi;  theta += sectorAngle) {// pick up all nodes that are apart by specified angle
					x = center[0] + r*cos(theta);
					y = center[1] + r*sin(theta);
					node = getClosestNode(x,y,z );
					
					if (node < 0) continue;
					dist = sqrt ( pow(h_x[node]-x,2) + pow(h_y[node]-y,2) + pow(h_z[node]-z,2));
					if (dist > 0.75*h_dx) continue;
					if (nodeVisited[node]) continue;// from a different 
					nodeSet.insertCheckForUnique(node);
				}
			}
			//if ( nodeSet.size() != axiSymmetry) continue;// should be the same
			// now make all TS equal
			tsNodeMax = -HUGE_VAL;
			tsNodeMin = HUGE_VAL;
			tsNodeAvg = 0;

			for (int i = 0; i < nodeSet.size(); i++){
				node = nodeSet.elementAt(i);
				nodeVisited[node] = true;
				tsNode = ts_topSensNode[node];
				tsNodeMin = MIN(tsNodeMin,tsNode);
				tsNodeMax = MAX(tsNodeMax,tsNode);
				tsNodeAvg += tsNode;
			}

			tsNodeAvg /=  nodeSet.size();
			relativeError = (tsNodeMax -tsNodeMin)/(tsNodeMax + tsNodeMin)/2;
			//if (relativeError > 0.1) continue;
			for (int i = 0; i < nodeSet.size(); i++)
				ts_topSensNode[nodeSet.elementAt(i)] = tsNodeAvg;		
		}
	}
	free(nodeVisited);
}
/**********************************************************************/
void  TopOpt::imposeGridPattern(int gridPattern[3])
{
    // impose a grid pattern in 3 directions
    // Example: if xGrid = 2, then the domain is divided into 2 pieces along X,
    // Then the TS sensitivity is forced to be identical in both pieces
    // A bounding box is used to split the domain into (xGrid,yGrid,zGrid) pieces

    int otherGridNode,otherNode,gridnode,node,nodes[100],nNodes,nxBar,nyBar,nzBar;
    int xBlockSize, yBlockSize, zBlockSize;
    int xGrid,yGrid,zGrid;
    double value;

    xGrid = max(1,gridPattern[0]);
    yGrid = max(1,gridPattern[1]);
    zGrid = max(1,gridPattern[2]);

    if (max(max(xGrid,yGrid),zGrid) == 1) return;// nothing to impose
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Imposing grid pattern (%d, %d, %d) ",gridPattern[0],gridPattern[1],gridPattern[2]);
    xBlockSize = ceil(1.0*h_nelx/xGrid);
    yBlockSize = ceil(1.0*h_nely/yGrid);
    zBlockSize = ceil(1.0*h_nelz/zGrid);
    for (int nx = 0; nx < xBlockSize; nx++){// walk thru the first block
        for (int ny = 0; ny < yBlockSize; ny++){
            for (int nz = 0; nz < zBlockSize; nz++){
                nNodes = 0;
                gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                node = h_gridNodeToRealNode[gridnode];
                if ((node < 0) || (node >= h_nNodes))continue;
                nodes[nNodes] = node;//keep track of all nodes with identical TS
                nNodes++;
                for (int ix = 1; ix <= xGrid; ix++){// collect node from all other blocks
                    nxBar = nx + (ix-1)*xBlockSize;
                    for (int iy = 1; iy <= yGrid; iy++){
                        nyBar = ny + (iy-1)*yBlockSize;
                        for (int iz = 1; iz <= zGrid; iz++){
                            nzBar = nz + (iz-1)*zBlockSize;
                            otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
                            if (otherGridNode >= (h_nelz+1)*(h_nely+1)*(h_nelx+1)) continue;
                            otherNode = h_gridNodeToRealNode[otherGridNode];                     
                            if ((otherNode < 0) || (otherNode == node) || (otherNode >= h_nNodes))continue;
                            nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                            nNodes++;
                        }
                    }
                }
                if (nNodes == 1) continue;// no other node
                value = 0;
                for (int i = 0; i < nNodes;i++)
                    value +=ts_topSensNode[nodes[i]];
                value /= nNodes;
                for (int i = 0; i < nNodes;i++)
                     ts_topSensNode[nodes[i]] = value;

            }
        }
    }
}
/**********************************************************************/
void  TopOpt::imposeSymmetry(int symmetryIndex)
/*symmetryIndex <= 0, means no symmetry
	 1 = x-symmetry
	 2 = y-symmetry
	 3 = x & y symmetry
	 4 = z-symmetry
	 5 = x & z symmetry
	 6 = y & z symmetry
	 7 = x, y & z symmetry*/
{
    int i, nx,ny,nz,gridnode,otherGridNode,nxBar,nyBar,nzBar;
    int node,otherNode,nNodes,nodes[100];
    double value,maxValue,minValue,relativeError;
    bool symmetryErrorReported =false;

    if (symmetryIndex <= 0) return;
    //if (getNumberOfParts() > 1) return;//bug??
    symmetryErrorReported = false;
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Imposing symmetry along %d", symmetryIndex);
    for (nx = 0; nx <= h_nelx; nx++){
        for (ny = 0; ny <= h_nely; ny++){
            for (nz = 0; nz <= h_nelz; nz++){
                nNodes = 0;
                gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                node = h_gridNodeToRealNode[gridnode];
                if (node < 0) continue;
                nodes[nNodes] = node;//keep track of all nodes of symmetry
                nNodes++;
                if (symmetryIndex == 1) {// x symmetry
                    nxBar = h_nelx - nx;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 2) {
                    nyBar =h_nely - ny;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 3) { //x and y symmetry
                    nxBar = h_nelx - nx;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    nyBar =h_nely - ny;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 4) {// z symmetrt
                    nzBar = h_nelz - nz;
                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 5) {// z and x
                    nxBar = h_nelx - nx;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    nzBar = h_nelz - nz;
                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 6) {// y and z
                    nyBar =h_nely - ny;
                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    nzBar = h_nelz - nz;
                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }
                }
                else if (symmetryIndex == 7) {// x, y and z
                    nxBar = h_nelx - nx;
                    nyBar = h_nely - ny;
                    nzBar = h_nelz - nz;

                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];

                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nz*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nx;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                    otherGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
                    otherNode = h_gridNodeToRealNode[otherGridNode];
                    if (otherNode > 0) {
                        nodes[nNodes] = otherNode;//keep track of all nodes of symmetry
                        nNodes++;
                    }

                }
                if (nNodes == 1) continue;
                value = 0;
                maxValue = -HUGE_VAL;
                minValue = HUGE_VAL;
                for (i = 0; i < nNodes;i++) {
                    value +=ts_topSensNode[nodes[i]];
                    maxValue = max(maxValue,ts_topSensNode[nodes[i]]);
                    minValue = min(minValue,ts_topSensNode[nodes[i]]);
                }
                value /= nNodes;
                relativeError = (maxValue-minValue)/(ts_TSMax);
                if ((relativeError > 0.2) && (!symmetryErrorReported)){
                    symmetryErrorReported = true;
                    //sprintf(ts_lastTopOptErrorMessage, "TopOpt: Topology does not appear to be symmetric ");
                    //writeMessageToParetoLog("%s ", ts_lastTopOptErrorMessage);
                }
                for (i = 0; i < nNodes;i++)
                   ts_topSensNode[nodes[i]] = value;
            }
        }
    }
}
/**********************************************************************/
void  TopOpt::imposeExtrude(int dir)
// Ensure that the topology can be laset-cut in the given direction
// dir = +/-1,+/- 2,+/- 3 (x,y,z). 
{
	int *nodes,nNodesAlongDir,i;
	int maxN = MAX(MAX(h_nelx+1,h_nely+1),h_nelz+1);
    double minVal = HUGE_VAL;
    double maxVal = -HUGE_VAL;
    double avgVal = 0.0;

	if  ((dir < 1) || (dir >3)) return;

    if (ts_verbose) writeMessageToParetoLog("TopOpt: Imposing through cut along %d", dir);
	nodes = (int *)malloc(maxN*sizeof(int));
	int nx,ny,nz;
	bool removeAllNodesInDir = false;
	bool keepAllNodesInDir = false;
	double cutoff;

	cutoff = getCurrentTSCutoff() ;//- 0.001*(ts_TSMax - ts_TSMin);
	// depending on the direction, sweep through appropriate plane
	for (nx = 0; nx <= h_nelx*(dir !=1); nx++){// if dir == 1, then nx goes from 0 to 0
        for (ny = 0; ny <= h_nely*(dir !=2); ny++){// if dir == 2, then ny goes from 0 to 0
            for (nz = 0; nz <= h_nelz*(dir !=3); nz++){// if dir == 3, then nz goes from 0 to 0
				nNodesAlongDir = findNodesAlongDirection(nx,ny,nz,dir,nodes);
				// find the location and value of the lowest and max TS value
				if (nNodesAlongDir == 0) continue;
                minVal = HUGE_VAL;
                maxVal = -HUGE_VAL;
                keepAllNodesInDir = false;
                removeAllNodesInDir = false;
                avgVal = 0.0;
                for (i = 0; i < nNodesAlongDir; i++){
                    maxVal = MAX(maxVal,ts_topSensNode[nodes[i]]);
                    minVal = MIN(minVal,ts_topSensNode[nodes[i]]);
					avgVal += ts_topSensNode[nodes[i]];
                }
				avgVal /= double(nNodesAlongDir);

				// if must be retained
				for (i = 0; i < nNodesAlongDir; i++){
					if ( !(isNodeFree(nodes[i])) || (isNodeRetained(nodes[i]))) {// don't remove along this ray
						keepAllNodesInDir = true;
						break;
					}
				}
				if (keepAllNodesInDir) {
                    for (i = 0; i < nNodesAlongDir; i++)
						ts_topSensNode[nodes[i]] = maxVal;
					continue;
				}

				// prefer to keep
				keepAllNodesInDir = (avgVal > cutoff) && (minVal < cutoff);
				if (keepAllNodesInDir) {
                    for (i = 0; i < nNodesAlongDir; i++)
                        ts_topSensNode[nodes[i]] = MAX(ts_topSensNode[nodes[i]],1.1*cutoff);
					continue;
				}

				// prefer to remove
                removeAllNodesInDir = (avgVal <= cutoff) && (maxVal >= cutoff);
				if (removeAllNodesInDir){
                    for (i = 0; i < nNodesAlongDir; i++)
                        ts_topSensNode[nodes[i]] = MIN(ts_topSensNode[nodes[i]],0.9*cutoff);
					continue;
				}
			}
		}
	}
	free(nodes);
}
/**********************************************************************/
void  TopOpt::imposeMaxFeatureConstraint(void)
{
    double *nodeDistanceToBoundary,cutoff;
    if (ts_maxFeatureSize < 0) return;

    if (ts_verbose) writeMessageToParetoLog("TopOpt: Imposing max distance constraint of %e",ts_maxFeatureSize);
    nodeDistanceToBoundary = (double *)malloc(getNumNodes()*sizeof(double));
    computeDistanceOfNodesToEvolvingBoundary(nodeDistanceToBoundary);

    cutoff = getCurrentTSCutoff();
    int counter = 0;
    for (int node = 0; node < h_nNodes; node++){
        double eta;
        eta = nodeDistanceToBoundary[node]/ts_maxFeatureSize;
        if (eta < 1) continue;// node outside the topology
        ts_topSensNode[node] *= exp(10*(1-eta));
        counter++;
    }
    cout << "imposing MaxFeatureConstraint on " << counter << " nodes " <<endl;
    free(nodeDistanceToBoundary);
}

/**********************************************************************/
void  TopOpt::imposeExtrudeOnElements(int dir)
// Ensure that the topology can be laser-cut in the given direction
// dir = +/-1,+/- 2,+/- 3 (x,y,z).
{
    int *elements,nElementsAlongDir,i;
    int maxN = MAX(MAX(h_nelx+1,h_nely+1),h_nelz+1);
    double minVal = HUGE_VAL,maxVal = -HUGE_VAL,averageVal;

    if  ((dir < 1) || (dir >3)) return;
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Imposing through cut along %d using elements", dir);
    elements = (int *)malloc(maxN*sizeof(int));
    int nx,ny,nz;

    double cutOff = getCurrentTSCutoff() ;
    bool keepAll = false;
    // depending on the direction, sweep through appropriate plane
    for (nx = 0; nx <= (h_nelx-1)*(dir !=1); nx++){// if dir == 1, then nx goes from 0 to 0
        for (ny = 0; ny <= (h_nely-1)*(dir !=2); ny++){// if dir == 2, then ny goes from 0 to 0
            for (nz = 0; nz <= (h_nelz-1)*(dir !=3); nz++){// if dir == 3, then nz goes from 0 to 0
                nElementsAlongDir = findElementsAlongDirection(nx,ny,nz,dir,elements);
                if (nElementsAlongDir == 0) continue;
                minVal = HUGE_VAL;
                maxVal = -HUGE_VAL;
                averageVal = 0;
                keepAll = false;
                for (i = 0; i < nElementsAlongDir; i++){
                    minVal = MIN(minVal,ts_topSensElem[elements[i]]);
                    minVal = MIN(minVal,ts_topSensElem[elements[i]]);
                    averageVal += ts_topSensElem[elements[i]];
                }

                averageVal /= (1.0*nElementsAlongDir);
                if (averageVal < cutOff) {
                    for (i = 0; i < nElementsAlongDir; i++)
                        if (!h_keepElem[elements[i]])
                            ts_topSensElem[elements[i]] = averageVal;
                }
                else {
                    for (i = 0; i < nElementsAlongDir; i++)
                        ts_topSensElem[elements[i]] = averageVal;
                }
            }
        }
    }
    free(elements);
}
/**********************************************************************/
void  TopOpt::imposeDrawDirection(int dir)
// Ensure that the topology can be 'drawn' in the given direction
// dir = 1,2,3 (x,y,z).
{
	int *nodes,nNodesAlongDir,i;
	int maxN = MAX(MAX(h_nelx+1,h_nely+1),h_nelz+1),minNodeLoc,maxNodeLoc;
	double ts_TSMinAlongDir,ts_TSMaxAlongDir,TSCurrent;

	if  ((dir < 1) || (dir >3)) return;

	if (ts_verbose) cout << "TopOpt: Imposing draw direction along " << dir << endl;
	nodes = (int *)malloc(maxN*sizeof(int));
	int nx,ny,nz;
	// depending on the direction, sweep through appropriate plane
	for (nx = 0; nx <= h_nelx; nx++){
		for (ny = 0; ny <= h_nely; ny++){
			for (nz = 0; nz <= h_nelz; nz++){
				nNodesAlongDir = findNodesAlongDirection(nx,ny,nz,dir,nodes);
				// find the location and value of the lowest and max TS value
				ts_TSMinAlongDir = 1e12;
				ts_TSMaxAlongDir = -1e12;
				if (nNodesAlongDir == 0)
					continue;
				for (i = 0; i < nNodesAlongDir; i++){
					if (ts_topSensNode[nodes[i]] < ts_TSMinAlongDir){
						ts_TSMinAlongDir = ts_topSensNode[nodes[i]];
						minNodeLoc = i;
					}
					if (ts_topSensNode[nodes[i]] > ts_TSMaxAlongDir){
						ts_TSMaxAlongDir = ts_topSensNode[nodes[i]];
						maxNodeLoc = i;
					}
				}
				// now depending on the location 
				TSCurrent = 0;
				for (i = 0; i < maxNodeLoc; i++){
					if (ts_topSensNode[nodes[i]] < TSCurrent){
						ts_topSensNode[nodes[i]] = TSCurrent;
						minNodeLoc = i;
					}
					else
						TSCurrent = ts_topSensNode[nodes[i]];
				}
				TSCurrent = 0;
				for (i =nNodesAlongDir-1; i > maxNodeLoc; i--){
					if (ts_topSensNode[nodes[i]] < TSCurrent){
						ts_topSensNode[nodes[i]] = TSCurrent;
						minNodeLoc = i;
					}
					else
						TSCurrent = ts_topSensNode[nodes[i]];
				}
			}
		}
	}
	free(nodes);
}
/**********************************************************************/
void  TopOpt::imposeSupportStructureConstraintOnNodes(void)
// Ensure that the topology can be AM built in the given direction, without support structures
// in x,y or z direction
{
    int xOffset,yOffset,zOffset,gridnode,nSupportNodes,elemCutoff;
	int gridNodesBelow[5],gridNodesAbove[5];
    double eta,isoCutoff;

	if (ts_AMBuildDirection == 0) return;
	xOffset = 1;
	yOffset = (h_nelx+1);
	zOffset = (h_nely+1)*(h_nelx+1);

    nSupportNodes = 5;

    /*
    eta = 1.0;
    std::vector<double> myvector(ts_topSensElem, ts_topSensElem+h_nElems);// the entire vector is used
    std::sort(myvector.begin(), myvector.end());
    elemCutoff = NEARESTINT(eta*h_nElems);
    isoCutoff = myvector[elemCutoff];
    vector<double>().swap(myvector);//release
    */
    isoCutoff  = ts_TSMax;
    int nIterations = 1;
    for (int iter = 1; iter <= nIterations; iter++){
		switch (ts_AMBuildDirection) {
		case 1: //+x
			gridNodesBelow[0] = -xOffset;
			gridNodesBelow[1] = -xOffset-yOffset;
			gridNodesBelow[2] = -xOffset+yOffset;
			gridNodesBelow[3] = -xOffset-zOffset;
			gridNodesBelow[4] = -xOffset+zOffset;
            for (int nx = 1; nx <= h_nelx; nx++){
				for (int ny = 0; ny <= h_nely; ny++){
					for (int nz = 0; nz <= h_nelz; nz++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure( gridnode,nSupportNodes,gridNodesBelow,isoCutoff);
					}
				}
			}

			break;
		case -1: //-x
            gridNodesAbove[0] = xOffset;
            gridNodesAbove[1] = xOffset-yOffset;
            gridNodesAbove[2] = xOffset+yOffset;
            gridNodesAbove[3] = xOffset-zOffset;
            gridNodesAbove[4] = xOffset+zOffset;


            for (int nx = h_nelx-1; nx >=0; nx--){// start from the bottom
				for (int ny = 0; ny <= h_nely; ny++){
					for (int nz = 0; nz <= h_nelz; nz++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure( gridnode,nSupportNodes,gridNodesAbove,eta);
					}
				}
			}

			break;
		case 2: //+y
			gridNodesBelow[0] = -yOffset;
			gridNodesBelow[1] = -yOffset-xOffset;
			gridNodesBelow[2] = -yOffset+xOffset;
			gridNodesBelow[3] = -yOffset-zOffset;
			gridNodesBelow[4] = -yOffset+zOffset;

            for (int ny = 1; ny <= h_nely; ny++){
				for (int nx = 0; nx <= h_nelx; nx++){
					for (int nz = 0; nz <= h_nelz; nz++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure(gridnode,nSupportNodes,gridNodesBelow,isoCutoff);
					}
				}
			}
			
			break;
		case -2: //-y
            gridNodesAbove[0] = yOffset;
            gridNodesAbove[1] = yOffset-xOffset;
            gridNodesAbove[2] = yOffset+xOffset;
            gridNodesAbove[3] = yOffset-zOffset;
            gridNodesAbove[4] = yOffset+zOffset;

            for (int ny =h_nely-1; ny >=0 ; ny--){
				for (int nx = 0; nx <= h_nelx; nx++){
					for (int nz = 0; nz <= h_nelz; nz++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure( gridnode,nSupportNodes,gridNodesAbove,isoCutoff);
					}
				}
			}

			break;
		case 3: //+z
			gridNodesBelow[0] = -zOffset;
			gridNodesBelow[1] = -zOffset-xOffset;
			gridNodesBelow[2] = -zOffset+xOffset;
			gridNodesBelow[3] = -zOffset-yOffset;
			gridNodesBelow[4] = -zOffset+yOffset;

            for (int nz = 1; nz <= h_nelz; nz++){// start from the top layer
				for (int nx = 0; nx <= h_nelx; nx++){
					for (int ny = 0; ny <= h_nely; ny++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure(gridnode,nSupportNodes,gridNodesBelow,eta);
					}
				}
			}

			break;
		case -3: //-z
            gridNodesAbove[0] = zOffset;
            gridNodesAbove[1] = zOffset-xOffset;
            gridNodesAbove[2] = zOffset+xOffset;
            gridNodesAbove[3] = zOffset-yOffset;
            gridNodesAbove[4] = zOffset+yOffset;

            for (int nz = h_nelz-1; nz >=0; nz--){
				for (int nx = 0; nx <= h_nelx; nx++){
					for (int ny = 0; ny <= h_nely; ny++){	
						gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
                        reduceGridNodeTSForSupportStructure( gridnode,nSupportNodes,gridNodesAbove,isoCutoff);
					}
				}
			}
			break;
		}
	}
}
/**********************************************************************/
void  TopOpt:: reduceGridNodeTSForSupportStructure(int gridnode,int nSupportNodes,int supportGridNodes[],double isoCutoff)
    // Ensure that the topology can be AM built in the given direction, without support structures
{
    int node,otherGridNode,otherNode,totalGridNodes;
    double tsmax,tsOtherNode,tsCutoff;

    totalGridNodes = (h_nelx+1)*(h_nely+1)*(h_nelz+1);
    node = h_gridNodeToRealNode[gridnode];
    if (!doesNodeExist(node)) return;


    if (ts_topSensNode[node] > isoCutoff) return;// node is important, do not change.

    tsmax = ts_TSMin;
    for (int i= 0; i < nSupportNodes; i++){
        otherGridNode = gridnode + supportGridNodes[i];
        if ((otherGridNode < 0) || (otherGridNode >= totalGridNodes)) continue;
        otherNode = h_gridNodeToRealNode[otherGridNode];
        if (otherNode < 0) continue;
        if (!doesNodeExist(node))
            tsOtherNode = ts_TSMin;
        else
            tsOtherNode = ts_topSensNode[otherNode];
        tsmax = MAX(tsmax,tsOtherNode);
    }
    // cap the TS value by the TS values at the support nodes
    ts_topSensNode[node] = MIN(ts_topSensNode[node],tsmax);

}

/**********************************************************************/
/*void  TopOpt:: reduceGridNodeTSForSupportStructure(int gridnode,int nSupportNodes,int supportGridNodes[],double eta)
	// Ensure that the topology can be AM built in the given direction, without support structures
{
	int node,otherGridNode,otherNode,totalGridNodes;
	double tsmax,tsOtherNode;

	totalGridNodes = (h_nelx+1)*(h_nely+1)*(h_nelz+1);
	node = h_gridNodeToRealNode[gridnode];
	if (!doesNodeExist(node)) return;

	tsmax = ts_TSMin;
	for (int i= 0; i < nSupportNodes; i++){
		otherGridNode = gridnode + supportGridNodes[i];
		if ((otherGridNode < 0) || (otherGridNode >= totalGridNodes)) continue;
		otherNode = h_gridNodeToRealNode[otherGridNode];
		if (otherNode < 0) continue;
		if (!doesNodeExist(node))
			tsOtherNode = ts_TSMin;
		else
			tsOtherNode = ts_topSensNode[otherNode];
		tsmax = MAX(tsmax,tsOtherNode);
	}
    // cap the TS value by the TS values at the support nodes
	ts_topSensNode[node] = (1-eta)*ts_topSensNode[node] + eta*MIN(ts_topSensNode[node],tsmax);

}*/
/**********************************************************************/
void  TopOpt:: increaseGridNodeTSForSupportStructure(int gridnode,int nSupportNodes,int supportGridNodes[],double eta)
	// Ensure that the topology can be AM built in the given direction, without support structures
{
	int node,otherGridNode,otherNode,totalGridNodes;
	double tsmin,tsOtherNode;

	totalGridNodes = (h_nelx+1)*(h_nely+1)*(h_nelz+1);
	node = h_gridNodeToRealNode[gridnode];
	if (!doesNodeExist(node)) return;

	tsmin = ts_TSMax;
	for (int i= 0; i < nSupportNodes; i++){
		otherGridNode = gridnode + supportGridNodes[i];
		if ((otherGridNode < 0) || (otherGridNode >= totalGridNodes)) continue;
		otherNode = h_gridNodeToRealNode[otherGridNode];
		if (otherNode < 0) continue;
		if (!doesNodeExist(node))
			tsOtherNode = ts_TSMin;
		else
			tsOtherNode = ts_topSensNode[otherNode];
		tsmin = MIN(tsmin,tsOtherNode);
	}
	ts_topSensNode[node] = (1-eta)*ts_topSensNode[node] + eta*MAX(ts_topSensNode[node],tsmin);

}
/**********************************************************************/
void  TopOpt::imposeSupportStructureConstraintOnElems(int dir)
// Ensure that the topology can be AM built in the given direction, without support structures
// dir = 1, 2, 3(x,y,z). 
{
	double boundingBox[3][2],layerBoundingBox[3][2],tsmax,startRange,endRange,delta;
	int *elemsInLayer,nElems,elem,neighborElems[27],neighborElem,absDir;
	int *supportElements;

	int xMinusSupport[9] = {1,7,8,10,15,16,23,24,25};// the 9 elements in the minus X direction, among 27 neighbors. See neighboringElemsOfElem
	int xPlusSupport[9] = {4,11,12,14,18,19,21,22,26};// the 9 elements in the plus X direction
	int yMinusSupport[9] = {2,7,9,10,17,18,22,24,26};
	//int yMinusSupport[5] = {2,7,9,17,18};
	int yPlusSupport[9] = {5,11,13,14,15,20,21,23,26};
	int zMinusSupport[9] = {3,8,9,10,19,20,21,25,26};
	int zPlusSupport[9] = {6,12,13,14,16,17,22,23,24};

	if  ((dir == 0) || (dir < -3) || (dir >3)) return;// plus and minus allowed

	absDir = (dir < 0)? -dir:dir;

	switch (dir) {
	case (-1): supportElements = xMinusSupport; break;
	case (1): supportElements = xPlusSupport; break;
	case (-2): supportElements = yPlusSupport; break;
	case (2): supportElements = yMinusSupport; break;
	case (-3): supportElements = zMinusSupport; break;
	case (3): supportElements = zPlusSupport; break;
	}
	elemsInLayer = (int *)malloc(h_nElems*sizeof(int));
	// assume z direction
	// start with the lowest layer and work up
	getBoundingBox(boundingBox);
	getBoundingBox(layerBoundingBox);// copy over

	if (absDir == 1)
		delta = h_dx;
	else if (absDir == 2)
		delta = h_dy;
	else
		delta = h_dz;

	if (dir > 0) {
		startRange = boundingBox[absDir-1][0]+delta;//start with the 2nd layer
		endRange = boundingBox[absDir-1][1];
	}
	else {
		startRange = boundingBox[absDir-1][1]-delta;
		endRange = boundingBox[absDir-1][0];
	}
	for (double value = startRange; value < endRange; value += delta) {	
		layerBoundingBox[absDir-1][0] = value;//change the box dimensions in the appropriate direction
		layerBoundingBox[absDir-1][1] = value+delta;
		nElems = findElemsInBoundingBox(layerBoundingBox,elemsInLayer);
		//  for each element impose the AM filter
		for (int i = 0; i < nElems; i++){// for all elements in layer	
			elem = elemsInLayer[i];
			if (elem == -1) continue;
			if (getElemPseudoDensity(elem) < 0.5) continue;
			getAllNeighboringElementsOfElem(elem,neighborElems);
			tsmax = ts_TSMin;
			for (int j = 0; j < 9; j++) {
				neighborElem = neighborElems[supportElements[j]];
				if (neighborElem == -1) continue;
				tsmax = MAX(tsmax,ts_topSensElem[neighborElem]);
			}
			ts_topSensElem[elem] = MIN(ts_topSensElem[elem],tsmax);
		}
	}
	
	free(elemsInLayer);
}
/**********************************************************************/
int TopOpt::checkConstraints(void)
{
	int partId;
	char msg[256];

	if (getDeformationMax() > ts_maxDisplacementAllowed){
        sprintf(msg,"Displacement limit of %.2e reached",ts_maxDisplacementAllowed);
		strcpy(ts_lastTopOptErrorMessage,msg);
        writeMessageToParetoLog("\n %s ",msg);
		return CONSTRAINT_FAILURE;
	}
	// check for stress constraint via yield
	if (isDesignPastYield(&partId,ts_stressSafetyFactor)){// also check for yield
        sprintf(msg,"Part %d is approaching stress limit", partId);
		strcpy(ts_lastTopOptErrorMessage,msg);
        writeMessageToParetoLog("\n %s ",msg);
		return CONSTRAINT_FAILURE;
	} 
	if ((ts_useEigenConstraint) && (h_eigenValues != NULL)) {
		double currentEigenHz;
		currentEigenHz = h_eigenValues[0];
		if (currentEigenHz < ts_minEigenValueHz) {
            sprintf(msg, "Eigen value is approaching lower limit  (hz) %.1f", ts_minEigenValueHz);
			strcpy(ts_lastTopOptErrorMessage, msg);
            writeMessageToParetoLog("\n %s ", msg);
			return CONSTRAINT_FAILURE;
		}
	}
	if (!userConstraintFunction()){//user defined virtual function
		strcpy(ts_lastTopOptErrorMessage,"User defined constraint failed");
		return CONSTRAINT_FAILURE;
	}
	return SUCCESS;
}

/**********************************************************************/
bool TopOpt::isIsoSurfaceDisconnected(void)
{
	// if an element isovalue is above threshold (inside), but all 8 nodal values are below threshold, then the isosurface is d
	int nodes[8];
	bool allNodesOnBoundary;

	for (int elem = 0; elem < h_nElems; elem++){
		if (getElemPseudoDensity(elem) < 0.5) continue;
		allNodesOnBoundary = true;
		getElemNodes(elem,nodes);
		for (int m = 0; m < 8; m++)
			if (isNodeInInterior(nodes[m]))
				allNodesOnBoundary = false;
		if (allNodesOnBoundary)
			return true;
	}
	return false;
}

/**********************************************************************/
int  TopOpt::IterateToVolume(double desiredVolumeFraction, double *JConverged,
							 int *nFEA, double *isoValueConverged, bool(*callBackFunction)(bool saveIsoSurface))
// Fixed-point iteration to desired volume fraction
//New version
{
	double isoValue,JTemp,JMin;
	int nIter,nElemsToRemove;

    if (ts_verbose) writeMessageToParetoLog("Topopt: Iterating to volume; starting J = %e \n",*JConverged);
	nIter = 0;
	isoValue = 0;
	
	nElemsToRemove = (int) ceil((1.0 -desiredVolumeFraction)*h_nElems +1);
	while (nIter < 	ts_max_topoptIterations) {// to avoid cycles
		// assume the fea solution is valid at this point
		computeTopSensAtElems();
		TSSmoothener();
		if ( (computeIsovalue(desiredVolumeFraction,&isoValue,&nElemsToRemove) == FAILURE)){	
			strcpy(ts_lastTopOptErrorMessage,"Could not extract iso value in TopOpt; check loading");
            writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
			return ITER_TO_VOL_ISOVALUE_FAILURE;
		}	
		setElementToVoid(ts_restartTopology,ts_topSensElem,isoValue,nElemsToRemove);
        if ((ts_keepTopologyConnected) && (!isEvolvingMeshConnected())){
            strcpy(ts_lastTopOptErrorMessage,"Topology separation; design is possibly over constrained");
            writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
            return MESH_DISCONNECTED_FAILURE;
        }

		if (!solvePrimaryAndAdjointProblems(nFEA)) {// could not solve problem!!!
            if (ts_verbose) writeMessageToParetoLog("FEA Failure");
			strcpy(ts_lastTopOptErrorMessage,"Could not solve FEA ... possible topology separation; try increasing mesh quality");
            writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
			//if (ts_verbose)  dumpVoxelFileBinary("disconnectedMesh.vox"); for debugging
			return ITER_TO_VOL_FEA_FAILURE;
		}
		if (!callBackFunction(false)) return USER_REQUEST_STOP;
		JTemp = computeJ();	
		if (nIter == 0) JMin = HUGE_VAL;
		else JMin = min(JTemp,JMin);
        if (ts_verbose) writeMessageToParetoLog("TopOpt: iter %d;  JTemp = %e, J/J0= %0.2f",nIter,JTemp,JTemp/(*JConverged));
		if ( (fabs(JTemp/(*JConverged)) > ts_relativeJConvergence) || (fabs(JTemp/(*JConverged)) < 1./ts_relativeJConvergence) ){
			strcpy(ts_lastTopOptErrorMessage,"Large increase in compliance ... possible topology separation; try increasing mesh quality");
            writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
			return DIVERGENCE_FAILURE;
        }

        if ((nIter >= ts_min_topoptIterations)  && (JTemp <= (1+ts_relativeJTolerance)*JMin) ){//relax the constraint with every iteration
            *isoValueConverged = isoValue;
            *JConverged = JTemp;
            return ITER_TO_VOL_SUCCESS;;//success
        }

        nIter++;
	}
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Did not converge in Iterate to Volume");
	return ITERATION_ERROR;
}
/**********************************************************************/
double TopOpt::TraceParetoCurve(double desiredVolFrac,double volDecrementMax, int *nFEA, 
							  double *isoValueConverged, bool (*callBackFunction)(bool saveIsoSurface),
							  double *volDecrementStart,double allocatedTimeInSecs)
{
	double J,JTemp,isoTemp,currentVolfrac,convergedVolFraction,S0,C0,S,C;
	int flag,nElemsToRemove,nAttempts;
	bool topologySaved;
	double tempVolDecrement;
		

	if (desiredVolFrac < 0.99*ts_minAllowableVolumeFraction) {
        writeMessageToParetoLog("TopOpt: Desired volume fraction %.3f is less than allowable %.3f",desiredVolFrac,ts_minAllowableVolumeFraction);
		desiredVolFrac = ts_minAllowableVolumeFraction;
	}
	if (ts_topSensElem == NULL) {
        writeMessageToParetoLog("TopOpt: Initialize Topopt!");
		return 1.0;
    }


    writeMessageToGUIWindow("------------TopOpt Constraints ----------");
    writeMessageToGUIWindow("TopOpt: Mode: %d", ts_topoptMode);
    writeMessageToGUIWindow("TopOpt: Symmetry index: %d", ts_symmetryIndex);
    writeMessageToGUIWindow("TopOpt: Through-cut: %d", ts_extrudeDirection);
    writeMessageToGUIWindow("TopOpt: AM Build: %d", ts_AMBuildDirection);
    writeMessageToGUIWindow("TopOpt: Draw-direction: %d", ts_drawDirection);
    writeMessageToGUIWindow("TopOpt: Cyclic-symmetry: %d", ts_axiSymmetryIndex);
    writeMessageToGUIWindow("TopOpt: XGridPattern: %d", ts_gridPattern[0]);
    writeMessageToGUIWindow("TopOpt: YGridPattern: %d", ts_gridPattern[1]);
    writeMessageToGUIWindow("TopOpt: ZGridPattern: %d", ts_gridPattern[2]);
    writeMessageToGUIWindow("TopOpt: Max disp. allowed: %e", ts_maxDisplacementAllowed);
    writeMessageToGUIWindow("TopOpt: Stress safety factor: %.2f", ts_stressSafetyFactor);
    writeMessageToGUIWindow("TopOpt: Min Eigen-value: %.2f", ts_minEigenValueHz);
    writeMessageToGUIWindow("TopOpt: Max Feature Size: %.3e", ts_maxFeatureSize);
    writeMessageToGUIWindow("TopOpt: Smoothening iterations: %d", ts_nSmoothingIterations);
    writeMessageToGUIWindow("-----------------------------------------");
    ts_timer.stop();
	ts_timer.start();


    //ts_saveEveryNIterations = NEARESTINT(1.0/volDecrementMax/MAX_ISO_SAVES)+1;
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Saving every %d iterations ",ts_saveEveryNIterations);
    ts_min_topoptIterations = 1;
	ts_max_topoptIterations = 4;
	ts_relativeJConvergence = 100.0;// default, between two consecutive pareto optimal topologies

	
    if (ts_verbose) writeMessageToParetoLog("TopOpt: Tracing Pareto Curve");
	
	currentVolfrac  = getCurrentVolumeFractionOfOptimizedParts();// we might be restarting from a previously converged result
	convergedVolFraction = currentVolfrac;

	if (currentVolfrac > 0.99){	
		ts_currentIsoCounter = -1;
		initializePseudoDensity();
	}
	if (!solvePrimaryAndAdjointProblems(nFEA))
		return 1.0;
	
	if ((ts_topoptQuantity != COMPLIANCE) && (ts_topoptQuantity != VONMISES))
		setTStoAnalytical(false);

    if (ts_topoptMode == TOPOPT_MODE_THERMAL){
        setTStoAnalytical(false);
    }

    if (!ts_useAnalyticalTS){
		computeTopSensInitial();
        ts_scaleTSField = false;
    }

	if ((ts_topoptMode == TOPOPT_MODE_ELASTIC)|| (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC)){
		S0 = getMaxVonMisesStress();
		ts_compliance0 =ts_compliance;
		ts_displacement0 = getDeformationMax();
		ts_vonMisesStress0 = getMaxVonMisesStress();
		ts_outputcompliance0 = ts_outputcompliance;
		C0 = ts_compliance;
	}

	computeTopSensAtElems();
	TSSmoothener();
	J = computeJ();// Quantity of Interest

	tempVolDecrement = *volDecrementStart;
	nElemsToRemove = 0;
	computeIsovalue(currentVolfrac,&isoTemp,&nElemsToRemove);
	if (isoTemp > 0.1) {// probably incorrect  constraints
		strcpy(ts_lastTopOptErrorMessage,"Isosurface extraction error in Topopt\n");
		return 1.0;
	}
	if (ts_useEigenConstraint)
		saveFEAandSolveEigen(nFEA);//needed for saving eigen value below
	
	topologySaved = saveTopOptField(currentVolfrac,isoTemp,true);// this is for isosurface

	pushToSavedData();// this is for recovery
	
	if (topologySaved)  callBackFunction(true);
	nAttempts = 0; // at current volume fraction
	ts_relativeJTolerance = MAX(ts_relativeJTolerance,0.015);

    if (ts_topoptMode == TOPOPT_MODE_THERMO_ELASTIC){
        ts_relativeJTolerance = MAX(ts_relativeJTolerance,0.015);
        setTStoAnalytical(false);
        ts_nSmoothingIterations = 2;
        ts_min_topoptIterations = 1;
        ts_scaleTSField = true;
    }
	if (ts_topoptQuantity == VONMISES)  {
		ts_complianceWt = 0.1;
		ts_relativeJTolerance = MAX(ts_relativeJTolerance,0.025);
	}
	if (ts_topoptQuantity == EIGENVALUE) {
		ts_complianceWt = 0.1;
		ts_relativeJTolerance = MAX(ts_relativeJTolerance, 0.025);
	}
    if (ts_AMBuildDirection != 0)
        ts_relativeJTolerance = 0.05;

	// remove elements with low TS value
    if (ts_verbose) writeMessageToParetoLog("TopOpt: currentVolfrac = %.3f; desiredVolFrac = %.3f",currentVolfrac,desiredVolFrac);


    double JIncrement = ts_relativeJIncrementForStepping;
    while (currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac) {// small error at the end acceptable
        convergedVolFraction = currentVolfrac;
        tempVolDecrement = MIN(tempVolDecrement,currentVolfrac-desiredVolFrac);
        JTemp = J;
		if (ts_timer.getElapsedTimeInSec() > allocatedTimeInSecs) {
            sprintf(ts_lastTopOptErrorMessage,"Exceeded max allowed time of %.1f seconds",allocatedTimeInSecs);
            writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
			break;
		} 
        if (ts_verbose) writeMessageToParetoLog("-------------------------------------------------------");
        writeMessageToParetoLog("TopOpt: %.3f -> %.3f;",currentVolfrac,currentVolfrac-tempVolDecrement);
        if (ts_verbose)  writeMessageToParetoLog(" ");
		flag = IterateToVolume(currentVolfrac-tempVolDecrement,&JTemp, nFEA,&isoTemp, callBackFunction);

		if (flag == USER_REQUEST_STOP) {
            sprintf(ts_lastTopOptErrorMessage, "User terminated TopOpt ");
			break;
		}
		if (ts_useEigenConstraint) {
			saveFEAandSolveEigen(nFEA);
		}
        if (flag == ITER_TO_VOL_SUCCESS)
			flag = checkConstraints();// check for constraints


		if (flag == ITER_TO_VOL_SUCCESS)  {//OK	
			if (!isStaticFEAValid()){
				currentVolfrac = getCurrentVolumeFractionOfOptimizedParts();
				solvePrimaryAndAdjointProblems(nFEA);
				computeTopSensAtElems();
				TSSmoothener();
				nElemsToRemove = (int) ceil((1.0 -currentVolfrac)*h_nElems +1);
				computeIsovalue(currentVolfrac,&isoTemp,&nElemsToRemove);
			}
			currentVolfrac = getCurrentVolumeFractionOfOptimizedParts();
            if (currentVolfrac < convergedVolFraction) // true convergence to a lower volume
				nAttempts = 0; // reset at current volume fraction
			else{
                if (ts_verbose) writeMessageToParetoLog("\nTopOpt: currentVolfrac (%0.3f) > convergedVolFraction (%0.3f)",currentVolfrac, convergedVolFraction);
                sprintf(ts_lastTopOptErrorMessage,"Unknown error: currentVolfrac (%0.3f) > convergedVolFraction (%0.3f)",currentVolfrac, convergedVolFraction);
                writeMessageToParetoLog("\n>>%s",ts_lastTopOptErrorMessage);
				flag = DIVERGENCE_FAILURE;
			}
		}
		// sometimes stress minimization does not help. Check
		if ((ts_topoptQuantity == VONMISES) && (getMaxVonMisesStress() > 1.5*ts_vonMisesStress0)) {
			ts_complianceWt = 1;
            sprintf(ts_lastTopOptErrorMessage, "Switching to compliance minimization");
            writeMessageToParetoLog("\n>>%s", ts_lastTopOptErrorMessage);
			ts_topoptQuantity = COMPLIANCE;
			flag = MODE_CHANGE_FAILURE;
		}
		if (flag == ITER_TO_VOL_SUCCESS)  {//converged	
			convergedVolFraction = currentVolfrac;
			topologySaved = saveTopOptField(convergedVolFraction,isoTemp);// this is for final isosurface extraction
		
			pushToSavedData();// this is for recovery from failure
			J = computeJ();
			*isoValueConverged = isoTemp;
			if (ts_topoptQuantity == COMPLIANCE) {
				C = ts_compliance;
				S = getMaxVonMisesStress();
                //writeMessageToParetoLog(" C/C0 = %.2f; S/S0 = %.2f; #FEA = %d",C/C0,S/S0,*nFEA);
			}
			else if (ts_topoptQuantity == VONMISES) {
				C = ts_compliance;
				S = getMaxVonMisesStress();
                //writeMessageToParetoLog(" C/C0 = %.2f; S/S0 = %.2f; #FEA = %d",C/C0,S/S0,*nFEA);
			}
			else if (ts_topoptQuantity == GEOMETRIC_ADVANTAGE) {
                writeMessageToParetoLog(" I/I0 = %.2f; O/O0 = %.2f; GA = %.2f",ts_compliance/ts_compliance0,ts_outputcompliance/ts_outputcompliance0,ts_geometricAdvantage);
			}
			if ((topologySaved) && (!callBackFunction(true)))  break;

            if (ts_AMBuildDirection == 0)
                tempVolDecrement = MIN(tempVolDecrement,*volDecrementStart/MIN(25,MAX(sqrt(C/C0),sqrt(S/S0))));
            else
                tempVolDecrement = tempVolDecrement; // there may be a large jump in compliance. So don't decrease vol decrement

			modifyTSWeights(); // user defined modification to ts weights
            sprintf(ts_lastTopOptErrorMessage, "No TopOpt error");
            //writeMessageToParetoLog("%s ", ts_lastTopOptErrorMessage);
		}
		else {
            //writeMessageToParetoLog(" *");//failed
			retrieveFromSavedData();
			isoTemp = *isoValueConverged ;
			postprocess();
            if (ts_useEigenConstraint) {
                saveFEAandSolveEigen(nFEA);
            }
			currentVolfrac = convergedVolFraction;
            JIncrement *=0.5;
			tempVolDecrement *= 0.5;
			if (h_workSpaceInitialized)
				ReleaseWorkSpace();
			if (ts_topoptQuantity == EIGENVALUE) {
                ts_complianceWt = MIN(1, ts_complianceWt + 1.0);
                sprintf(ts_lastTopOptErrorMessage, "\nIncreasing compliance wt");
				if (ts_complianceWt == 1) {
                    sprintf(ts_lastTopOptErrorMessage, "\nSwitching to compliance minimization");
                    writeMessageToParetoLog("%s ", ts_lastTopOptErrorMessage);
					ts_topoptQuantity = COMPLIANCE;
					flag = MODE_CHANGE_FAILURE;
				}
				else
					tempVolDecrement *= 2.0;// don't decrease volume fraction to avoid early termination
			}
				
		}
		nAttempts++;
		if ((nAttempts >= ts_maxAttempts) ) {
            if (ts_verbose) writeMessageToParetoLog("TopOpt: Exiting ... nAttempts =  %d ",ts_maxAttempts);
			currentVolfrac = convergedVolFraction;
			break;
		}
		if (((currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac)) && (tempVolDecrement < ts_volErrorFraction)) {// don't apply this rule for AM constraint
            if (ts_verbose) writeMessageToParetoLog("TopOpt: Exiting ... tempVolDecrement = %.4f; ts_volErrorFraction = %.4f ",tempVolDecrement,ts_volErrorFraction);
			currentVolfrac = convergedVolFraction;
			break;
		}
	}

	saveTopOptField(currentVolfrac,isoTemp,true);// this is for plotting
	*volDecrementStart = tempVolDecrement;
	solvePrimaryAndAdjointProblems(nFEA);// solve one last time!
	callBackFunction(true);
	return currentVolfrac;
}

/**********************************************************************/
double TopOpt::computeAreaOfTriangle(double x[3],double y[3],double z[3])
{
	// Compute the area of a single triangle on GPU or CPU (common code)
	// It uses the accurate version (avoid round off errors)
	double asqr,bsqr,csqr,A,B,C,temp,area;
	asqr = ( (x[1]-x[0])*(x[1]-x[0]) + (y[1]-y[0])*(y[1]-y[0]) + (z[1]-z[0])*(z[1]-z[0]));
	bsqr = ( (x[2]-x[1])*(x[2]-x[1]) + (y[2]-y[1])*(y[2]-y[1]) + (z[2]-z[1])*(z[2]-z[1]));
	csqr = ( (x[0]-x[2])*(x[0]-x[2]) + (y[0]-y[2])*(y[0]-y[2]) + (z[0]-z[2])*(z[0]-z[2]));

	// sort the 3 numbers
	if ((asqr > bsqr) && (bsqr > csqr)) {
		A = sqrt(asqr); B = sqrt(bsqr);C = sqrt(csqr);
	}
	else if ((asqr > csqr) && (csqr > bsqr)) {
		A = sqrt(asqr); B = sqrt(csqr);C = sqrt(bsqr);
	}
	else if ((bsqr > asqr) && (asqr > csqr)) {
		A = sqrt(bsqr); B = sqrt(asqr);C = sqrt(csqr);
	}
	else if ((bsqr > csqr) && (csqr > asqr)) {
		A = sqrt(bsqr); B = sqrt(csqr);C = sqrt(asqr);
	}
	else if ((csqr > asqr) && (asqr > bsqr)) {
		A = sqrt(csqr); B = sqrt(asqr);C = sqrt(bsqr);
	}
	else {
		A = sqrt(csqr); B = sqrt(bsqr);C = sqrt(asqr);
	}

	temp = (A+(B+C))*(C*C-(A-B)*(A-B))*(A+(B-C));
	area = (double) 0.25*sqrt(temp);
	area = MAX(area,10*TOL);
	return area;
}
/**********************************************************************/
void TopOpt::getTriangleNormal(double x[3],double y[3],double z[3],double normal[3])
{
	double v1[3], v2[3],norm;

	v1[0] = x[1]-x[0];
	v1[1] = y[1]-y[0];
	v1[2] = z[1]-z[0];

	v2[0] = x[2]-x[0];
	v2[1] = y[2]-y[0];
	v2[2] = z[2]-z[0];

	normal[0] = v1[1]*v2[2]-v2[1]*v1[2];
	normal[1] = -(v1[0]*v2[2]-v2[0]*v1[2]);
	normal[2] = v1[0]*v2[1]-v2[0]*v1[1];

	norm = sqrt(normal[0]*normal[0] +normal[1]*normal[1]+normal[2]*normal[2]);

	norm = MAX(norm,TOL);
	for (int i = 0; i < 3; i++)
		normal[i] /=norm;
}
/**********************************************************************/
void TopOpt::computeContributionOfTriangle(double x[3],double y[3],double z[3],double *vol,double *area,double center[3])
{
	//uses divergence theorem
	double xBar,yBar,zBar,nz,normal[3];
	*area = computeAreaOfTriangle(x,y,z);
	*area = MAX(*area,10*TOL);
	getTriangleNormal(x,y,z,normal);
	nz = normal[2];
	xBar = (x[0]+x[1]+x[2])/3.0;
	yBar = (y[0]+y[1]+y[2])/3.0;
	zBar = (z[0]+z[1]+z[2])/3.0;

	*vol  = *area*nz*zBar;

	center[0] = *area*xBar;
	center[1] = *area*yBar;
	center[2] = *area*zBar;

}
/**********************************************************************/
TopOpt::~TopOpt()
{
    if (ts_verbose) writeMessageToParetoLog("Releasing TopOpt Object \n");
	

	safeFree(ts_topSensElem);ts_topSensElem = NULL;
    safeFree(ts_topSensElemUnscaled);ts_topSensElemUnscaled = NULL;
	safeFree(ts_topSensNode);ts_topSensNode = NULL;
	for (int i =0; i < MAX_ISO_SAVES; i++){
		safeFree(ts_topSensNodeSave[i]);
		ts_topSensNodeSave[i] = NULL;

		safeFree(ts_elemExists[i]);
		ts_elemExists[i] = NULL;
	}

	
	safeFree(ts_stressAdjoint);ts_stressAdjoint = NULL;
	safeFree(ts_stressAdjointForce);ts_stressAdjointForce = NULL; 
	safeFree(ts_backupData.sol);ts_backupData.sol = NULL;
	safeFree(ts_backupData.pseudoDensity);ts_backupData.pseudoDensity = NULL;
	safeFree(ts_backupData.nodeExists);ts_backupData.nodeExists = NULL;
	safeFree(ts_backupData.topSensElem);ts_backupData.topSensElem = NULL;
	safeFree(ts_backupData.topSensNode);ts_backupData.topSensNode = NULL;
	safeFree(ts_backupData.neighborNodes);ts_backupData.neighborNodes = NULL;
	safeFree(ts_backupData.nodeSignature);ts_backupData.nodeSignature = NULL;

} 
