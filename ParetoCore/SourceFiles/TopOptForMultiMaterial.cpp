#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#ifndef __APPLE__
#include <omp.h>
#endif
#include "Timer.h"
#include <Eigen/Dense>
#include <Eigen/QR>
#include <IntegerSet.h>
#include "TopOptForMultiMaterial.h"
#include "queue.h"
using namespace std;
#pragma warning(disable:4996)

/**********************************************************************/
TopOptForMultiMaterial::TopOptForMultiMaterial()
{
}
/**********************************************************************/
void TopOptForMultiMaterial::enableMultiMaterialVerbose()
{
	tomm_verbose = true;
}
/**********************************************************************/
void TopOptForMultiMaterial::disableMultiMaterialVerbose()
{
	tomm_verbose = false;
}

/**********************************************************************/
void TopOptForMultiMaterial::setMultiMaterialNumSubSteps(int num)
{
	tomm_numSupSteps = num;
}

/**********************************************************************/
void TopOptForMultiMaterial::initializeTopOptForMultiMaterial()
{

    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	tomm_foundNodeNeighbors = false;

	tomm_ParetoCounter = 0;

    tomm_savePerStep = 2;

    tomm_numSupSteps = 2;
	tomm_currentI.assign(h_nElems,0);
	tomm_alternateI.assign(h_nElems,0);
	tomm_currentMaterial.assign(h_nElems,0);

	tomm_Ranked.clear();

	tomm_isChanged.assign(h_nElems,false);

	//tomm_initalCost = tomm_targetCost = tomm_deltaCost = 0.0;
    tomm_initialCompliance = tomm_initialDeformatoin = 0.0;
	tomm_nSmoothing = 2;

	tomm_topoptQuantity = STIFFNESS;
	tomm_minAllowableCostFraction = 0.05;
	tomm_nSensSmootheningIter = ts_nSmoothingIterations;


	tomm_showElem.assign(h_nElems,true);
	tomm_isCrossSectionAvailable = false;
    tomm_Costs.assign(h_nMaterials,0);
    tomm_CostsSorted.assign(h_nMaterials,0);
    tomm_CostsSortedID.assign(h_nMaterials,0);

    strcpy(ts_lastTopOptErrorMessage,"No TopOpt error message reported");
    for (int matl = 0; matl < h_nMaterials; matl++)
        tomm_Costs[matl] = getPriceOfMaterial(matl);


    cout << "material costs: " << endl;
    for (int i= 0;i < h_nMaterials;i++)
        cout << i << ": " << tomm_Costs[i] << endl;
    sortCostsDescending();
}
/**********************************************************************/
bool TopOptForMultiMaterial::isSavingMultiMaterialDesign(int skipStep)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (tomm_ParetoCounter%skipStep == 0) 
		return true;
	else 
		return false;
}
/**********************************************************************/
bool TopOptForMultiMaterial::isNodeNeighborsFound()
{
	return tomm_foundNodeNeighbors;
}
/**********************************************************************/
void TopOptForMultiMaterial::sortCostsDescending()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	vector<double> costs(h_nMaterials,0);
	costs = tomm_Costs;

	tomm_CostsSortedID.assign(h_nMaterials,0);
    for (int mat= 0;mat<h_nMaterials;mat++){
        tomm_CostsSortedID[mat] = mat;
    }
	sort(tomm_CostsSortedID.begin(),tomm_CostsSortedID.end(),[&costs](size_t i1, size_t i2) {return costs[i1] > costs[i2];});
	tomm_CostsSorted = costs;
	if (tomm_verbose){
        cout << "costs sorted: " << endl;
		for (int i= 0;i<h_nMaterials;i++){
            cout << i << ": " << tomm_CostsSorted[i] << endl;
		}
	}
    if (tomm_verbose) cout << "---------------------------------------------------" << endl;
}
/**********************************************************************/
void TopOptForMultiMaterial::setMaterialIdOfAll(int mat)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		setMaterialIdOfElem(mat,elem);
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::setMultiMaterialNumberOfSmoothing(int nSmoothing)
{
	tomm_nSmoothing = nSmoothing;
}
/**********************************************************************/
void TopOptForMultiMaterial::setMultiMaterialQuantityOfInterest(const TOPOPT_OBJECTIVES &quantity)
{
	tomm_topoptQuantity = quantity;
}
/**********************************************************************/
void TopOptForMultiMaterial::setMaterialSymmetry(int materialSymmetry)
{
	tomm_materialSymmetry = false;
	if (materialSymmetry == 1)
		tomm_materialSymmetry = true;
}
/**********************************************************************/
int TopOptForMultiMaterial::initializeToStiffestDesign(double CostDecrement,int *nFEA,int drawDirection,
										int symmetryIndex, bool keepElems)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	ts_drawDirection = drawDirection;
	ts_symmetryIndex = symmetryIndex;

	double maximumCostPossible = h_nElems*tomm_CostsSorted[0];
	double currentCost;

	tomm_currentIsoCounter = -1;
	initializePseudoDensity();
	findMultiMaterialTemplates(0); // 0: expensive, 1: cheap, 2: random
    //setStructuringSize(19);
    //findAllNeighboringElems();
	tomm_initalCost = computeCurrentCost();
	currentCost = tomm_initalCost;

	double currentCostfrac  = currentCost/maximumCostPossible;// we might be restarting from a previously converged result
	double convergedCostFraction = currentCostfrac;

    cout <<"TopMM: Initial Cost = " << tomm_initalCost << " Current Cost frac. = " << currentCostfrac << endl;

	// solve first time
    if(!solvePrimaryAndAdjointProblems_MultiMaterial(nFEA)){
        cout << "TopMM: iterate to stiffest:  FEA Failure! " << endl;
		return ITER_TO_VOL_FEA_FAILURE;
	}

    ts_vonMisesStress0 = getMaxVonMisesStress();

	double C0 = computeCompliance();
	ts_compliance = C0;

	// find sensitivty
    computeTopSensInitial();
    computeTopSensAtElemsApproximate_MultiMaterial();

	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);

    double lastIterationObjective = computeCompliance();
	tomm_HistoryData.costFraction.push_back(currentCostfrac);
	tomm_HistoryData.objective.push_back(lastIterationObjective);
	tomm_HistoryData.compliance.push_back(computeCompliance());

	pushToSavedData_MultiMaterial();

	findMaterialPercentages();

	// store previous design on Pareto
		
	vector<double> convergedPseudoDensity(h_nElems,0);
	vector<int> convergedMaterial(h_nElems,-1);
	for (int elem=0;elem<h_nElems;elem++){
		convergedPseudoDensity[elem] = getElemPseudoDensity(elem);
		convergedMaterial[elem] = getElemMaterialID(elem);
	}
	double *convergerdSol = (double *)malloc(3*h_nNodes*sizeof(double));
	getSolutionVector(convergerdSol);

	double covergedCost = getCurrentCostFraction_MultiMaterial();
	//double targetCost = covergedCost - CostDecrement;

	// design of previous substep
	vector<double> lastIterationPseudoDensity = convergedPseudoDensity;
	vector<int> lastIterationMaterial = convergedMaterial;

	// design of current substep
	vector<double> currentIterationPseudoDensity(h_nElems,0);
	vector<int> currentIterationMaterial(h_nElems,0);

	
	double dcost = CostDecrement;
	double objectiveConverged;

	// keep all elems ? 
	if (keepElems){
		for (int elem = 0;elem < h_nElems;elem++){
			h_keepElem[elem] = true;
		}
	}

	int nSubsteps = 20;
	int maxAllowedChange = int (CostDecrement*h_nElems);
	for (int substep = 0; substep < nSubsteps+1; substep++){
		computeVoidToSolidRankID();
		reduceInefficiency(dcost,maxAllowedChange);
		removeDisconnectedElements_MultiMaterial(0.5);
		deleteStaticFEAResults();
		findNeighboringElemsOfNodes();
		findTemplates();
		// solve
        if(!solvePrimaryAndAdjointProblems_MultiMaterial(nFEA)){
            cout << "TopMM: iterate to cost: Reducing inefficiency FEA Failure!" << endl;
			return ITER_TO_VOL_FEA_FAILURE;
		}
		// update current
		for (int elem=0;elem<h_nElems;elem++){
			currentIterationPseudoDensity[elem] = getElemPseudoDensity(elem);
			currentIterationMaterial[elem] = getElemMaterialID(elem);
		}
        objectiveConverged = computeCompliance();
		// update last iteration
		int nChanges = 0;
		for (int elem=0;elem<h_nElems;elem++){
			if (lastIterationPseudoDensity[elem] != currentIterationPseudoDensity[elem]){
				nChanges++;
				continue;
			}
			if (lastIterationMaterial[elem] != currentIterationMaterial[elem]){
				nChanges++;
				continue;
			}
		}

		currentCost = computeCurrentCost();

		bool success = true;
		bool isConverged = false;
		if (objectiveConverged - lastIterationObjective > 0.0) success = false;
		//if (fabs(tomm_initalCost - currentCost) > 0.001 * tomm_initalCost) isConverged = false;
		if (double(nChanges) < 0.001*CostDecrement*h_nElems) isConverged = true;
		if (fabs(objectiveConverged - lastIterationObjective) < 0.001*lastIterationObjective) isConverged = true;

		if (substep == nSubsteps) isConverged = true;


		if ( success){ // successful substep
			lastIterationObjective = objectiveConverged;
			lastIterationPseudoDensity = currentIterationPseudoDensity;
			lastIterationMaterial = currentIterationMaterial;

			tomm_HistoryData.costFraction.push_back(currentCostfrac);
			tomm_HistoryData.objective.push_back(objectiveConverged);
			tomm_HistoryData.compliance.push_back(computeCompliance());

			findMaterialPercentages();
			pushToSavedData_MultiMaterial();
			tomm_ParetoCounter++;
		}
		else if (!success) {
			for (int elem=0; elem<h_nElems;elem++){
				setPseudoDensityOfElem(elem, lastIterationPseudoDensity[elem]); 
				setMaterialIdOfElem(lastIterationMaterial[elem],elem);
			}
			retrieveFromSavedData_MultiMaterial(nFEA);
		}
		if (isConverged){
			tomm_ParetoCounter = 0;
			break;
		}
		computeSolidToVoidRankID();
		reduceCost(dcost,maxAllowedChange);
		removeDisconnectedElements_MultiMaterial(0.5);
		deleteStaticFEAResults();
		findNeighboringElemsOfNodes();
		findTemplates();
		// solve
        if(!solvePrimaryAndAdjointProblems_MultiMaterial(nFEA)){
            cout << "TopMM: iterate to cost: Reducing cost FEA Failure!" << endl;
			return ITER_TO_VOL_FEA_FAILURE;
		}
	}
    safeFree(convergerdSol);
	return SUCCESS;

}
/**********************************************************************/
void TopOptForMultiMaterial::initializeToMostExpensiveMaterial()
{

    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// sorted in descending order
	sortCostsDescending();
	int mostExpensiveMaterial = tomm_CostsSortedID[0]; 
	setMaterialIdOfAll(mostExpensiveMaterial);

    if (tomm_verbose) cout << "initializing to most expensive material with cost of " << tomm_CostsSorted[0] << endl;
	for (int elem=0;elem<h_nElems;elem++){
		tomm_currentMaterial[elem] = mostExpensiveMaterial;
	}

}
/**********************************************************************/
void TopOptForMultiMaterial::initializeToLeastExpensiveMaterial()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	// sorted in descending order
	sortCostsDescending();
	int mostExpensiveMaterial = tomm_CostsSortedID[h_nMaterials-1]; 
	setMaterialIdOfAll(mostExpensiveMaterial);

    for (int elem=0;elem<h_nElems;elem++)
		tomm_currentMaterial[elem] = mostExpensiveMaterial;

}
/**********************************************************************/
void TopOptForMultiMaterial::initializeToRandomDistributionMaterial()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// sorted in descending order
	sortCostsDescending();
	//int mostExpensiveMaterial = tomm_CostsSortedID[h_nMaterials-1]; 
	//setMaterialIdOfAll(mostExpensiveMaterial);
	for (int elem=0;elem<h_nElems;elem++){
		int currentId = rand()%h_nMaterials;
		setMaterialIdOfElem(currentId,elem);
		tomm_currentMaterial[elem] = currentId;
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::findMultiMaterialTemplates(int state)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// compute template matrices and initialize to most expensive material
	int materialindex = 0;
	int elem;
	// set all to zero
	for (elem=0;elem<h_nElems;elem++)
		setMaterialIdOfElem(materialindex,elem);
	// create one template of each material
	for (elem=0;elem<h_nMaterials;elem++){
		materialindex = elem;
		setMaterialIdOfElem(materialindex,elem);
	}
	// compute templates
	findTemplates();
	computeKeOfTemplates();
	// initialize for Pareto
	if (state == 0) initializeToMostExpensiveMaterial();
	else if (state == 1) initializeToLeastExpensiveMaterial();
	else initializeToRandomDistributionMaterial();
	// find templates
	findTemplates();

	deleteStaticFEAResults();
}
/**********************************************************************/
double TopOptForMultiMaterial::computeCurrentCost()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	double currentCost = 0.0;
	for (int elem = 0;elem < h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		currentCost += tomm_Costs[getMaterialIdOfElem(elem)];
	}
    if (tomm_verbose) cout <<" Current cost is " << currentCost << endl;
	return currentCost;
}
/**********************************************************************/
void TopOptForMultiMaterial::computeTopSensAtElemsApproximate_MultiMaterial()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// Compute Topological sensitivity at the center of each element
	int		elem;
	double	TSCompliance = 0, TSObjective= 0;
	double elemSol[24];

	if (ts_topSensElem == NULL) {
        cout << "TopOptMM: Initialize Topopt!" << endl;
		return;
	}
	// find the topsensitivity at all elements
