#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string> 
#include <fstream> 
#include <iostream> 
#ifdef _WIN32
#include <omp.h>
#endif
#include "cmatrix.h"
#include "Timer.h"
#include "TopOptMultiLoadsMultiConstraints.h"

using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)

/**********************************************************************/
TopOptMultiLoadsMultiConstraints::TopOptMultiLoadsMultiConstraints() 
{
	ts_allConstraintsML_TS = NULL;

	ts_StepNumber_ALM = 0;
	ts_initializeMultiplierFun = true;

	ts_augmentedTS=NULL;//augmented topological sensitivity fields
	ts_objective_TS=NULL;//objective TS field

	ts_nBCSet = 1;
	
}
/************************************************************/
void TopOptMultiLoadsMultiConstraints::InitializeMLMCParameters(void)
{
	ts_initializeMultiplierFun = true;
	ts_StepNumber_ALM = 0;
	ts_objectBCSet = 0;

	ts_useAllLoads = true;
	setTStoAnalytical(true);
	
	for (int counter = 0; counter < MAX_ISO_SAVES; counter++) {
		for(int load = 0; load <MAX_BC_SET;load++){
			ts_Disp[counter][load] = 0;
			for (int part = 0; part < getNumberOfParts(); part++)
				ts_MLMaxStress[counter][load][part] = 0;
		}
	}
	for(int load = 0; load <MAX_BC_SET;load++){
		ts_complianceLastStep[load] = 0.0;
		ts_J[load] = 0;
		for(int j=0; j<MAX_CONSTRAINTS; j++){
			ts_gFunction[load][j] = 0;
			ts_Saved_gFunction[load][j] = 0;

			ts_Sigma[load][j] = 100.0;
			ts_Penalty[load][j] = 5.0; 
		}
	}

	//Topological sensitivity fields
	safeFree(ts_allConstraintsML_TS); ts_allConstraintsML_TS = NULL;
	ts_allConstraintsML_TS = (double **)malloc(MAX_BC_SET*sizeof(double*)); 

	for(int load=0; load<MAX_BC_SET;load++) {
		ts_allConstraintsML_TS[load]=(double *)malloc(MAX_CONSTRAINTS*h_nElems*sizeof(double)); 
		for(int j=0;j<MAX_CONSTRAINTS*h_nElems;j++)
			ts_allConstraintsML_TS[load][j] = 0;
	}

	//augmented topological sensitivity fields
	safeFree(ts_augmentedTS); ts_augmentedTS = NULL;
	ts_augmentedTS = (double *)malloc(h_nElems*sizeof(double));
	memset(ts_augmentedTS,0,h_nElems*sizeof(double));

	//objective TS
	safeFree(ts_objective_TS); ts_objective_TS=NULL;
	ts_objective_TS = (double *)malloc(h_nElems*sizeof(double)); 
	memset(ts_objective_TS,0,h_nElems*sizeof(double));
}
/************************************************************/
double TopOptMultiLoadsMultiConstraints::compute_gfunc_perConstraint_perBCSet(int BCSet, int constraint)
{
	//single scaled g function
	double gFunc,maxStressInPart,yieldStrength;
	int matId;
	int type = ts_constraintType[BCSet][constraint];

	if(type == (int) COMPLIANCE){
		gFunc = getDeformationMax()/ts_maxDisplacementAllowed - 1;//upper limit assumed
		for (int part = 0;  part < getNumberOfParts(); part++) { 
			maxStressInPart = getMaxStressInPart(part);
			matId = getMaterialIDofPart(part);
			getYieldStressOfMaterial(matId,&yieldStrength);
			ts_MLMaxStress[ts_currentIsoCounter][BCSet][part] = maxStressInPart;
			gFunc = MAX(gFunc,maxStressInPart/yieldStrength -1);
			if(ts_StepNumber_ALM == 0)
				ts_MLMaxStress0[BCSet][part] = ts_MLMaxStress[ts_currentIsoCounter][BCSet][part];
		}
	}
	else if(type == (int) VONMISES){
		gFunc = -HUGE_VAL;
		for (int part = 0;  part < getNumberOfParts(); part++) { 
			maxStressInPart = getMaxStressInPart(part);
			matId = getMaterialIDofPart(part);
			getYieldStressOfMaterial(matId,&yieldStrength);
			ts_MLMaxStress[ts_currentIsoCounter][BCSet][part] = maxStressInPart;
			gFunc = MAX(gFunc,maxStressInPart/yieldStrength -1);
			if(ts_StepNumber_ALM == 0)
				ts_MLMaxStress0[BCSet][part] = ts_MLMaxStress[ts_currentIsoCounter][BCSet][part];
		}
	}


	return gFunc;
}
/************************************************************/
double TopOptMultiLoadsMultiConstraints::compute_initialQuantity_perConstraint_perBCSet(int BCSet, int constraint)
{
	double initialValue;
	int type = ts_constraintType[BCSet][constraint];

	if(type == (int) COMPLIANCE){
		initialValue = ts_J0[BCSet];
	}
	else if(type == (int) VONMISES){
		initialValue = ts_MLMaxStress0[BCSet][0];//need to fix this
	}
	else if(type == (int) BUCKLING){
		initialValue = ts_BucklingLoad0[BCSet];
	}
	else if(type == (int) EIGENVALUE){
		initialValue = ts_EigenVaule0[BCSet]; //lowest eigen-value
	}
	else if(type == (int) DISPLACEMENT){
		//ts_Disp0 = ;
	}
	else if(type == (int) OUTPUT_COMPLIANCE){ 
		initialValue = ts_OJ0[BCSet];
	}
	else if(type == (int) GEOMETRIC_ADVANTAGE){
		initialValue = ts_MJ0[BCSet];
	}
	else {
		cout << "Invalid constraint" << endl;
	}
	return initialValue;
}