#pragma omp parallel for private(elem,elemSol,TSCompliance,TSObjective)
	for (elem = 0; elem < h_nElems;elem++) {
		if (getElemPseudoDensity(elem) < 0.5) continue;// cannot compute TS of elements that are outside; use filtering later on
		getElemSolution(elem,elemSol); 
		TSCompliance = computeElemInnerProduct(elem,elemSol,elemSol);
		//TSCompliance += computeBodyForceSensitivity(elem,elemSol);
		TSCompliance /= TSComplianceNorm0;
		ts_topSensElem[elem] = TSCompliance ;
	}
}
/**********************************************************************/
void  TopOptForMultiMaterial::computeNodalTSField_MultiMaterial()
{
	int node,j,maxElem,codedElemNum,nodeIndex,elem,count;

    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
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

			if (getElemPseudoDensity(elem) < 0.5) continue;
			// else even if the element is outside, use the previously computed TS value
			count++;
			ts_topSensNode[node] += ts_topSensElem[elem];	
		}
		count = MAX(1,count);
		ts_topSensNode[node] /= count;
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::smoothenSensitivty_MultiMaterial(int nIter)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
		// Compute TS at each node as average of each neighboring element
	// Then compute TS at each elem as average of 8 nodes
	// Finally recompute TS at node as average of each neighboring element
	int node=0,elem,m=0;

	for (int iter = 0; iter < ts_nSmoothingIterations; iter++) {		
		computeNodalTSField_MultiMaterial();
		//imposeDrawDirection(ts_drawDirection);
		//imposeSymmetry(ts_symmetryIndex);
        //imposeExtrude(ts_throughCutDirection);
		imposeCyclicSymmetry_MultiMaterial(ts_axiSymmetryIndex);

#pragma omp parallel for private(elem,m,node)
		for (elem = 0; elem < h_nElems;elem++) {
			//if (h_keepElem[elem]){// imposing constraints may have affected nodes TS
			//	ts_topSensElem[elem] = ts_TSMax;
			//	continue;
			//}
			if (getElemPseudoDensity(elem) <0.5) continue;
			ts_topSensElem[elem] = 0;
			for (m = 0; m < 8; m++) {
				node = h_elems[(h_nElems)*m + elem];
				ts_topSensElem[elem] += ts_topSensNode[node];
			}
			ts_topSensElem[elem] /= 8;		
		}
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::smoothenElemField_MultiMaterial(vector<double>&elemfield,int nIter)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// field: element based
	int neighborElems[8],ElemNodes[8];
	int node,elem,nNeighbors,count;
	double fieldMax;

	// find nodal field
	vector<double> nodefield (h_nNodes,0);
	fieldMax = -1e12;
	for (int iter=0;iter<nIter;iter++){
		nodefield.assign(h_nNodes,0);
		for (node=0;node<h_nNodes;node++){
			nNeighbors = getNeighboringElemsOfNode(node,neighborElems);
			count = 0;
			for(int i=0;i<nNeighbors;i++){
				elem = neighborElems[i];
				//if (getElemPseudoDensity(elem) <0.5) continue;
				count++;
				nodefield[node] += elemfield[elem];
				fieldMax = MAX(fieldMax,elemfield[elem]);
			}
			if (count > 0)
				nodefield[node] /=count;
		}
		// compute element field
		elemfield.assign(h_nElems,0);
		for (elem=0;elem<h_nElems;elem++){
			/*if (h_keepElem[elem]){
				elemfield[elem] = fieldMax;
				continue;
			}*/
			if (getElemPseudoDensity(elem) <0.5) continue;
			getElemNodes(elem,ElemNodes);
			for (int i=0;i<8;i++){
				node = ElemNodes[i];
				elemfield[elem] += nodefield[node]/8.0; 
			}
		}
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::swapMaterials(double CostDecrement,int maxNumElems)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	double dcost = 0.0;
	tomm_isChanged.assign(h_nElems,false);

	double DeltaCost = CostDecrement * tomm_initalCost;

    if (tomm_verbose) cout << "CostDecrement = " <<CostDecrement <<", DeltaCost = " << DeltaCost << endl;

	int numChangedElems = 0;

	int counter = -1;
	for (int i=0;i<h_nElems;i++){
		counter++;
		int currentElem = tomm_Ranked[i].element;
		int alternateMaterial = tomm_Ranked[i].material;
		int currentMaterial = tomm_currentMaterial[currentElem]; 
		if (currentElem < 0) continue;
		if (getElemPseudoDensity(currentElem) < 0.5)continue;
		if (tomm_isChanged[currentElem]) continue;

		if (alternateMaterial == -1){
			if (h_keepElem[currentElem]) continue;
			setPseudoDensityOfElem(currentElem,0.0);
			tomm_isChanged[currentElem] = true;
			int nChangedElems = imposeManufacturingConstraintsOnElem(currentElem);
			dcost += (nChangedElems+1)*tomm_Costs[currentMaterial];
			numChangedElems += nChangedElems;

		}
		else{
			setMaterialIdOfElem(alternateMaterial,currentElem);
			tomm_isChanged[currentElem] = true;
			int nChangedElems = 0;
			if (tomm_materialSymmetry) {
				nChangedElems = imposeManufacturingConstraintsOnElem(currentElem);
			}
			dcost += (nChangedElems+1)*fabs(tomm_Costs[currentMaterial] - tomm_Costs[alternateMaterial]);
			numChangedElems += nChangedElems;
		}

		if (dcost > DeltaCost) break;

		// 
		if (numChangedElems > maxNumElems) break;
	}
    if (tomm_verbose) cout << "\nNumber of elements changed = " << numChangedElems << endl;
	if (tomm_verbose) printf("%i elements swapped with dcost = %g \n",counter,dcost);

}
/**********************************************************************/
double TopOptForMultiMaterial::getCostFractionAtCounter(int counter)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (tomm_CostFractionsSaved.size() == 0 || counter >= tomm_CostFractionsSaved.size() || counter < 0){
        cout << "No data found, Invalid Counter! " << endl;
		return -1;
	}

	return (tomm_CostFractionsSaved[counter]);
}
/**********************************************************************/
int  TopOptForMultiMaterial::getClosestCounterAtCostFraction(double costFrac)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (tomm_currentIsoCounter == -1)
		return -1;

	int bestCounter = 0;
	double maxErr=1e8;
	for (int counter = 0; counter <= tomm_CostFractionsSaved.size();counter++){
		if (fabs(tomm_CostFractionsSaved[counter] - costFrac) <= maxErr){
			bestCounter = counter;
			maxErr = fabs(tomm_CostFractionsSaved[bestCounter] - costFrac) ;
		}
	}
    if (tomm_verbose) cout <<"TopoptForMultiMaterial: Retrieving counter " << bestCounter << endl;
	return bestCounter;
}
/**********************************************************************/
bool TopOptForMultiMaterial::removeDisconnectedElements_MultiMaterial(double cutoff)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (!h_voxelgrid) {
		return false;
	}
	int elem,currentElem,neighborElem,startElement,nElemsToRemove;
	bool *elemVisited;
	Queue Q;

	elemVisited = (bool *) malloc(h_nElems*sizeof(bool));
	memset(elemVisited,false,h_nElems*sizeof(false));
	Q = CreateQueue(h_nElems);
	MakeEmpty(Q);
	// find an element that we must keep
	for (elem = 0; elem < h_nElems;elem++) {	
		if (!h_keepElem[elem]) continue;
		startElement = elem;
		break;
	}

	Enqueue(startElement, Q); // add elem to queue
	elemVisited[startElement] = true;
	while (!IsEmpty(Q)) {
		currentElem = Front(Q);
		Dequeue(Q);
		for (int i=0;i<7;i++){// only consider 6 face neighbors + self
			neighborElem = h_StructuringElems[h_StructuringSize*currentElem+i];
			if (neighborElem == -1) continue;
			if (elemVisited[neighborElem]) continue;
			if (getElemPseudoDensity(neighborElem) < cutoff) continue;
			Enqueue(neighborElem, Q);
			elemVisited[neighborElem] = true;
		}
	}
	MakeEmpty(Q);
	DisposeQueue(Q);
	nElemsToRemove= 0;
	for (elem = 0; elem < h_nElems;elem++) {	
		if ((!elemVisited[elem]) && (getElemPseudoDensity(elem) > cutoff) && (!h_keepElem[elem])){
			setPseudoDensityOfElem(elem,0);// remove it
			nElemsToRemove++;
		}
	}
	safeFree(elemVisited);
	return true;
}
/**********************************************************************/
void TopOptForMultiMaterial::findMaterialPercentages()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	tomm_MaterialPercentage.assign(h_nMaterials,0);
	for (int elem = 0;elem < h_nElems;elem++){
		if (getElemPseudoDensity(elem) < 0.5) continue;
		tomm_MaterialPercentage[getMaterialIdOfElem(elem)] ++;
	}
     if (tomm_verbose)
         for (int matl = 0; matl < h_nMaterials; matl++)
                cout <<"Matl " << matl <<"; #elems = " << tomm_MaterialPercentage[matl] << endl;
}
/**********************************************************************/
int TopOptForMultiMaterial::getNumElemsOfMaterial(int matId)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (matId < 0 || matId >= h_nMaterials) {
        return 0;
	}
	return tomm_MaterialPercentage[matId];
}
/**********************************************************************/
int TopOptForMultiMaterial::getNeighboringElementsOfNodeRecursive(const int node,vector<int> &neighboringElemsOfElem,int nRecursions)
{
    //if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	neighboringElemsOfElem.clear();
	IntegerSet neighbors;
	int tempNodeNeighbors[8],n,currentStart,currentEnd,recursion,tempElem,i,j,nNeighbors;

	neighbors.clear();
	n = getNeighboringElemsOfNode(node,tempNodeNeighbors);

	for ( i=0;i<n;i++){
		if (tempNodeNeighbors[i] < 0) continue;
		if (getElemPseudoDensity(tempNodeNeighbors[i]) < 0.5) continue;
		neighbors.insertCheckForUnique(tempNodeNeighbors[i]);
	}
	nRecursions = MIN(nRecursions,3);//max allowed
	recursion = 1;//completed one recursion
	currentStart = 0;
	while (recursion < nRecursions){
		currentEnd = neighbors.size();
		for ( j = currentStart; j < currentEnd; j++) {
			tempElem = neighbors.elementAt(j);
			int tempElemNeighbors[27];
			n = getNeighboringElementsOfElem(tempElem,tempElemNeighbors);
			for ( i=0;i<n;i++){
				if (tempElemNeighbors[i] < 0) continue;
				if (getElemPseudoDensity(tempElemNeighbors[i]) < 0.5) continue;
				neighbors.insertCheckForUnique(tempElemNeighbors[i]);
			}
		}
		currentStart= currentEnd;
		recursion++;
	}
	nNeighbors = neighbors.size();
	
    for ( j = 0; j < nNeighbors; j++)
		neighboringElemsOfElem.push_back(neighbors.elementAt(j));//copy over 

	neighbors.clear();
	return nNeighbors;
}
/**********************************************************************/
void TopOptForMultiMaterial::findAllNodeNeighbors(int recursion)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (tomm_foundNodeNeighbors) return;

	int node;
	vector<int> neighboringElemsOfElem;
	tomm_NodeNeighbors.clear();
	tomm_NodeNeighbors.resize(h_nNodes);
	//#pragma omp parallel for private(node,nodeMaterial,neighboringElemsOfElem)
	for (node = 0; node < h_nNodes;node++){
		neighboringElemsOfElem.clear();
		int n = getNeighboringElementsOfNodeRecursive(node,neighboringElemsOfElem,recursion);
		tomm_NodeNeighbors[node].resize(n);
		tomm_NodeNeighbors[node] = neighboringElemsOfElem;
	}
	tomm_foundNodeNeighbors = true;
}
/**********************************************************************/
void TopOptForMultiMaterial::findAllNodeMaterials()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	vector<int> nodeMaterial;
	tomm_NodeMaterial.resize(h_nNodes);
	vector<int> neighboringElemsOfElem;
	int node ;
//#pragma omp parallel for private(node,neighboringElemsOfElem)
	for (node = 0; node < h_nNodes;node++){
		neighboringElemsOfElem.clear();
		//int n = getNeighboringElementsOfNodeRecursive(node,neighboringElemsOfElem,nRecursions);
		int n = tomm_NodeNeighbors[node].size();
		if (n == 0){
			cout << "no neighbors for node " << node << endl;
			break;
		}
		nodeMaterial.assign(h_nMaterials,0);
		for (int i=0;i < n;i++){
			int neighbor = tomm_NodeNeighbors[node][i];
			if ( neighbor < 0) continue;
			if (getElemPseudoDensity(neighbor) < 0.5) continue;
			int matId = getMaterialIdOfElem(neighbor);
			nodeMaterial[matId]  = nodeMaterial[matId] + 1;
		}
		tomm_NodeMaterial[node] = nodeMaterial;
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::setMaterialGradingLevel(int gradinglevel)
{
	tomm_gradingLevel = gradinglevel;
}
/**********************************************************************/
void TopOptForMultiMaterial::getNodeMaterials(const int node,vector<int> &nodeMaterials)
{
	nodeMaterials = tomm_NodeMaterial[node];
}
/**********************************************************************/
void TopOptForMultiMaterial::setNodeNeighborsStatus(bool status)
{
	tomm_foundNodeNeighbors = status;
}
/**********************************************************************/
double TopOptForMultiMaterial::TraceParetoForMultiMaterial(double desiredCostFrac,double CostDecrementMax, int maxFEA,int *nFEA, 
														bool(*callBackFunction)(bool saveIsoSurface),int drawDirection,int symmetryIndex,
																	double *CostDecrementStart,double allocatedTimeInSecs)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	/***************************************************/

    // auto functions are functions that are only used locally in the parent function
	auto isTargetCostFracValid = [](const double &desiredCostFrac, const double &minAllowableCostFraction)
	{
		return (desiredCostFrac > minAllowableCostFraction);
	};
	auto isFirstTimeSolving = [](const double &currentCost,const double &maximumCostPossible){
		return (fabs(currentCost-maximumCostPossible) < 0.01*maximumCostPossible);
	};
	auto adjustDecrementForFinalStep = [](double &tempCostDecrement,const double &currentCostfrac,const double &desiredCostFrac){
		tempCostDecrement = MIN(tempCostDecrement,currentCostfrac-desiredCostFrac);
	};

	auto tooMuchTime = [](const double &elapsed, const double &allowed){
		return (elapsed > allowed);
	};
	auto tooMuchChangeInCompliance = [](const double &currentComplaince, const double &previousCompliance, const double &sf){
		return (currentComplaince > sf*previousCompliance);
	};
	auto isStuck = [](const double &lastCost, const double &currentCost, const double &stepSize){
		return (lastCost - currentCost < 0.3*stepSize);
	};
	/***************************************************/

    disableNodalSpMV();// nodal SPMV is not possible for material optimization


    if (tomm_verbose) cout << "Tracing Multimaterial Pareto" << endl;

	if (! isTargetCostFracValid(desiredCostFrac,tomm_minAllowableCostFraction)){
        cout << "TopOptForMultiMaterial: Desired cost fraction " << desiredCostFrac << " is less than allowable " << tomm_minAllowableCostFraction << endl;
		desiredCostFrac = tomm_minAllowableCostFraction;
	}

	tomm_timer.stop();
	tomm_timer.start();
	ts_drawDirection = drawDirection;
	ts_symmetryIndex = symmetryIndex;

	double maximumCostPossible = h_nElems*tomm_CostsSorted[0];
	double currentCost = computeCurrentCost();


	if (isFirstTimeSolving(currentCost,maximumCostPossible)){	
        ts_currentIsoCounter = -1;
        initializePseudoDensity();
		findMultiMaterialTemplates();
		tomm_initalCost = computeCurrentCost();
		currentCost = tomm_initalCost;
		tomm_ParetoCounter = 0;
        cout << " tomm_gradingLevel  = " << tomm_gradingLevel << endl;
		findAllNodeNeighbors(tomm_gradingLevel);
	}
	initializeToStiffestDesign(CostDecrementMax,nFEA,drawDirection,symmetryIndex);

	double currentCostfrac  = currentCost/tomm_initalCost;// we might be restarting from a previously converged result
	double convergedCostFraction = currentCostfrac;


    if (solvePrimaryAndAdjointProblems_MultiMaterial(nFEA) != 1)
		return 1.0;

    double S0 = getMaxVonMisesStress();
    ts_vonMisesStress0 = S0;

	double C0 = computeCompliance();
	ts_compliance = C0;

	// find sensitivty
     if (tomm_topoptQuantity == STIFFNESS){
		computeTopSensInitial();
		computeTopSensAtElemsApproximate_MultiMaterial();
	}
	//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);


    double tempCostDecrement = *CostDecrementStart;
	double tempCostFraction = computeCurrentCost()/tomm_initalCost;

	double previousMaxStress = S0;
	double previousCompliance = C0;
    double objective0 = computeCompliance();
	double previousObjective = objective0;
	double currentObjtive = previousObjective;
	double objectiveTemp;

	double *lastPseudoDensities = (double *)malloc(h_nElems*sizeof(double));
	getPseudoDensity(lastPseudoDensities);

	vector<double> lastMaterials(h_nElems,0); 
	for (int elem=0;elem<h_nElems;elem++){
		lastMaterials[elem] = getElemMaterialID(elem);
	}

	int nAttempts = 0; // at current cost fraction
	int maxAttempts = 5;
	double complianceSF = 2.;//1.1;
	double stressSF = 1.05;
	double objectiveSF = 2;//1.1;


	tomm_HistoryData.costFraction.push_back(currentCostfrac);
	tomm_HistoryData.objective.push_back(currentObjtive);
	tomm_HistoryData.compliance.push_back(computeCompliance());

	findMaterialPercentages();
	//findAllNodeMaterials();
	callBackFunction(true);

	while (currentCostfrac > (1+ts_volErrorFraction)*desiredCostFrac) {// small error at the end acceptable
		tomm_ParetoCounter++;

		convergedCostFraction = currentCostfrac;
		adjustDecrementForFinalStep(tempCostDecrement,currentCostfrac,desiredCostFrac);

		objectiveTemp =  currentObjtive;
		if (tooMuchTime(tomm_timer.getElapsedTimeInSec(),allocatedTimeInSecs)){
            cout << "TopOptForMultiMaterial: Exceeded max allowed time (seconds) " << allocatedTimeInSecs << endl;
			break;
		}
        cout <<"TopOptMM: " << currentCostfrac << " -> " <<currentCostfrac-tempCostDecrement ;

		int flag = iterateToCost_MultiMaterial(tempCostDecrement,objectiveTemp,nFEA);

		if (flag != ITER_TO_VOL_SUCCESS)
            cout <<"\n Failed iterate To Volume!" << endl;

		if (flag == ITER_TO_VOL_SUCCESS) {
			flag = checkConstraintsForMultiMaterial();// check for constraints
			if (flag != ITER_TO_VOL_SUCCESS)
                cout <<"\n Failed MultiMaterial Constraints!" << endl;
		}

		if ( tooMuchChangeInCompliance(computeCompliance(),previousCompliance,complianceSF)){
            printf(", C/C0 = %.2f\n",computeCompliance()/C0);
			printf("Compliance divergence!\n");
			flag = DIVERGENCE_FAILURE;
		}

		tempCostFraction = getCurrentCostFraction_MultiMaterial();

		if (flag == ITER_TO_VOL_SUCCESS){
            currentObjtive = computeCompliance();
			currentCostfrac = computeCurrentCost()/tomm_initalCost;
			
            cout << "; C/C0 = " << computeCompliance()/C0;
            cout << "; #FEA = " << *nFEA << endl;
			getPseudoDensity(lastPseudoDensities);
			for (int elem=0;elem<h_nElems;elem++){
				lastMaterials[elem] = getElemMaterialID(elem);
			}
			pushToSavedData_MultiMaterial();
			previousObjective = currentObjtive;
			previousCompliance = computeCompliance();
			nAttempts = 0;

			tomm_HistoryData.costFraction.push_back(currentCostfrac);
			tomm_HistoryData.objective.push_back(currentObjtive);
			tomm_HistoryData.compliance.push_back(computeCompliance());

			tempCostDecrement = *CostDecrementStart;

			findMaterialPercentages();
			//findAllNodeMaterials();
			if (!callBackFunction(true)) break;
		}
		else{
			printf(" * \n");
			setPseudoDensity(lastPseudoDensities);

			for (int elem=0; elem<h_nElems;elem++){
				setMaterialIdOfElem(lastMaterials[elem],elem);
			}

			retrieveFromSavedData_MultiMaterial(nFEA);

			if (isStuck(convergedCostFraction, tempCostFraction, tempCostDecrement)){ 
				// converged but swapping between two local optima with the same cost
				// happens when materials are the same cost;
				if (tempCostDecrement < 0.09)  tempCostDecrement *= 2.0;
				else tempCostDecrement *= 1.5;
				cout << "tempCostDecrement: " << tempCostDecrement << endl;
			}
			else{
				tempCostDecrement *= 0.8;
			}
			currentCostfrac = convergedCostFraction;
			nAttempts++;
		}
		if (tempCostDecrement > 0.5){
			printf("step size %g is too large, optimization terminated!\n",tempCostDecrement);
			break;
		}

		/*if ((nAttempts >= maxAttempts)  || ( (currentCostfrac > (1+ts_volErrorFraction)*desiredCostFrac) )&& (tempCostDecrement < ts_volErrorFraction)){
			printf("step size %g is too small, optimization terminated!\n",tempCostDecrement);
			break;
		}*/
	}

	printf("-------------------------------------------------------\n");
	safeFree(lastPseudoDensities);
	return currentCostfrac;
}
/**********************************************************************/
bool TopOptForMultiMaterial::checkConstraintsForMultiMaterial()
{
	return SUCCESS;
}
/**********************************************************************/
void TopOptForMultiMaterial::computeSolidToVoidRankID()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	// assume the fea solution is valid at this point

    if (tomm_topoptQuantity == STIFFNESS){
		computeTopSensAtElemsApproximate_MultiMaterial(); // Note: using parent function normalizes TS field at wrong place for multimaterial
	}