/**********************************************************************/
double TopOptMultiLoadsMultiConstraints::compute_Quantity_perConstraint_perBCSet(int BCSet, int constraint)
{
	double objective;
	int type = ts_constraintType[BCSet][constraint];

	if(type == (int) COMPLIANCE){
		objective = ts_J[BCSet];
		if(ts_StepNumber_ALM == 0)
			ts_J0[BCSet] = ts_J[BCSet];

		for (int part = 0;  part < getNumberOfParts(); part++) {
			ts_MLMaxStress[ts_currentIsoCounter][BCSet][part] = getMaxStressInPart(part);
			if(ts_StepNumber_ALM == 0)
				ts_MLMaxStress0[BCSet][part] = ts_MLMaxStress[ts_currentIsoCounter][BCSet][part];
		}
	}
	else if(type == (int) VONMISES){
		objective = getMaxVonMisesStress(); 
		for (int part = 0;  part < getNumberOfParts(); part++) {
			ts_MLMaxStress[ts_currentIsoCounter][BCSet][part] = getMaxStressInPart(part);
			if(ts_StepNumber_ALM == 0)
				ts_MLMaxStress0[BCSet][part] = ts_MLMaxStress[ts_currentIsoCounter][BCSet][part];
		}
	}
	else if(type == (int) BUCKLING){
		objective = getBucklingLoad(); //buckling load
		ts_BucklingLoad[BCSet] = objective;
		if(ts_StepNumber_ALM == 0)
			ts_BucklingLoad0[BCSet] = objective;
	}
	else if(type == (int) EIGENVALUE){
		objective = h_eigenValues[0]; //lowest eigen-value
		ts_EigenVaule[BCSet] = objective;
		if(ts_StepNumber_ALM == 0)
			ts_EigenVaule0[BCSet] = objective;
	}
	else if(type == (int) DISPLACEMENT){
		//write
	}
	else if(type == (int) OUTPUT_COMPLIANCE){ 
		objective = computeOutputCompliance(); 
		ts_OJ[BCSet] = objective;
		if(ts_StepNumber_ALM == 0)
			ts_OJ0[BCSet] = objective;
	}
	else if(type == (int) GEOMETRIC_ADVANTAGE){
		objective = computeMutualCompliance(); 
		ts_MJ[BCSet] = objective;
		if(ts_StepNumber_ALM == 0)
			ts_MJ0[BCSet] = objective;
	}
	else 
		cout << "Invalid Constraint" << endl;
	return objective;
}
/************************************************************/
void TopOptMultiLoadsMultiConstraints::compute_InitialMultiplier_loadMagnitude(void)
{
	//scale sigma based on corresponding gfunction value
	double minValue = 1e6;
	double manipulator = 1.25;
	int i_min=0, j_min=0;

	vector<double> gfunc;
	vector<double>::iterator iter; 
	if (ts_verbose) cout << "TopOpt_MLMC: compute_InitialMultiplier_loadMagnitude " << endl;
	for(int i = 0; i < ts_nBCSet; i++){
		for (int j = 0; j < ts_nConstraints[i]; j++){
			if(!ts_isHardConstraint[i][j]) continue; //not consider soft constraint
			gfunc.push_back(ts_gFunction[i][j]); 
			if(ts_gFunction[i][j] < minValue){ //smallest value & position
				minValue = ts_gFunction[i][j];
				i_min = i;
				j_min = j;
			}
		}
	}

	iter=gfunc.begin();
	for(int i = 0; i < ts_nBCSet; i++){
		for (int j = 0; j < ts_nConstraints[i]; j++){
			if(!ts_isHardConstraint[i][j]) continue;
			ts_Sigma[i][j] = ts_Sigma[i_min][j_min]*(MIN(minValue/(*iter), manipulator));
			if (ts_verbose) cout << "TopOpt_MLMC:  ts_Sigma[i][j] = " << ts_Sigma[i][j] << endl;
			iter++;
		}
	}
}
/************************************************************/
void TopOptMultiLoadsMultiConstraints::augmentedTSFields(void) 
{
	double softConsCoeff = 0.1,cutoff,multiplier;

	for (int elem = 0; elem < h_nElems; elem++)
        ts_topSensElem[elem] = ts_objective_TS[elem];// typically the volume


	for(int i = 0; i < ts_nBCSet; i++){
		for (int j = 0; j < ts_nConstraints[i]; j++){
			cutoff = ts_Sigma[i][j]+ts_Penalty[i][j]*ts_gFunction[i][j];
			multiplier = (cutoff > 0)? cutoff:softConsCoeff;// a small component needed to avoid zero TS field
			
			if (ts_verbose) cout << "TopOpt_MLMC: ts_Sigma = " << ts_Sigma[i][j] << " ts_Penalty = " << ts_Penalty[i][j] << ", ts_gFunction = " << ts_gFunction[i][j] << endl;
			if (ts_verbose) cout << "TopOpt_MLMC: bcSet = " << i << ", constraint: " << j << " multiplier " << multiplier << endl;

			if (!ts_isHardConstraint[i][j])  multiplier *= softConsCoeff;
			for (int elem = 0; elem < h_nElems; elem++)
				ts_topSensElem[elem] += ts_allConstraintsML_TS[i][elem+j*h_nElems]*multiplier;
		}
	}

	ts_TSMin = HUGE_VAL;
	ts_TSMax = -HUGE_VAL;
	for (int elem = 0; elem < h_nElems; elem++){
		ts_TSMin = MIN(ts_TSMin,ts_topSensElem[elem]);
		ts_TSMax = MAX(ts_TSMax,ts_topSensElem[elem]);
	}
	for (int elem = 0; elem < h_nElems;elem++) {
		ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin+TOL);// avoid division by zero
		if (h_keepElem[elem])
			ts_topSensElem[elem] = 1.0;
		if (getElemPseudoDensity(elem)< 0.5) 
			ts_topSensElem[elem] = 0;
	}
	ts_TSMax = 1.0; ts_TSMin = 0;
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::onSuccess(void) 
{
	//update constraints & multipliers
	for(int i=0; i < ts_nBCSet; i++){
		ts_complianceLastStep[i] = ts_J[i];
		for (int j = 0; j < ts_nConstraints[i]; j++) {
			ts_Saved_gFunction[i][j] = ts_gFunction[i][j];
		}
	}
	updateSigma();
	ts_StepNumber_ALM++; //update penalty
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::onFailure(void)
{
	//update multipliers & penalty
	updatePenalty();
	updateSigma();
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::updatePenalty(void) 
{
	//update penalty parameter
    double sai = 0.25;
    int r = 10;
	for(int i=0; i < ts_nBCSet; i++){
		for (int j=0; j < ts_nConstraints[i]; j++){
			if (MIN(ts_gFunction[i][j],0) <= sai*MIN(ts_Saved_gFunction[i][j],0)) {
				;//update nothing
			}
			else
				ts_Penalty[i][j] = ts_Penalty[i][j]/r; //ts_Penalty[i][j] = MAX(r*ts_Penalty[i][j],ts_StepNumber_ALM^2);

		}
	}
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::updateSigma(void) 
{
	//update Lagrangian multiplier
	for(int i=0;i < ts_nBCSet; i++){
		for (int j = 0; j < ts_nConstraints[i]; j++) {
			ts_Sigma[i][j] = MAX(0.0, ts_Sigma[i][j] + ts_Penalty[i][j] * ts_gFunction[i][j]);
			if (ts_verbose) cout << "Topopt_MLMC: ts_Sigma[i][j] = " << ts_Sigma[i][j] << endl;
		}
	}
}
/************************************************************/
void TopOptMultiLoadsMultiConstraints::changeTopOptBC(void)
{
	resetBC();
	for (int faceGroup = 0; faceGroup < ts_numTriGroups; faceGroup++)
		setBCOnNodesWithLabel(faceGroup,ts_BCType[ts_currentBCSet][faceGroup],ts_BCValue[ts_currentBCSet][faceGroup]);
}
/**********************************************************************/
bool TopOptMultiLoadsMultiConstraints::checkDispAndStressConstraintsForAllBCSets(int *constraintBCSet)
{
	int partId;
	bool success;

	success = true;
	for (int bcSet = 0; bcSet < ts_nBCSet; bcSet++){
		ts_currentBCSet = bcSet;
		changeTopOptBC(); 
		solveElasticityStatic();
		postprocess();
		ts_Disp[ts_currentIsoCounter][bcSet] = getDeformationMax();
		for (int part = 0;  part < getNumberOfParts(); part++) 
			ts_MLMaxStress[ts_currentIsoCounter][bcSet][part] = getMaxStressInPart(part);
		if (ts_Disp[ts_currentIsoCounter][bcSet] > ts_maxDisplacementAllowed){
			sprintf(ts_lastTopOptErrorMessage, "TopOpt_MLMC: BCSet: %d; displacement of %.2e exceeds allowable %.2e ",bcSet,ts_Disp[ts_currentIsoCounter][bcSet],ts_maxDisplacementAllowed);
			*constraintBCSet = bcSet;
			success = false;
		}
		// stress must be checked via yield
		if ((isDesignPastYield(&partId,ts_stressSafetyFactor))){
			sprintf(ts_lastTopOptErrorMessage, "\nBCSet: %d; part %d is approaching stress limit ",bcSet,partId);
			*constraintBCSet = bcSet;
			success = false;
		}
	}
	return success;
}
/**********************************************************************/
bool TopOptMultiLoadsMultiConstraints::solve_FEA_TS_ML(int* nFEA)
{
    double maxStress[MAX_BC_SET];

    for (int bcSet = 0; bcSet <MAX_BC_SET; bcSet++)
        maxStress[bcSet] = 0.0;

	if(ts_objective == 0){// volume objective, this is typical for multi-load problems
		;//nothing to do
	}
	else { // this is rarely used for multi-objective
		ts_currentBCSet = ts_objectBCSet;
		changeTopOptBC(); 
		ts_topoptQuantity = (TOPOPT_QUANTITY) ts_objective;
		if(!solvePrimaryAndAdjointProblems(nFEA)) {//solve FEA for objective
			cout << "TopOpt_MLMC: FEA error for objective " << endl;
			return false;
		}
		if(ts_StepNumber_ALM == 0){
			computeTopSensInitial(); //initialize objective TS
			ts_currentIsoCounter = -1;	
		}
		computeTopSensAtElems(); //solve for objective TS
		memcpy(ts_objective_TS,ts_topSensElem,h_nElems*sizeof(double));
	}
	//for all constraints
	for(int bcSet = 0; bcSet < ts_nBCSet; bcSet++){
		if (!ts_activeBCSets[bcSet]) continue;

		ts_currentBCSet = bcSet;
		changeTopOptBC(); 
        ReleaseWorkSpace();// if ALLOW_INDEPENDENT_RESTRAINTS
		for(int j = 0; j < ts_nConstraints[bcSet]; j++){

			ts_topoptQuantity = (TOPOPT_QUANTITY) ts_constraintType[bcSet][j];
			initializeSolution(); //necessary for multi-load
			if(!solvePrimaryAndAdjointProblems(nFEA)) {//solve different types of FEAs
				cout << "TopOpt_MLMC: FEA error for BCSet: " << bcSet << " constraint: " << j << endl;
				return false;
			}

            for (int part = 0;  part < getNumberOfParts(); part++)
                maxStress[bcSet] = MAX(maxStress[bcSet],getMaxStressInPart(part));//used below

			if(ts_StepNumber_ALM == 0) computeTopSensInitial(); //initialize different TS
			ts_gFunction[bcSet][j] = compute_gfunc_perConstraint_perBCSet(bcSet,j); //update g functions
			if (ts_verbose)
				cout << "TopOpt_MLMC:  BCSet: " << bcSet << " constraint: " << j << ", gfunc = " << ts_gFunction[bcSet][j] <<endl;
			computeTopSensAtElems(); //solve for various constraint TS
			for(int elem = 0; elem < h_nElems; elem++)
				ts_allConstraintsML_TS[bcSet][elem+j*h_nElems] = ts_topSensElem[elem];
		}
        ts_J[bcSet] = computeCompliance();//ts_compliance
        if(ts_StepNumber_ALM == 0)
            ts_J0[bcSet] = ts_J[bcSet];
	}
    bool unscaled;
    unscaled = true;
    if (unscaled){
        // an unscaled TS field is necessary for the critical load
        int criticalLoad;
        double criticalStress;
        criticalLoad = 0;
        criticalStress = 0;
        for(int bcSet = 0; bcSet < ts_nBCSet; bcSet++){
            if (maxStress[bcSet] > criticalStress){
                criticalStress = maxStress[bcSet];
                criticalLoad = bcSet;
            }
        }
        ts_currentBCSet = criticalLoad;
        changeTopOptBC();
        ts_topoptQuantity = (TOPOPT_QUANTITY) COMPLIANCE;
        initializeSolution(); //necessary for multi-load
        if(!solvePrimaryAndAdjointProblems(nFEA)) {//solve different types of FEAs
            cout << "TopOpt_MLMC: FEA error for BCSet: " << criticalLoad << endl;
            return false;
        }
        computeTopSensAtElems();
    }
    // If all gfunc values are close to -1, then none of the constraints are active
    // Since the multiplier is computed based on gfunc, all multipliers will be equal
    // i.e., all loads will be given equal weightage. This will lead to an incorrect result.
    // We need to give more weightage for the more stringent load
    // The code below updates the gfunc under these conditions

    bool areAllLoadSetsInactive;
    double artificialMaxStress;
    areAllLoadSetsInactive = true;
    artificialMaxStress = 0;
    for(int bcSet = 0; bcSet < ts_nBCSet; bcSet++){
        artificialMaxStress = MAX(artificialMaxStress,1.1*maxStress[bcSet]);// 10% more than the highest stress
        for(int j = 0; j < ts_nConstraints[bcSet]; j++){
            if (ts_gFunction[bcSet][j]> -0.5)// at least one constraint is becoming critical
                areAllLoadSetsInactive = false;
        }

    }
    if (ts_verbose)
        cout << "TopOpt_MLMC: areAllLoadSetsInactive " << areAllLoadSetsInactive <<endl;

    if (areAllLoadSetsInactive){// if all loads are inactive, use the artifical stress to recompute gfunc
        for(int bcSet = 0; bcSet < ts_nBCSet; bcSet++)
            for(int j = 0; j < ts_nConstraints[bcSet]; j++)
                ts_gFunction[bcSet][j] = maxStress[bcSet]/artificialMaxStress - 1;
    }

	if ((ts_StepNumber_ALM == 0) && (ts_initializeMultiplierFun)) {
		compute_InitialMultiplier_loadMagnitude();
		for(int bcSet = 0; bcSet < ts_nBCSet; bcSet++)
			for(int j = 0; j < ts_nConstraints[bcSet]; j++)
				ts_Saved_gFunction[bcSet][j] = ts_gFunction[bcSet][j]; 
		ts_initializeMultiplierFun = false;
	}

	augmentedTSFields();
	connectivityTSSmoothener();
	return true;
}
/**********************************************************************/
bool TopOptMultiLoadsMultiConstraints::constraintVerify(int *constrainingBCSet) 
{

	*constrainingBCSet  = 0;
	double gsMax = -HUGE_VAL;
	for(int i=0; i < ts_nBCSet; i++){
		for(int j = 0; j < ts_nConstraints[i]; j++){
			if (ts_gFunction[i][j] > gsMax) {
				gsMax = ts_gFunction[i][j];
				*constrainingBCSet = i;
			}
			if ((ts_gFunction[i][j] > 0) && ts_isHardConstraint[i][j]) {// failure of a hard constraint
				sprintf(ts_lastTopOptErrorMessage, "TopOpt_MLMC: Constraint violated; bcSet: %d, constraint: %d ", i, j);
				return false;
			}
		}
	}
	return true;
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::computeJ(double J[MAX_BC_SET])
{
	for(int i=0; i < ts_nBCSet; i++)
		J[i] = ts_J[i]/ts_J0[i]; //ratio
	
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::get_allCompliances_ML(double J[MAX_BC_SET])
{
	for(int i=0; i<ts_nBCSet; i++)
		J[i] = ts_J[i]; 
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::get_allMaxStressesAtCounterForAllLoadsAllPart_ML(int isoCounter,double Stress[MAX_BC_SET][MAX_PARTS])
{
	if (isoCounter == -1) return;
	if (isoCounter > ts_currentIsoCounter) return;
	for(int bc = 0; bc< ts_nBCSet; bc++)
		for (int part = 0; part < getNumberOfParts(); part++)
			Stress[bc][part] = ts_MLMaxStress[isoCounter][bc][part];
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::get_allMaxDisplacementsAtCounter_ML(int isoCounter,double Disp[MAX_BC_SET])
{
	if (isoCounter == -1) return;
	if (isoCounter > ts_currentIsoCounter) return;
	for(int i=0; i< ts_nBCSet; i++)
		Disp[i] = ts_Disp[isoCounter][i];
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::retrieveFromSavedData_ML(void)
{
	//setSolutionAtAllNodes(ts_backupData.sol);
	setPseudoDensity(ts_backupData.pseudoDensity);
	setNodeExists(ts_backupData.nodeExists);

	memcpy(ts_topSensElem,ts_backupData.topSensElem,h_nElems*sizeof(double));
	memcpy(ts_topSensNode,ts_backupData.topSensNode,h_nNodes*sizeof(double));

	if (isSpMVMethodNodeBased()) {
		setNeighborNodes(ts_backupData.neighborNodes);
		setNodeSignatures(ts_backupData.nodeSignature);
	}

	for (int i = 0; i < MAX_BC_SET; i++){
		for (int part = 0; part < getNumberOfParts(); part++)
			ts_MLMaxStress[ts_currentIsoCounter][i][part] = ts_backupData.maxStress[i][part];
		ts_Disp[ts_currentIsoCounter][i] = ts_backupData.maxDisp[i];
	}

	if (ts_verbose) cout << "Topopt_MLMC: Retrieved saved data for isocounter " << ts_currentIsoCounter << endl;
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::pushToSavedData_ML(void)
{
	if (ts_verbose) cout << "Topopt_MLMC: Saving back up for recovery for isocounter " << ts_currentIsoCounter << endl;

	//getSolutionVector(ts_backupData.sol);
	getPseudoDensity(ts_backupData.pseudoDensity);
	getNodeExists(ts_backupData.nodeExists);

	memcpy(ts_backupData.topSensElem,ts_topSensElem,h_nElems*sizeof(double));
	memcpy(ts_backupData.topSensNode,ts_topSensNode,h_nNodes*sizeof(double));

	if (isSpMVMethodNodeBased()) {
		getNeighborNodes(ts_backupData.neighborNodes);
		getNodeSignatures(ts_backupData.nodeSignature);
	}

	for (int i = 0; i < MAX_BC_SET; i++){
		for (int part = 0; part < getNumberOfParts(); part++)
			ts_backupData.maxStress[i][part] = ts_MLMaxStress[ts_currentIsoCounter][i][part];
		ts_backupData.maxDisp[i] = ts_Disp[ts_currentIsoCounter][i];
	}
}
/**********************************************************************/
bool  TopOptMultiLoadsMultiConstraints::saveTopOptField_ML(double volFrac,double isoValue,bool forceSave)
{
    double volfracPrev;
    // isosurfaces are saved under the following 3 conditions
    // 1. forced by user
    // 2. more than x% change in volume since last save
    // 3. the change in compliance is more than 25%
    static int saveCounter = 0;
    double J[MAX_BC_SET];
    double stress[MAX_BC_SET][MAX_PARTS];


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
    get_allCompliances_ML(J);
    get_allMaxStressesAtCounterForAllLoadsAllPart_ML(ts_currentIsoCounter,stress);
    for (int loadSet =0; loadSet < ts_nBCSet; loadSet++){
        ts_complianceSave[loadSet][ts_currentIsoCounter+1] = J[loadSet];
        for (int part  = 0; part < getNumberOfParts(); part++)
            ts_maxStressSave[loadSet][ts_currentIsoCounter+1][part] = stress[loadSet][part];
    }

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
double TopOptMultiLoadsMultiConstraints::TraceParetoCurve_MLMC(double desiredVolFrac,double volDecrementMax, int *nFEA, 
						double *isoValueConverged, bool (*callBackFunction)(bool saveIsoSurface),
						double *volDecrementStart,int constraintType[MAX_BC_SET][MAX_CONSTRAINTS], int *numConstraints, double currentConstraintsValue[MAX_BC_SET][MAX_CONSTRAINTS][6], 
						int topOptObjective, int numBCSet,
                        int BCType[MAX_BC_SET][1000][3], double BCValue[MAX_BC_SET][1000][6], int numTriGroups,
						bool activeBCSets[100],double allocatedTimeInSecs)
						// drawDirection = 0 (no constraint)
						// drawDirection = 1 (along x)
						// drawDirection = 2 (along y)
						// drawDirection = 3 (along z)
{
	double J[MAX_BC_SET], JTemp[MAX_BC_SET], S0[MAX_BC_SET][MAX_PARTS], C0[MAX_BC_SET], S[MAX_BC_SET][MAX_PARTS], C[MAX_BC_SET];
	double isoTemp,currentVolfrac,convergedVolFraction;
	int flag,nElemsToRemove,nAttempts,maxAttempts;
	double tempVolDecrement;
	int constrainingBCSet;
	bool topologySaved;

	// Initialize problem parameters
	ts_objective = topOptObjective;
	ts_nBCSet = numBCSet;
    ts_numTriGroups = numTriGroups;

    //ts_saveEveryNIterations = NEARESTINT(1.0/volDecrementMax/MAX_ISO_SAVES)+1;
	if (ts_verbose) cout << "TopOpt_MLMC: Saving every " << ts_saveEveryNIterations  <<" iterations "  << endl;

	for(int i = 0; i < ts_nBCSet; i++){
		ts_nConstraints[i] = numConstraints[i];
		ts_activeBCSets[i] = activeBCSets[i];
		for(int j = 0; j < numConstraints[i]; j++){
			ts_isUpperBound[i][j] = (currentConstraintsValue[i][j][0]> 0.5)? true:false; 
			ts_isHardConstraint[i][j] = (currentConstraintsValue[i][j][1]> 0.5)? true:false;  
			ts_UpperOrLowerLimit[i][j] = currentConstraintsValue[i][j][2]; 
			ts_constraintType[i][j] = constraintType[i][j];
		}
	}

	for(int i = 0; i < MAX_BC_SET; i++){
		for(int j = 0; j < 1000; j++){
			for(int k = 0; k < 3; k++)
				ts_BCType[i][j][k] = BCType[i][j][k];
			for(int p = 0; p < 6; p++)
				ts_BCValue[i][j][p] = BCValue[i][j][p];
		}
	}

	int dummy = 1;
	for(int i = 0; i < ts_nBCSet; i++){
		for(int j = 0; j < numConstraints[i]; j++){
			dummy *= ts_isHardConstraint[i][j]; 
		}
	}
	if(dummy)
		ts_isAllConstraintsHard = true;
	else
		ts_isAllConstraintsHard = false;

	if (desiredVolFrac < ts_minAllowableVolumeFraction) {
		cout << "TopOpt_MLMC: Desired volume fraction " << desiredVolFrac << " is less than allowable " << ts_minAllowableVolumeFraction << endl;
		desiredVolFrac = ts_minAllowableVolumeFraction;
	}
	if (ts_topSensElem == NULL) {
		cout << "TopOpt_MLMC: Initialize Topopt!" << endl;
		return 1.0;
	}
	currentVolfrac  = getCurrentVolumeFractionOfOptimizedParts(); // maybe restart from a previously converged result
	if (currentVolfrac > 0.99){	
		ts_currentIsoCounter = -1;
		initializePseudoDensity();
	}

	maxAttempts = 5;
	ts_timer.stop();
	ts_timer.start();

	InitializeMLMCParameters();

	if (ts_verbose) cout << "TopOpt_MLMC: Tracing Pareto Curve " << endl;
	
	convergedVolFraction = currentVolfrac;
	if(ts_topoptQuantity != EIGENVALUE){
		if (getForceNorm() == 0) {
			cout << "TopOpt_MLMC: Zero force vector detected" << endl;
			return FAILURE;
		}
	}

	setTStoAnalytical(true);// default
	if ((ts_topoptQuantity != COMPLIANCE) && (ts_topoptQuantity != VONMISES))
		setTStoAnalytical(false);

	if (!solve_FEA_TS_ML(nFEA)){ 
		cout << "TopOptMLMC:  solve_FEA_TS error " << endl;
		return FAILURE;
	}
	computeJ(J); 
	ts_displacement0 = getDeformationMax();
	ts_vonMisesStress0 = getMaxVonMisesStress();

	tempVolDecrement = *volDecrementStart;
	nElemsToRemove =0;
	computeIsovalue(currentVolfrac,&isoTemp,&nElemsToRemove);

	if (ts_useEigenConstraint)
		saveFEAandSolveEigen(nFEA);//needed for saving eigen value below

    topologySaved = saveTopOptField_ML(currentVolfrac,isoTemp,true);
	
	checkDispAndStressConstraintsForAllBCSets(&constrainingBCSet);
	pushToSavedData_ML();


    writeMessageToGUIWindow("------------TopOpt Constraints ----------");
    writeMessageToGUIWindow("TopOpt_MLMC: Symmetry index: %d", ts_symmetryIndex);
    writeMessageToGUIWindow("TopOpt_MLMC: Through-cut: %d", ts_extrudeDirection);
    writeMessageToGUIWindow("TopOpt_MLMC: Draw-direction: %d", ts_drawDirection);
    writeMessageToGUIWindow("TopOpt_MLMC: Cyclic-symmetry: %d", ts_axiSymmetryIndex);
    writeMessageToGUIWindow("TopOpt_MLMC: XGridPattern: %d", ts_gridPattern[0]);
    writeMessageToGUIWindow("TopOpt_MLMC: YGridPattern: %d", ts_gridPattern[1]);
    writeMessageToGUIWindow("TopOpt_MLMC: ZGridPattern: %d", ts_gridPattern[2]);
    writeMessageToGUIWindow("TopOpt_MLMC: Max disp. allowed: %e", ts_maxDisplacementAllowed);
    writeMessageToGUIWindow("TopOpt_MLMC: Stress safety factor: %.2f", ts_stressSafetyFactor);
    writeMessageToGUIWindow("TopOpt_MLMC: Min Eigen-value: %.2f", ts_minEigenValueHz);
    writeMessageToGUIWindow("TopOpt_MLMC: Smoothening iterations: %d", ts_nSmoothingIterations);
    writeMessageToGUIWindow("-----------------------------------------");

	if (topologySaved)  callBackFunction(true);
	//initial values
	get_allCompliances_ML(C0); 
	get_allMaxStressesAtCounterForAllLoadsAllPart_ML(0,S0);
	
	nAttempts = 0; // at current volume fraction
	
    double maxAllowableVolumeDecrement;
    double JIncrement = ts_relativeJIncrementForStepping;

    while (currentVolfrac > (1+5*ts_volErrorFraction)*desiredVolFrac) {// small error at the end acceptable
        convergedVolFraction = currentVolfrac;

        maxAllowableVolumeDecrement = computeAllowableVolumeDecrement(JIncrement);
        cout << "maxAllowableVolumeDecrement "<< maxAllowableVolumeDecrement<< endl;
        if (maxAllowableVolumeDecrement < 0)// pathological conditions
            maxAllowableVolumeDecrement = 0.025;

        tempVolDecrement = MIN(tempVolDecrement,currentVolfrac-desiredVolFrac);
        for(int i=0; i< ts_nBCSet; i++) JTemp[i] = J[i];
        if (ts_timer.getElapsedTimeInSec() > allocatedTimeInSecs) {
			cout << "TopOpt_MLMC: Exceeded max allowed time of " << allocatedTimeInSecs  <<"  seconds " << endl;
			break;
		}
        writeMessageToParetoLog("TopOpt_MLMC: %.3f -> %.3f;",currentVolfrac,currentVolfrac-tempVolDecrement);
		if (ts_verbose) cout << endl;// extra line for readability
		flag = IterateToVolume_MLMC(currentVolfrac-tempVolDecrement, JTemp, nFEA, &isoTemp, callBackFunction);

		if (flag == USER_REQUEST_STOP) {
			sprintf(ts_lastTopOptErrorMessage, "TopOpt_MLMC: User terminated TopOpt ");
			break;
		}
		if (flag == ITER_TO_VOL_SUCCESS){
			flag = constraintVerify(&constrainingBCSet);
			if (!flag) 
				cout << ts_lastTopOptErrorMessage << endl;
			flag = checkDispAndStressConstraintsForAllBCSets(&constrainingBCSet);
			if (!flag) 
				cout << ts_lastTopOptErrorMessage << endl;
		}
		if (flag == ITER_TO_VOL_SUCCESS)  {	
			if (ts_verbose) cout << "TopOpt_MLMC: Iteration converged " << endl;
			currentVolfrac = getCurrentVolumeFractionOfOptimizedParts();
			if (currentVolfrac < 0.9*convergedVolFraction) 
				nAttempts = 0; // reset at current volume fraction
			else if (currentVolfrac > convergedVolFraction ) 
				break;//something went wrong
			convergedVolFraction = currentVolfrac;
			if (ts_verbose) cout << "TopOpt_MLMC: convergedVolFraction = " << convergedVolFraction << endl;
			
            topologySaved = saveTopOptField_ML(convergedVolFraction,isoTemp);// this is for plotting
			checkDispAndStressConstraintsForAllBCSets(&constrainingBCSet);// check once more to save correct displacements, etc
			pushToSavedData_ML();// this is for recovery from failure
			for(int i=0; i< ts_nBCSet; i++) 
				J[i] = JTemp[i];
			get_allCompliances_ML(C); 
			get_allMaxStressesAtCounterForAllLoadsAllPart_ML(ts_currentIsoCounter,S); 
			double maxIncreaseJ,maxIncreaseS;
			maxIncreaseJ = 1;
			maxIncreaseS = 1;
			//to have meaningful ratio, both C and S have to be set as constraints
			for(int i=0; i< ts_nBCSet; i++) {
				if (ts_verbose) cout << "TopOpt_MLMC: BCSet = " << i << " C/C0 = " << C[i] / C0[i] << "; S/S0 = " <<  S[i][0] / S0[i][0] << endl;
				maxIncreaseJ = MAX(maxIncreaseJ,C[i]/C0[i]);
				for (int part = 0; part < getNumberOfParts(); part++){
					maxIncreaseS = MAX(maxIncreaseS,S[i][part]/S0[i][part]);
				}
			}
			*isoValueConverged = isoTemp;
			cout << " J/J0 = " << maxIncreaseJ << ", S/S0 = " << maxIncreaseS << endl;

			if ( (topologySaved) && (!callBackFunction(true))) break;
			// the volume step size must be reduced as we move up the Pareto curve
			// but if the increase is too large, the volume decreases and the optimization quits

            tempVolDecrement = MIN(tempVolDecrement,*volDecrementStart/MIN(25,sqrt(MAX(maxIncreaseJ,maxIncreaseS))));
			onSuccess();
			if (ts_verbose) cout << "TopOpt_MLMC: tempVolDecrement = " << tempVolDecrement << endl << endl;
		}
		else {
			nAttempts++;
			cout << "* " << endl;
			retrieveFromSavedData_ML();
			//postprocess_ML();
			currentVolfrac = convergedVolFraction;
            JIncrement *=0.5;
            tempVolDecrement *= 0.5;
			onFailure();
		}
		if ((nAttempts >= maxAttempts) || ( (currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac) && (tempVolDecrement < ts_volErrorFraction)) ){
			if (ts_verbose) cout << "TopOpt_MLMC: Exiting ... tempVolDecrement = " << tempVolDecrement << " ts_volErrorFraction = " << ts_volErrorFraction << endl;
			currentVolfrac = convergedVolFraction;
			break;
		}
	}

	ts_currentBCSet = constrainingBCSet;// to find the max stress
	changeTopOptBC(); 
	solveElasticityStatic();
	postprocess();
    saveTopOptField_ML(convergedVolFraction,isoTemp, true);
	checkDispAndStressConstraintsForAllBCSets(&constrainingBCSet);// this also saves results, do not remove
	*volDecrementStart = tempVolDecrement;
	//solvePrimaryAndAdjointProblems(nFEA);// solve one last time!
	callBackFunction(true);// forces save
	return convergedVolFraction;
}
/**********************************************************************/
int  TopOptMultiLoadsMultiConstraints::IterateToVolume_MLMC(double desiredVolumeFraction, double *JConverged,
															int *nFEA, double *isoValueConverged, bool(*callBackFunction)(bool saveIsoSurface))
{
	double isoValue;
	double JTemp[MAX_BC_SET],JMin[MAX_BC_SET];
	int nIter,nElemsToRemove;
	bool converged;

	if (ts_verbose) cout << "Topopt_MLMC: Iterating to volume " << endl;
	nIter = 0;

	nElemsToRemove = (int) ceil((1.0 -desiredVolumeFraction)*h_nElems +1);
	while (nIter < ts_max_topoptIterations) {// to avoid cycles
		if (ts_verbose) cout << "TopOpt_MLMC: iter  = " << nIter << endl;
		solve_FEA_TS_ML(nFEA);//includes FEA, TS field computation and smoothening

		if ((computeIsovalue(desiredVolumeFraction,&isoValue,&nElemsToRemove) == FAILURE)|| (_isnan(isoValue))){	
			strcpy(ts_lastTopOptErrorMessage,"TopOpt_MLMC: Isovalue extraction error... try increasing mesh quality and/or check loading");
			cout << endl << ts_lastTopOptErrorMessage << endl;
			return ITER_TO_VOL_ISOVALUE_FAILURE;
		}

        if (ts_verbose) writeMessageToParetoLog("Topopt_MLMC: isovalue of %e \n",isoValue);
		setElementToVoid(ts_restartTopology,ts_topSensElem,isoValue,nElemsToRemove);
		if (!solve_FEA_TS_ML(nFEA)){ 
			strcpy(ts_lastTopOptErrorMessage,"TopOpt_MLMC: Could not solve FEA ... possible topology separation; try increasing mesh quality");
			cout << endl << ts_lastTopOptErrorMessage << endl;
			return ITER_TO_VOL_FEA_FAILURE;
		}
		if (!callBackFunction(false)) return USER_REQUEST_STOP;
		computeJ(JTemp); 
		for(int i=0; i < ts_nBCSet; i++)
			JMin[i] =(nIter==0)? HUGE_VAL:min(JTemp[i],JMin[i]);

		converged = true;
		for(int i=0; i< ts_nBCSet; i++){
            if (ts_verbose)
                cout << "TopOpt_MLMC: BCset: " << i << "; J/JConverged=  " << JTemp[i]/JConverged[i] << "; J/JMin=  " << JTemp[i]/JMin[i] << endl;
			if ( (fabs(JTemp[i]/JConverged[i]) > ts_relativeJConvergence) || (fabs(JTemp[i]/JConverged[i]) < 1./ts_relativeJConvergence) ){
				strcpy(ts_lastTopOptErrorMessage,"Large increase in compliance ... possible topology separation; try increasing mesh quality");
				cout << endl << ts_lastTopOptErrorMessage << endl;
				return DIVERGENCE_FAILURE;
			}
			if (JTemp[i] > (1+ts_relativeJTolerance)*JMin[i])
				converged = false;
		}
		if ((nIter >= ts_min_topoptIterations) && converged)  {
			*isoValueConverged = isoValue;
			if (ts_verbose) cout << "Topopt_MLMC: Converged to isovalue of  " << isoValue << endl;
			for(int i=0; i< ts_nBCSet; i++)
				JConverged[i] = JTemp[i];
			return ITER_TO_VOL_SUCCESS;;//success
		}
		nIter++;
	}
	return ITERATION_ERROR;
}
/**********************************************************************/
void TopOptMultiLoadsMultiConstraints::getSafetyFactorsAtVolumeFraction(double volFrac,double *displacementSF,int *criticalLoadForDisplaement, 
														double *stressSF,int *criticalLoadForStress, int *criticalPartForStress)
{
	// find the worst safety factor
	double tempDouble,yieldStress,actualVolumeFraction,isoCutoff,timeInSecs;
	double allStresses[MAX_BC_SET][MAX_PARTS], allDisplacements[MAX_BC_SET];

	*displacementSF = 100;
	*stressSF = 100;
	*criticalLoadForStress = 0;
	*criticalPartForStress = 0;
	*criticalLoadForDisplaement  = 0;

	int savedCounter = getClosestCounterAtVolFraction(volFrac,&actualVolumeFraction,&isoCutoff, &timeInSecs);
	if (savedCounter < 0) 	return;
	
	if ((ts_nBCSet > 1) && (ts_useAllLoads)){// multiload 
		get_allMaxDisplacementsAtCounter_ML(savedCounter,allDisplacements);
		for (int part = 0; part < getNumberOfParts(); part++){
			get_allMaxStressesAtCounterForAllLoadsAllPart_ML(savedCounter,allStresses);
		}
	}
	else {// single load
		allDisplacements[0] =  getMaxDispAtCounter(savedCounter);
		for (int part = 0; part < getNumberOfParts(); part++)
			allStresses[0][part] = getMaxStressInPartAtCounter(savedCounter,part);
	}

	*displacementSF = HUGE_VAL;
	for (int bcSet = 0; bcSet < ts_nBCSet; bcSet++) {
		tempDouble = ts_maxDisplacementAllowed/allDisplacements[bcSet];
		if (tempDouble < *displacementSF) {
			*displacementSF = tempDouble;
			*criticalLoadForDisplaement = bcSet;
		}
	}

	*stressSF = HUGE_VAL;
	for (int bcSet = 0; bcSet < ts_nBCSet; bcSet++) {
		for (int part = 0; part < getNumberOfParts(); part++){
			int matId = getMaterialIDofPart(part);
			getYieldStressOfMaterial(matId,&yieldStress);
			tempDouble = yieldStress/allStresses[bcSet][part];
			if (tempDouble < *stressSF) {
				*stressSF = tempDouble;
				*criticalLoadForStress = bcSet;
				*criticalPartForStress = part;
			}
		}
	}

}
/**********************************************************************/
TopOptMultiLoadsMultiConstraints::~TopOptMultiLoadsMultiConstraints()
{
	safeFree(ts_allConstraintsML_TS);	ts_allConstraintsML_TS = NULL;
} 