//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);
	// create rank Id
	RankedData data;
	data.element = -1; data.material = -1; data.value = 0;
	tomm_Ranked.assign(h_nElems,data);

	tomm_currentI.assign(h_nElems,0);
	tomm_currentMaterial.assign(h_nElems,0);


	if (tomm_topoptQuantity == STRENGTH){
		for (int elem = 0;elem<h_nElems;elem++){
			tomm_Ranked[elem].element = elem;
			tomm_currentI[elem] = ts_topSensElem[elem];
			tomm_currentMaterial[elem] = getMaterialIdOfElem(elem);
			if (getElemPseudoDensity(elem)<0.5) continue;
			int material = tomm_currentMaterial[elem];
			double dI = tomm_currentI[elem];
			if (h_keepElem[elem]) dI = fabs(dI)*1e8;
			double dC = -tomm_Costs[material]/tomm_CostsSorted[0];
			tomm_Ranked[elem].value = -dI/dC;
			tomm_Ranked[elem].material = -1;
		}
	}
	else{
		for (int elem = 0;elem<h_nElems;elem++){
			tomm_Ranked[elem].element = elem;
			tomm_currentI[elem] = ts_topSensElem[elem];
			tomm_currentMaterial[elem] = getMaterialIdOfElem(elem);
			if (getElemPseudoDensity(elem)<0.5) continue;
			int material = tomm_currentMaterial[elem];
			double dI = tomm_currentI[elem];
			if (h_keepElem[elem]) dI *= 1e8;
			double dC = -tomm_Costs[material]/tomm_CostsSorted[0];
			tomm_Ranked[elem].value = -dI/dC;
			tomm_Ranked[elem].material = -1;
		}
	}
	if (tomm_verbose){
		for (int i = 0;i<h_nElems;i++){
			if (i%5000 != 0) continue;
            ;//printf("%i: value %g material %i\n",tomm_Ranked[i].element,tomm_Ranked[i].value, tomm_Ranked[i].material);
		}
	}
	if (tomm_verbose) printf("---------------------------------------------------\n");
}
/**********************************************************************/
void TopOptForMultiMaterial::computeVoidToSolidRankID()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
    if (tomm_topoptQuantity == STIFFNESS){
		computeTopSensAtElemsApproximate_MultiMaterial(); // Note: using parent function normalizes TS field at wrong place for multimaterial
	}
//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);

	RankedData data; // create rank Id
	data.element = -1; data.material = -1; data.value = 0;
	tomm_Ranked.assign(h_nElems,data);

	tomm_currentI.assign(h_nElems,0);
	tomm_currentMaterial.assign(h_nElems,0);

	// get current values
	for (int elem = 0 ;elem<h_nElems;elem++){
		tomm_currentI[elem] = ts_topSensElem[elem];
		tomm_currentMaterial[elem] = getMaterialIdOfElem(elem);
	}

	if (tomm_topoptQuantity == STRENGTH){
		for (int elem = 0;elem<h_nElems;elem++){
			tomm_Ranked[elem].element = elem;
			tomm_Ranked[elem].material = tomm_currentMaterial[elem];
			// only for void elements
			if (getElemPseudoDensity(elem) > 0.5) continue;

			int material = tomm_currentMaterial[elem];
			double dI = -tomm_currentI[elem];
			double dC = tomm_Costs[material]/tomm_CostsSorted[0];
			tomm_Ranked[elem].value = dI/dC;
			tomm_Ranked[elem].material = material;
		}
	}
	else{
		for (int elem = 0;elem<h_nElems;elem++){
			tomm_Ranked[elem].element = elem;
			tomm_Ranked[elem].material = tomm_currentMaterial[elem];
			// only for void elements
			if (getElemPseudoDensity(elem) > 0.5) continue;

			int material = tomm_currentMaterial[elem];
			double dI = -tomm_currentI[elem];
			double dC = tomm_Costs[material]/tomm_CostsSorted[0];
			tomm_Ranked[elem].value = dI/dC;
			tomm_Ranked[elem].material = material;
		}
	}
	if (tomm_verbose){
		for (int i = 0;i<h_nElems;i++){
			if (i%1000 != 0) continue;
            ;//printf("%i: value %g material %i\n",tomm_Ranked[i].element,tomm_Ranked[i].value, tomm_Ranked[i].material);
		}
	}
	if (tomm_verbose) printf("---------------------------------------------------\n");
}
/**********************************************************************/
void TopOptForMultiMaterial::computeSolidToSolidRankID_Decreasing()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	if (h_nMaterials == 1) return;

	for (int elem = 0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		int material1 = tomm_currentMaterial[elem]; // current state
		int material2 = getMaterialIdOfElem(elem); // possible future state
		if (material1 == material2) continue;

		double dC = (tomm_Costs[material2] - tomm_Costs[material1])/tomm_CostsSorted[0];
		double dI;
		if (dC < 0){
			dI = (tomm_currentI[elem]-tomm_alternateI[elem]);
			double r = -dI/dC;
			if (tomm_Ranked[elem].value > r){
				tomm_Ranked[elem].value = r;
				tomm_Ranked[elem].material = material2;
			}
		}
		else{
			if (fabs(dC) < 1e-6){
				dI = (tomm_currentI[elem]-tomm_alternateI[elem]);
				if (dI > 0) continue;
				if (tomm_Ranked[elem].value > dI){
					tomm_Ranked[elem].value = dI;
					tomm_Ranked[elem].material = material2;
				}
			}
		}
	}
	if (tomm_verbose) printf("---------------------------------------------------\n");

}
/**********************************************************************/
void TopOptForMultiMaterial::computeSolidToSolidRankID_Increasing()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	double SMALL_VAL = 1e-10;
	if (h_nMaterials == 1) return;

	int nChanged = 0;
	for (int elem = 0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		int material1 = tomm_currentMaterial[elem]; // current state
		int material2 = getMaterialIdOfElem(elem); // possible future state
		if (material1 == material2) continue;

		double dC = (tomm_Costs[material2] - tomm_Costs[material1])/tomm_CostsSorted[0];
		double dI = (tomm_currentI[elem] - tomm_alternateI[elem]);
		if (fabs(dC) > 0){
			double r = (dC > 0)? dI/dC : -dI/dC; // Rank = sign(dC)*dI/dC
			if (tomm_Ranked[elem].value > r){
				tomm_Ranked[elem].value = r;
				tomm_Ranked[elem].material = material2;
			}
		}
		else{
			if (fabs(dC) < 1e-6){
				if (dI > 0) continue;
				if (tomm_Ranked[elem].value > dI){
					tomm_Ranked[elem].value = dI;
					tomm_Ranked[elem].material = material2;
				}
			}
		}
	}
	if (tomm_verbose) printf("---------------------------------------------------\n");

}
/**********************************************************************/
bool TopOptForMultiMaterial::reduceCost(double CostDecrement,int maxNumElems)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	for (int mat=0;mat<h_nMaterials;mat++){
		setMaterialIdOfAll(mat);
		findTemplates();

        computeTopSensAtElemsApproximate_MultiMaterial(); // Note: using parent function normalizes TS field at wrong place for multimaterial

	//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);
	

		tomm_alternateI.assign(h_nElems,0);
		for (int elem = 0;elem<h_nElems;elem++){
			if (tomm_topoptQuantity == STRENGTH){
				tomm_alternateI[elem] = ts_topSensElem[elem];
			}
			else{
				tomm_alternateI[elem] = ts_topSensElem[elem];
			}
		}
		computeSolidToSolidRankID_Decreasing();
	}
	// set to original values
	for (int elem = 0;elem<h_nElems;elem++){
		h_materialIdOfElem[elem] = tomm_currentMaterial[elem];
	}
	sortRankIDs(tomm_Ranked);
	swapMaterials(CostDecrement,maxNumElems);
	findTemplates();
	return true;
}
/**********************************************************************/
bool TopOptForMultiMaterial::reduceInefficiency(double CostDecrement,int maxNumElems)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	if (h_nMaterials > 1){
		for (int mat=0;mat<h_nMaterials;mat++){
			setMaterialIdOfAll(mat);
			findTemplates();

            computeTopSensAtElemsApproximate_MultiMaterial(); // Note: using parent function normalizes TS field at wrong place for multimaterial

			//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);

			tomm_alternateI.assign(h_nElems,0);
			for (int elem = 0;elem<h_nElems;elem++){
				if (tomm_topoptQuantity == STRENGTH){
					tomm_alternateI[elem] = ts_topSensElem[elem];
				}
				else{
					tomm_alternateI[elem] = ts_topSensElem[elem];
				}
			}
			computeSolidToSolidRankID_Increasing();
		}
	}
	// set to original values
	for (int elem = 0;elem<h_nElems;elem++){
		h_materialIdOfElem[elem] = tomm_currentMaterial[elem];
	}

	sortRankIDs(tomm_Ranked);
	swapMaterials_ReduceInefficiency(CostDecrement,maxNumElems);
	findTemplates();
	return true;
}
/**********************************************************************/
void TopOptForMultiMaterial::swapMaterials_ReduceInefficiency(double CostDecrement,int maxNumElems )
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	double dcost = 0.0;
	tomm_isChanged.assign(h_nElems,false);

	double DeltaCost = CostDecrement * tomm_initalCost;

	if (tomm_verbose)printf("CostDecrement = %g, DeltaCost = %g\n",CostDecrement,DeltaCost);

	//
	int numChangedElems = 0;

	int counter = -1;
	for (int i=0;i<h_nElems;i++){
		counter++;
		int currentElem = tomm_Ranked[i].element;
		int alternateMaterial = tomm_Ranked[i].material;
		int currentMaterial = tomm_currentMaterial[currentElem]; 
		if (currentElem < 0) continue;

		if (tomm_isChanged[currentElem]) continue;

		if (getElemPseudoDensity(currentElem) < 0.5){
			setPseudoDensityOfElem(currentElem,1.0);
			tomm_isChanged[currentElem] = true;
			int nChangedElems = imposeManufacturingConstraintsOnElem(currentElem);
			dcost += (nChangedElems+1)*tomm_Costs[currentMaterial];
			numChangedElems += nChangedElems;
		}
		else{
			if (h_nMaterials ==1) continue;
			setMaterialIdOfElem(alternateMaterial,currentElem);
			tomm_isChanged[currentElem] = true;
			int nChangedElems = 0;
			if (tomm_materialSymmetry){
				nChangedElems = imposeManufacturingConstraintsOnElem(currentElem);
			}
			dcost += (nChangedElems+1)*fabs(tomm_Costs[currentMaterial] - tomm_Costs[alternateMaterial]);
			numChangedElems += nChangedElems;
		}

		if (dcost > DeltaCost) break;

		// 
		if (numChangedElems > maxNumElems) break;
	}
	if (tomm_verbose) printf("%i elements swapped with dcost = %g \n",counter,dcost);
	if (tomm_verbose) printf("---------------------------------------------------\n");

}
/**********************************************************************/
int TopOptForMultiMaterial::iterateToCost_MultiMaterial(double CostDecrement, double &objectiveConverged,int *nFEA)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	// store previous design on Pareto
	vector<double> convergedPseudoDensity(h_nElems,0);
	vector<int> convergedMaterial(h_nElems,-1);

	for (int elem=0;elem<h_nElems;elem++){
		convergedPseudoDensity[elem] = getElemPseudoDensity(elem);
		convergedMaterial[elem] = getElemMaterialID(elem);
	}
	double *convergerdSol = (double *)malloc(3*h_nNodes*sizeof(double));
	getSolutionVector(convergerdSol);

	double covergedCost = getCurrentCostFraction_MultiMaterial();
	double targetCost = covergedCost - CostDecrement;
	if (tomm_verbose) printf("stored converged solution: Norm Sol = %g \n",Norm(convergerdSol));
	// design of previous substep
	vector<double> lastIterationPseudoDensity = convergedPseudoDensity;
	vector<int> lastIterationMaterial = convergedMaterial;

	// design of current substep
	vector<double> currentIterationPseudoDensity(h_nElems,0);
	vector<int> currentIterationMaterial(h_nElems,0);

	int nSubsteps = tomm_numSupSteps;
	double dcost = CostDecrement;
	double lastIterationObjective = 0;
	for (int substep = 0; substep < nSubsteps+1; substep++){
		if (tomm_verbose) printf("subset %i\n",substep);
		//dcost = CostDecrement / (double)nSubsteps;
		// reduce cost
		computeSolidToVoidRankID();
		reduceCost(dcost);
		removeDisconnectedElements_MultiMaterial(0.5);

		deleteStaticFEAResults();
		findNeighboringElemsOfNodes();
		findTemplates();

		if (tomm_verbose) 
			printf("reduced cost fraction to %g\n",computeCurrentCost()/tomm_initalCost);
		// update current
		for (int elem=0;elem<h_nElems;elem++){
			currentIterationPseudoDensity[elem] = getElemPseudoDensity(elem);
			currentIterationMaterial[elem] = getElemMaterialID(elem);
		}

		// solve
        if(!solvePrimaryAndAdjointProblems_MultiMaterial(nFEA)){
			printf("iterato to cost: Reducing cost FEA Failure!\n"); 
			return ITER_TO_VOL_FEA_FAILURE;
		}
        objectiveConverged = computeCompliance();
		// update last iteration
		int nChanges = 0;
		for (int elem=0;elem<h_nElems;elem++){
			if (lastIterationPseudoDensity[elem] != currentIterationPseudoDensity[elem]){
				nChanges++;
				continue;
			}
			if (lastIterationMaterial[elem] != currentIterationMaterial[elem]){
				nChanges++;
				continue;
			}
		}
		if (tomm_verbose) 
			printf("\n substep = %i, dcost = %g, #changes = %i\n",substep,dcost,nChanges);

		if (double(nChanges) < 0.01*CostDecrement*h_nElems) break;
		if (substep == nSubsteps) break;
		lastIterationObjective = objectiveConverged;
		lastIterationPseudoDensity = currentIterationPseudoDensity;
		lastIterationMaterial = currentIterationMaterial;
		// reduce inefficiancy
		dcost *= 0.5;
		if (tomm_verbose) printf("reduce inefficiancy ...\n");
		computeVoidToSolidRankID();
		reduceInefficiency(dcost);
		removeDisconnectedElements_MultiMaterial(0.5);

		deleteStaticFEAResults();
		findNeighboringElemsOfNodes();
		findTemplates();

		// update current
		for (int elem=0;elem<h_nElems;elem++){
			currentIterationPseudoDensity[elem] = getElemPseudoDensity(elem);
			currentIterationMaterial[elem] = getElemMaterialID(elem);
		}
		if (tomm_verbose) 
			printf("reduced inefficiancy to cost %g\n",computeCurrentCost()/tomm_initalCost);
		// solve
        if(!solvePrimaryAndAdjointProblems_MultiMaterial(nFEA)){
			printf("iterato to cost: Reducing inefficiency FEA Failure!\n"); 
			return ITER_TO_VOL_FEA_FAILURE;
		}
	}
	safeFree(convergerdSol);
	return SUCCESS;
}
/**********************************************************************/
void TopOptForMultiMaterial::sortRankIDs(vector<RankedData> &Ranked)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	vector<double> rankIDs(h_nElems,0);
	vector<int> sortIndex(h_nElems,0);
	vector<RankedData> RankDataInput = Ranked;

	for (int i = 0;i<h_nElems;i++){
		rankIDs[i] = Ranked[i].value;
		sortIndex[i] = i;
	}

	sort(sortIndex.begin(),sortIndex.end(),[&rankIDs](size_t i1, size_t i2) {return rankIDs[i1] < rankIDs[i2];});

	for (int i=0;i<h_nElems;i++){
		int elem = sortIndex[i];
		Ranked[i] = RankDataInput[elem];
	}
}
/**********************************************************************/
double TopOptForMultiMaterial::getCurrentCostFraction_MultiMaterial()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	return(computeCurrentCost()/tomm_initalCost);
}
/**********************************************************************/
int TopOptForMultiMaterial::solvePrimaryAndAdjointProblems_MultiMaterial(int *nFEA)
{

    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
    tomm_nSmoothing += floor(h_nElems/50000);// heuristic
	//
	double *h_solSave = (double *)malloc(3*h_nNodes*sizeof(double));// just in case solution fails, store previous solution
	memset(h_solSave,0,3*h_nNodes*sizeof(double));

	if (!isStaticFEAValid()) {// static results invalid
		getSolutionVector(h_solSave);// save the primary solution
		initializeSolution();
		*nFEA +=1;
        if (tomm_verbose) cout << "TopOpt: Solving primary " << endl;
		if (!solveElasticityStatic()){
			printf("\n Primary FEA solving error!\n");
			setSolutionAtAllNodes(h_solSave);
			safeFree(h_solSave);
			return FAILURE; 
		}

		postprocess();

		getSolutionVector(h_solSave);// save the primary solution

		if (tomm_verbose) 
			printf("Norm(f) = %g,Norm(sol) = %g,computeCompliance= %g\n",Norm(h_f),Norm(h_solSave),computeCompliance());

		ts_compliance = computeCompliance();
	}
	safeFree(h_solSave);
	return SUCCESS;
}

/**********************************************************************/
bool TopOptForMultiMaterial::getMultiMaterialVerbose(void)
{
	return tomm_verbose;
}
/**********************************************************************/
void TopOptForMultiMaterial::getMultiMaterialQuantityOfInterest(TOPOPT_OBJECTIVES &quantity)
{
	quantity = tomm_topoptQuantity;
}

/**********************************************************************/
void TopOptForMultiMaterial::getMultiMaterialNodeDeformation(int node,double *u,double *v,double *w,double *MultiMaterial_stress)
{
	getNodeDeformation(node,u,v,w,MultiMaterial_stress);
}
/**********************************************************************/
bool TopOptForMultiMaterial::getMultiMaterialElasticitySolutionAt(Pt &pt,Vec &uvw,double stresses[6])
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	int elem,nodes[8],node;
	if (!h_voxelgrid) return false;
	double vonMises,P[3],UVW[3];
	P[0] = pt.x;
	P[1] = pt.y;
	P[2] = pt.z;
	getElasticitySolutionAt(P,UVW,stresses,&vonMises);
	uvw.i = UVW[0];
	uvw.j = UVW[1];
	uvw.k = UVW[2];

	elem = findNonVoidElemClosestTo(pt.x,pt.y,pt.z,3); // within relative tolerance
	if ((elem < 0) || (elem >= h_nElems)) return false;
	getElemNodes(elem,nodes);

	//return true;// if we return here, it will be a crude approximation
    double xietamu[3],x[8],y[8],z[8],N[8];
	//get the element corners
	for (int m = 0;m < 8; m++) {
		x[m] = h_x[nodes[m]];
		y[m] = h_y[nodes[m]];
		z[m] = h_z[nodes[m]];
	}
	// find the normalized coordinates of point
	xietamu[0] = 2*(pt.x - (x[1]+x[0])/2.0)/(x[1] - x[0]);
	xietamu[1] = 2*(pt.y - (y[2]+y[1])/2.0)/(y[2] - y[1]);
	xietamu[2] = 2*(pt.z - (z[4]+z[0])/2.0)/(z[4] - z[0]);

	for (int m =0; m <3; m++)
		xietamu[m] = MAX(-1,MIN(xietamu[m],1));

	computeN(xietamu[0],xietamu[1],xietamu[2],N);

	return true;
}
/**********************************************************************/
void TopOptForMultiMaterial::setTopOptMultiMaterialToCounter(int counter)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	int elem;
#pragma omp parallel for private(elem)
	for (elem = 0; elem < h_nElems;elem++){
		setPseudoDensityOfElem(elem,tomm_PsudoDensitySaved[counter][elem]);
		setMaterialIdOfElem(tomm_MaterialIdsSaved[counter][elem],elem);
	}
	findTemplates();
}
/**********************************************************************/
void TopOptForMultiMaterial::saveTopOptMultiMaterial()
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	tomm_currentIsoCounter++;

	vector<double> pseudoDensity(h_nElems,0);
	vector<int> materialId(h_nElems,-1);

	tomm_CostFractionsSaved.push_back(getCurrentCostFraction_MultiMaterial());
	int elem;
#pragma omp parallel for private(elem)
	for (elem = 0; elem < h_nElems;elem++) {
		pseudoDensity[elem] = getElemPseudoDensity(elem);
		materialId[elem] = getMaterialIdOfElem(elem);
	}

	tomm_PsudoDensitySaved.push_back(pseudoDensity);
	tomm_MaterialIdsSaved.push_back(materialId);

}
/**********************************************************************/
bool TopOptForMultiMaterial::pushMultiMaterialInitialStaticSolutionToTessellation(void)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	printf("Number of triangles = %d, number of points = %d \n",h_numTesselationTriangles,h_numTesselationPoints);

	if (h_numTesselationPoints == 0) return false;
	for (int i = 0; i < h_numTesselationPoints; i++) {
        double stresses[6];
		Pt pt;
		Vec uvw;
		pt.x = h_tessellationXYZ[3*i];
		pt.y = h_tessellationXYZ[3*i+1];
		pt.z = h_tessellationXYZ[3*i+2];
        getMultiMaterialElasticitySolutionAt(pt,uvw,stresses);
	}
	return true;
}
/**********************************************************************/
bool TopOptForMultiMaterial::getMultiMaterialTessellationElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3],double FI[3],double SR[3],double Gamma[3])
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;

	int nodes[3];
	if ((tri < 0) || (tri >= h_numTesselationTriangles)) return false;

	for (int i = 0; i < 3; i++)
		nodes[i] = h_tessellationTriangles[3*tri+i];

	for (int i = 0; i < 3; i++){
		u[i] = h_tessellationUVW[3*nodes[i]];
		v[i] = h_tessellationUVW[3*nodes[i]+1];
		w[i] = h_tessellationUVW[3*nodes[i]+2];
	}
	return true;
}
/**********************************************************************/
bool TopOptForMultiMaterial::pushMultiMaterialFinalStaticSolutionToIsoSurface(void)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	double stresses[6],FI,SR,Gamma;
	Pt pt;
	Vec uvw;
	int isoCounter;

	/*
	isoCounter = getCurrentIsoCounter();

	if (isoCounter < 0) return false;
	if (ts_isoTriangles[isoCounter] == 0) return false;

	for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) {
		pt.x = ts_isoXYZ[isoCounter][3*i];
		pt.y = ts_isoXYZ[isoCounter][3*i+1];
		pt.z = ts_isoXYZ[isoCounter][3*i+2];
		getMultiMaterialElasticitySolutionAt(pt,uvw,stresses,FI,SR,Gamma);	

		tomm_FinalFI[i] = FI;
		tomm_FinalSR[i] = SR;
		tomm_FinalGamma[i] = Gamma;
	}
	*/
	return true;
}
/**********************************************************************/
bool TopOptForMultiMaterial::getMultiMaterialFinalIsoElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3])
{
	int nodes[3],isoCounter;
/*
	isoCounter = getCurrentIsoCounter();
	if (isoCounter < 0) return false;
	if (ts_isoTriangles[isoCounter] == 0) return false;

	for (int i = 0; i < 3; i++)
		nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

	for (int i = 0; i < 3; i++){
		u[i] = ts_isoUVWFinal[3*nodes[i]];
		v[i] = ts_isoUVWFinal[3*nodes[i]+1];
		w[i] = ts_isoUVWFinal[3*nodes[i]+2];

	}
    */
	return true;
	
}
/**********************************************************************/
bool TopOptForMultiMaterial::getMultiMaterialElasticitySolutionInCenterOfElem(int elem,double uvw[3],double stresses[6],double *MultiMaterial_stress)
{
	return(getElasticitySolutionInCenterOfElem(elem,uvw,stresses,MultiMaterial_stress));
}
/**********************************************************************/
void TopOptForMultiMaterial::pushToSavedData_MultiMaterial(void)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
     if (tomm_verbose) cout << " tomm_ParetoCounter = " << tomm_ParetoCounter << endl;

	memcpy(ts_backupData.topSensElem,ts_topSensElem,h_nElems*sizeof(double));
	memcpy(ts_backupData.topSensNode,ts_topSensNode,h_nNodes*sizeof(double));

	if (isSavingMultiMaterialDesign(tomm_savePerStep)) {
        ts_currentIsoCounter++;
        if (tomm_verbose) cout << "Saving at ts_currentIsoCounter  " << ts_currentIsoCounter << endl;
		ts_volFracSave[ts_currentIsoCounter] = getCurrentCostFraction_MultiMaterial();
	}
}
/**********************************************************************/
void TopOptForMultiMaterial::setMultiMaterialSavePerStep(int savePerStep)
{
	tomm_savePerStep = savePerStep;
}
/**********************************************************************/
void TopOptForMultiMaterial::retrieveFromSavedData_MultiMaterial(int *nFEA)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	findNeighboringElemsOfNodes();
	findTemplates();
    solvePrimaryAndAdjointProblems_MultiMaterial(nFEA);

    computeTopSensAtElemsApproximate_MultiMaterial();

//connectivityTSSmoothener();
	//smoothenElemField(ts_topSensElem,tomm_nSensSmootheningIter);
	smoothenSensitivty_MultiMaterial(tomm_nSensSmootheningIter);
}
/**********************************************************************/
void TopOptForMultiMaterial::getMultiMaterialHistoryData(size_t &size,vector<double> &volFractions,vector<double> &Compliance,vector<double> &maxStress,vector<double> &Stress_pNorm)
{
    if (tomm_verbose) cout << ">>Entering " << __func__ << endl;
	size =  tomm_HistoryData.costFraction.size();
	volFractions = tomm_HistoryData.costFraction;
	Compliance = tomm_HistoryData.compliance;
	maxStress = tomm_HistoryData.stress;
	Stress_pNorm = tomm_HistoryData.objective;
}
/**********************************************************************/
void TopOptForMultiMaterial::findCrossSectionPlane(int plane, int value)
{
	Pt xyzMin,xyzMax,pt1,pt2,pt3;
	Vec normal,edge1,edge2;
	double dtprd;

	double pts[3][8];

	int faces[4][6];

	double sectionX[4],sectionY[4],sectionZ[4];

	vector<Pt> crossSectionPlane;
	Pt crossSectionCenter;
	Vec XSectionDir;
	switch (plane)
	{
	case 0:
		return;
	case 1://+X
		XSectionDir.i = 1;
		XSectionDir.j = 0;
		XSectionDir.k = 0;
		break;
	case 2://+Y
		XSectionDir.i = 0;
		XSectionDir.j = 1;
		XSectionDir.k = 0;
		break;
	case 3://+Z
		XSectionDir.i = 0;
		XSectionDir.j = 0;
		XSectionDir.k = 1;
		break;
	case -1://-X
		XSectionDir.i = -1;
		XSectionDir.j = 0;
		XSectionDir.k = 0;
		break;
	case -2://-Y
		XSectionDir.i = 0;
		XSectionDir.j = -1;
		XSectionDir.k = 0;
		break;
	case -3://-Z
		XSectionDir.i = 0;
		XSectionDir.j = 0;
		XSectionDir.k = -1;
		break;
	}


	double boundingBox[3][2];
	getBoundingBox(boundingBox);

	xyzMin.x = boundingBox[0][0];
	xyzMin.y = boundingBox[1][0];
	xyzMin.z = boundingBox[2][0];

	xyzMax.x = boundingBox[0][1];
	xyzMax.y = boundingBox[1][1];
	xyzMax.z = boundingBox[2][1];

	pts[0][0] = xyzMin.x; pts[1][0] = xyzMin.y; pts[2][0] = xyzMin.z;
	pts[0][1] = xyzMax.x; pts[1][1] = xyzMin.y; pts[2][1] = xyzMin.z;
	pts[0][2] = xyzMin.x; pts[1][2] = xyzMax.y; pts[2][2] = xyzMin.z;
	pts[0][3] = xyzMax.x; pts[1][3] = xyzMax.y; pts[2][3] = xyzMin.z;
	pts[0][4] = xyzMin.x; pts[1][4] = xyzMin.y; pts[2][4] = xyzMax.z;
	pts[0][5] = xyzMax.x; pts[1][5] = xyzMin.y; pts[2][5] = xyzMax.z;
	pts[0][6] = xyzMin.x; pts[1][6] = xyzMax.y; pts[2][6] = xyzMax.z;
	pts[0][7] = xyzMax.x; pts[1][7] = xyzMax.y; pts[2][7] = xyzMax.z;

	faces[0][0] = 0; faces[1][0] = 1; faces[2][0] = 3; faces[3][0] = 2;
	faces[0][1] = 4; faces[1][1] = 6; faces[2][1] = 7; faces[3][1] = 5;
	faces[0][2] = 0; faces[1][2] = 4; faces[2][2] = 5; faces[3][2] = 1;
	faces[0][3] = 2; faces[1][3] = 3; faces[2][3] = 7; faces[3][3] = 6;
	faces[0][4] = 0; faces[1][4] = 2; faces[2][4] = 6; faces[3][4] = 4;
	faces[0][5] = 1; faces[1][5] = 5; faces[2][5] = 7; faces[3][5] = 3;
	for (int face = 0;face<6;face++){
		pt1.x = pts[0][faces[0][face]];pt1.y = pts[1][faces[0][face]];pt1.z = pts[2][faces[0][face]];
		pt2.x = pts[0][faces[1][face]];pt2.y = pts[1][faces[1][face]];pt2.z = pts[2][faces[1][face]];
		pt3.x = pts[0][faces[3][face]];pt3.y = pts[1][faces[3][face]];pt3.z = pts[2][faces[3][face]];

		edge1 = Normalize3D_Vectors(Subtract3D_Points(pt1,pt2));
		edge2 = Normalize3D_Vectors(Subtract3D_Points(pt1,pt3));

		normal  = Normalize3D_Vectors(CrossProd_Vectors(edge1,edge2));

		dtprd = DotProd_Vectors(normal,XSectionDir);
		if (dtprd<0.9) continue;
		for(int i=0;i<4;i++){
			sectionX[i] = pts[0][faces[i][face]];
			sectionY[i] = pts[1][faces[i][face]];
			sectionZ[i] = pts[2][faces[i][face]];
		}
		break;
	}
	
	for(int i=0;i<4;i++){
		Pt p;
		p.x = sectionX[i];
		p.y = sectionY[i];
		p.z = sectionZ[i];
		crossSectionPlane.push_back(p);
	}


	// translate 
	int grid[3];
	grid[0] = h_nelx;
	grid[1] = h_nely;
	grid[2] = h_nelz;


	double dsize[3];
	dsize[0] = h_dx;
	dsize[1] = h_dy;
	dsize[2] = h_dz;

	switch (plane)
	{
	case 1: //+X
		value = min(value,grid[0]);
		crossSectionPlane[0].x += value * dsize[0];
		crossSectionPlane[1].x += value * dsize[0];
		crossSectionPlane[2].x += value * dsize[0];
		crossSectionPlane[3].x += value * dsize[0];
		break;
	case 2: //+Y
		value = min(value,grid[1]);
		crossSectionPlane[0].y += value * dsize[1];
		crossSectionPlane[1].y += value * dsize[1];
		crossSectionPlane[2].y += value * dsize[1];
		crossSectionPlane[3].y += value * dsize[1];
		break;
	case 3: //+Z
		value = min(value,grid[2]);
		crossSectionPlane[0].z += value * dsize[2];
		crossSectionPlane[1].z += value * dsize[2];
		crossSectionPlane[2].z += value * dsize[2];
		crossSectionPlane[3].z += value * dsize[2];
		break;
	case -1: //-X
		value = min(value,grid[0]);
		crossSectionPlane[0].x -= (h_nelx - value) * dsize[0];
		crossSectionPlane[1].x -= (h_nelx - value)  * dsize[0];
		crossSectionPlane[2].x -= (h_nelx - value)  * dsize[0];
		crossSectionPlane[3].x -= (h_nelx - value)  * dsize[0];
		break;
	case -2: //-Y
		value = min(value,grid[1]);
		crossSectionPlane[0].y -= (h_nely - value) * dsize[1];
		crossSectionPlane[1].y -= (h_nely - value) * dsize[1];
		crossSectionPlane[2].y -= (h_nely - value) * dsize[1];
		crossSectionPlane[3].y -= (h_nely - value) * dsize[1];
		break;
	case -3: //-Z
		value = min(value,grid[2]);
		crossSectionPlane[0].z -= (h_nelz - value) * dsize[2];
		crossSectionPlane[1].z -= (h_nelz - value) * dsize[2];
		crossSectionPlane[2].z -= (h_nelz - value) * dsize[2];
		crossSectionPlane[3].z -= (h_nelz - value) * dsize[2];
		break;
	}
	
	crossSectionCenter.x = crossSectionCenter.y = crossSectionCenter.z = 0;  
	for (int i=0;i<4;i++){
		crossSectionCenter.x += 0.25*crossSectionPlane[i].x;
		crossSectionCenter.y += 0.25*crossSectionPlane[i].y;
		crossSectionCenter.z += 0.25*crossSectionPlane[i].z;
	}

	tomm_CrossSectionPlane = crossSectionPlane;
	tomm_CrossSectionCenter = crossSectionCenter;

	tomm_isCrossSectionAvailable = true;
}
/**********************************************************************/
bool TopOptForMultiMaterial::getCrossSectionPlane(vector<Pt> &crossSectionPlane,Pt &crossSectionCenter)
{
	if(!tomm_isCrossSectionAvailable) return false; 

	crossSectionPlane = tomm_CrossSectionPlane;
	crossSectionCenter = tomm_CrossSectionCenter;

	return true;
}
/************************************************************/
void TopOptForMultiMaterial::imposeCrossSection(int plane, int value)
{
	if (plane == 0){
		tomm_isCrossSectionAvailable = false;
		tomm_CrossSectionPlane.clear();
		tomm_showElem.assign(h_nElems,true);
		return;
	}

	findCrossSectionPlane(plane,value);

	vector<bool>showNode(h_nNodes,true);
	tomm_showElem.assign(h_nElems,true);
	int gridnode,node;
	switch (plane) {
	case 1: //+x
		for (int nx = value; nx <= h_nelx; nx++){// start from the bottom
			for (int ny = 0; ny <= h_nely; ny++){
				for (int nz = 0; nz <= h_nelz; nz++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	case -1: //-x
		for (int nx = value; nx >= 0 ; nx--){// start from the 2nd layer of nodes
			for (int ny = 0; ny <= h_nely; ny++){
				for (int nz = 0; nz <= h_nelz; nz++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	case 2: //+y
		for (int ny = value; ny <= h_nely; ny++){// start from the bottom
			for (int nx = 0; nx <= h_nelx; nx++){
				for (int nz = 0; nz <= h_nelz; nz++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	case -2: //-y
		for (int ny = value; ny >= 0; ny--){// start from the 2nd layer of nodes
			for (int nx = 0; nx <= h_nelx; nx++){
				for (int nz = 0; nz <= h_nelz; nz++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	case 3: //+z
		for (int nz = value; nz <= h_nelz; nz++){// start from the 2nd layer of nodes
			for (int nx = 0; nx <= h_nelx; nx++){
				for (int ny = 0; ny <= h_nely; ny++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	case -3: //-z
		for (int nz = value; nz >= 0; nz--){// start from the 2nd layer of nodes
			for (int nx = 0; nx <= h_nelx; nx++){
				for (int ny = 0; ny <= h_nely; ny++){	
					gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
					if (gridnode < 0 || gridnode >= (h_nelz + 1)*(h_nely+1)*(h_nelx+1)) continue;
					node = h_gridNodeToRealNode[gridnode];
					if (node < 0) continue;
					showNode[node] = false;
				}
			}
		}
		break;
	}

	// find shown elements
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem) < 0.5) {
			tomm_showElem[elem] = false;
			continue;
		}
		int nodes[8];
		getElemNodes(elem,nodes);
		for (int k=0;k<8;k++){
			if (!showNode[nodes[k]]) {
				tomm_showElem[elem] = false;
				break;
			}
		}
	}
}
/**********************************************************************/
bool TopOptForMultiMaterial::isElementShown(int elem)
{
	return (tomm_showElem[elem]);
}
/**********************************************************************/
int TopOptForMultiMaterial::imposeManufacturingConstraintsOnElem(int elem)
{
	int numElemsChanged = 0;
    if (ts_extrudeDirection > 0){
        numElemsChanged = imposeExtrudeOnElem(elem);
	}
	else if (tomm_nSymmetry > 0){
		numElemsChanged = imposeSymmetryOnElem(elem);
	}

	return numElemsChanged;
}
/**********************************************************************/
void TopOptForMultiMaterial::setSymmetry_MultiMaterial(int symmetryIndex)
{
	if (tomm_verbose) printf("setting symmetry ...\n");
	tomm_symmetryIndex = symmetryIndex;
	// find nSymmetry for updating cost while imposing symmetry
	if (tomm_symmetryIndex==1 || tomm_symmetryIndex==2 || tomm_symmetryIndex==4)
		tomm_nSymmetry = 1.0;
	else if (tomm_symmetryIndex==3 || tomm_symmetryIndex==5 || tomm_symmetryIndex==6)
		tomm_nSymmetry = 2.0;
	else if (tomm_symmetryIndex==7)
		tomm_nSymmetry = 3.0;
	else
		tomm_nSymmetry = 0.0;

	if (tomm_verbose) printf("---------------------------------------------------\n");
}
/**********************************************************************/
int TopOptForMultiMaterial::imposeSymmetryOnElem(int elem)
{
	if (tomm_nSymmetry == 0) return 0;

	int otherGridElem;
	int matID = getMaterialIdOfElem(elem);
	double pseudodensity = getElemPseudoDensity(elem);
	double center[3];
	getElemCenter(elem,center);
	double x = center[0];
	double y = center[1];
	double z = center[2];

	double cx = h_xMin + 0.5*h_nelx*h_dx;
	double cy = h_yMin + 0.5*h_nely*h_dy;
	double cz = h_zMin + 0.5*h_nelz*h_dz;
	double xMirror = x + 2*(cx-x);
	double yMirror = y + 2*(cy-y);
	double zMirror = z + 2*(cz-z);

	int numElemsChanged = 0;

	if (tomm_symmetryIndex == 1){
		// only x
		otherGridElem = findElemWithCenter(xMirror,y,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			 numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 2){
		// only y
		otherGridElem = findElemWithCenter(x,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 4){
		// only z
		otherGridElem = findElemWithCenter(x,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems){
			printf("otherGridElem = %i\n",otherGridElem);	
			return numElemsChanged;
		}
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 3){
		// only x , y
		otherGridElem = findElemWithCenter(xMirror,y,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return  numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return  numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(xMirror,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 5){
		// only x , z
		otherGridElem = findElemWithCenter(xMirror,y,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(xMirror,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 6){
		// only y , z
		otherGridElem = findElemWithCenter(x,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,yMirror,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	else if (tomm_symmetryIndex == 7){
		// x, y , z
		otherGridElem = findElemWithCenter(xMirror,y,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(xMirror,yMirror,z);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(xMirror,y,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(x,yMirror,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
		otherGridElem = findElemWithCenter(xMirror,yMirror,zMirror);
		if (otherGridElem<0 || otherGridElem >= h_nElems) return numElemsChanged;
		if (!tomm_isChanged[otherGridElem]) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			if (tomm_materialSymmetry) setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged++;
		}
	}
	return numElemsChanged;
}
/**********************************************************************/
void  TopOptForMultiMaterial::imposeCyclicSymmetry_MultiMaterial(int axiSymmetry)
{
		// axiSymmetry = 0, nothing
	// axiSymmetry = 1, full symmetry
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
	if ( axiSymmetry== 0) return;

	pi = 3.14159265359;
	// assumes z axis for axi symmetry
	
	if (axiSymmetry == 1)
		sectorAngle = 0.1*pi/180;// really small steps
	else
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
			// take the average of the TS values of all theses node and assign to each of them
			nodeSet.clear();
			for (double z = boundingBox[2][0]-0.1*h_dz; z <= boundingBox[2][1]+0.1*h_dz; z += h_dz){// walk through every node layer in z
				for (double theta = startAngle; theta < 2*pi;  theta += sectorAngle) {// pick up all nodes that are apart by specified anfle
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
///**********************************************************************/
int  TopOptForMultiMaterial::imposeExtrudeOnElem(int elem)
// Ensure that the topology can be laset-cut in the given direction
// dir = +/-1,+/- 2,+/- 3 (x,y,z). 
{

	int dir = 3;
	//if  ((dir < 1) || (dir >3)) return;

	int otherGridElem;
	int matID = getMaterialIdOfElem(elem);
	double pseudodensity = getElemPseudoDensity(elem);

	double elemCenter[3];
	getElemCenter(elem,elemCenter);
	double elemX = elemCenter[0];
	double elemY = elemCenter[1];
	double elemZ = elemCenter[2];

	double boundingBox[3][2];
	getBoundingBox(boundingBox);
	double zMin = boundingBox[2][0];
	double zMax = boundingBox[2][1];

	int numElemsChanged = 0;
	for (double z = zMin+0.5*h_dz; z <= zMax-0.5*h_dz; z += h_dz){
		otherGridElem = findElemWithCenter(elemX,elemY,z);
		if (otherGridElem < 0 || otherGridElem >= h_nElems) {
			cout << "not found!" << endl;
			continue;
		}
		if (!tomm_isChanged[otherGridElem]) {
			if (pseudodensity < 0.5 && h_keepElem[otherGridElem]) continue;
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			tomm_isChanged[otherGridElem] = true;
			numElemsChanged ++;
		}
	}
    return numElemsChanged;
}
///**********************************************************************/
//void  TopOpt::imposeDrawDirectionOnElem(int element)
//// Ensure that the topology can be 'drawn' in the given direction
//// dir = +/-1,+/- 2,+/- 3 (x,y,z). 
//{
//	int *nodes,nNodesAlongDir,i;
//	int maxN = MAX(MAX(h_nelx+1,h_nely+1),h_nelz+1),minNodeLoc,maxNodeLoc;
//	double ts_TSMinAlongDir,ts_TSMaxAlongDir,TSCurrent;
//
//	if  ((dir < 1) || (dir >3)) return;
//
//	nodes = (int *)malloc(maxN*sizeof(int));
//	int nx,ny,nz;
//	// depending on the direction, sweep through appropriate plane
//	for (nx = 0; nx <= h_nelx; nx++){
//		for (ny = 0; ny <= h_nely; ny++){
//			for (nz = 0; nz <= h_nelz; nz++){
//				nNodesAlongDir = findNodesAlongDirection(nx,ny,nz,dir,nodes);
//				// find the location and value of the lowest and max TS value
//				//printf("nNodesAlongDir= %d \n",nNodesAlongDir);getchar();
//				ts_TSMinAlongDir = 1e12;
//				ts_TSMaxAlongDir = -1e12;
//				if (nNodesAlongDir == 0)
//					continue;
//				for (i = 0; i < nNodesAlongDir; i++){
//					if (ts_topSensNode[nodes[i]] < ts_TSMinAlongDir){
//						ts_TSMinAlongDir = ts_topSensNode[nodes[i]];
//						minNodeLoc = i;
//					}
//					if (ts_topSensNode[nodes[i]] > ts_TSMaxAlongDir){
//						ts_TSMaxAlongDir = ts_topSensNode[nodes[i]];
//						maxNodeLoc = i;
//					}
//				}
//				// now depending on the location 
//				TSCurrent = 0;
//				for (i = 0; i < maxNodeLoc; i++){
//					if (ts_topSensNode[nodes[i]] < TSCurrent){
//						ts_topSensNode[nodes[i]] = TSCurrent;
//						minNodeLoc = i;
//					}
//					else
//						TSCurrent = ts_topSensNode[nodes[i]];
//				}
//				TSCurrent = 0;
//				for (i =nNodesAlongDir-1; i > maxNodeLoc; i--){
//					if (ts_topSensNode[nodes[i]] < TSCurrent){
//						ts_topSensNode[nodes[i]] = TSCurrent;
//						minNodeLoc = i;
//					}
//					else
//						TSCurrent = ts_topSensNode[nodes[i]];
//				}
//			}
//		}
//	}
//	free(nodes);
//}
/**********************************************************************/
TopOptForMultiMaterial::~TopOptForMultiMaterial()
{
}


