#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string> 
#include <fstream> 
#include <iostream> 
#include <omp.h>
#include "cmatrix.h"
#include "Timer.h"
#include "TopOptSingleLoadMultiConstraints.h"

using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)

/**********************************************************************/
TopOptSingleLoadMultiConstraints::TopOptSingleLoadMultiConstraints() 
{
	ts_StepNumber_ALM = 0;
	ts_complianceLastStep = 0.0;
	ts_verbose_SLMC = false;
	ts_initializeMultiplierFun = true;
	

	ts_allConstraints_TS=NULL;//All Topological sensitivity fields
	ts_augmentedTS=NULL;//augmented topological sensitivity fields
	ts_objective_TS=NULL;//objective TS field
}
/************************************************************/
void TopOptSingleLoadMultiConstraints::InitializeSLMCParameters(void)
{

	ts_StepNumber_ALM = 0;
	ts_complianceLastStep = 0.0;

	setTStoAnalytical(true);
	for(int i=0;i<MAX_CONSTRAINTS;i++){
		ts_Sigma[i] = 100.0;//Lagrangian multiplier
		ts_Penalty[i] = 1.0; //penalty parameters
		ts_gFunction[i] = 0.0;// g function
		ts_Saved_gFunction[i] = 0.0;// saved g values
	}

	safeFree(ts_allConstraints_TS);//Topological sensitivity fields
	ts_allConstraints_TS = (double *)malloc(MAX_CONSTRAINTS*h_nElems*sizeof(double)); 
	memset(ts_allConstraints_TS,0,MAX_CONSTRAINTS*h_nElems*sizeof(double));

	safeFree(ts_augmentedTS);//augmented topological sensitivity fields
	ts_augmentedTS = (double *)malloc(h_nElems*sizeof(double));
	memset(ts_augmentedTS,0,h_nElems*sizeof(double));

	safeFree(ts_objective_TS);
	ts_objective_TS=(double *)malloc(h_nElems*sizeof(double)); //objective TS
	memset(ts_objective_TS,0,h_nElems*sizeof(double));
}
/**********************************************************************/
double TopOptSingleLoadMultiConstraints::compute_gfunc_perConstraint(int constraint)
{
	//single scaled g function
	double gFunc,objective, ratio, initialValue;

	ratio = ts_UpperOrLowerLimit[constraint];
	objective = compute_Quantity_perConstraint(constraint); 
	initialValue = compute_initialQuantity_perConstraint(constraint);

	if(!ts_isUpperBound[constraint]) //lower bound problem
		gFunc = 1-objective/(ratio*initialValue);
	else //upper bound problem
		gFunc = objective/(ratio*initialValue)-1;

	return gFunc;
}
/************************************************************/
double TopOptSingleLoadMultiConstraints::compute_initialQuantity_perConstraint(int constraint)
{
	double initialValue;
	int type = ts_constraintType[constraint];

	if(type == (int) COMPLIANCE){
		initialValue = ts_J0;
	}
	else if(type == (int) VONMISES){
		initialValue = ts_Stress0;
	}
	else if(type == (int) BUCKLING){
		initialValue = ts_BucklingLoad0;
	}
	else if(type == (int) EIGENVALUE){
		initialValue = ts_EigenVaule0; //lowest eigen-value
	}
	else if(type == (int) DISPLACEMENT){
		//ts_Disp0 = ;
	}
	else if(type == (int) OUTPUT_COMPLIANCE){ 
		initialValue = ts_OJ0;
	}
	else if(type == (int) GEOMETRIC_ADVANTAGE){
		initialValue = ts_MJ0;
	}
	else {
		printf("Invalid constraint\n");
	}
	return initialValue;
}
/**********************************************************************/
double TopOptSingleLoadMultiConstraints::compute_Quantity_perConstraint(int constraint)
{
	double objective;
	int type = ts_constraintType[constraint];

	if(type == (int) COMPLIANCE){
		objective = computeCompliance();
		if(ts_StepNumber_ALM == 0)
			ts_J0 = objective;
	}
	else if(type == (int) VONMISES){
		objective = getMaxVonMisesStress(); 
		if(ts_StepNumber_ALM == 0)
			ts_Stress0 = objective;
	}
	else if(type == (int) BUCKLING){
		objective = getBucklingLoad(); //buckling load
		//objective = computeCompliance();
		if(ts_StepNumber_ALM == 0)
			ts_BucklingLoad0 = objective;
	}
	else if(type == (int) EIGENVALUE){
		objective = h_eigenValues[0]; //lowest eigen-value
		if(ts_StepNumber_ALM == 0)
			ts_EigenVaule0 = objective;
	}
	else if(type == (int) DISPLACEMENT){
		//write
	}
	else if(type == (int) OUTPUT_COMPLIANCE){ 
		objective = computeOutputCompliance(); 
		if(ts_StepNumber_ALM == 0)
			ts_OJ0 = objective;
	}
	else if(type == (int) GEOMETRIC_ADVANTAGE){
		objective = computeMutualCompliance(); 
		if(ts_StepNumber_ALM == 0)
			ts_MJ0 = objective;
	}
	else 
		printf("Invalid Constraint\n");
	return objective;
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::compute_InitialMultiplier(void) //Deng
{
	// set initial lagrangian multiplier values so that tighter constraints put significantly higher weights
	int *sortIndex;
	sortIndex = (int *)malloc(ts_nConstraints*sizeof(int));
	memset(sortIndex,0,ts_nConstraints*sizeof(int));
	double *temp;
	temp = (double *)malloc(ts_nConstraints*sizeof(double));
	double factorRatio[MAX_CONSTRAINTS] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
	double amplifier = 1000.0; //amplifify difference in initial sigma

	//make tighter constraint have more weight on its TS field
	for (int i = 0; i < ts_nConstraints; i++)
		temp[i] = ts_gFunction[i];

	bubbleSortIncreasing(temp,sortIndex,ts_nConstraints);

	for (int i = 0; i < ts_nConstraints-1; i++){	
		if((temp[i]/temp[i+1] < 1.01) && (temp[i]/temp[i+1] > 0.99)) {//if constraints are equal, there would be no AMPLIFYING effect
			factorRatio[i] = 1.0;
		}
		else{
			factorRatio[i] = abs(temp[i+1]/temp[i]*amplifier); 
		}
	}

	for (int i = 1; i < ts_nConstraints; i++){
		ts_Sigma[sortIndex[i]] = factorRatio[i-1]*(ts_Sigma[sortIndex[i-1]]+ts_Penalty[i-1]*temp[i-1])-ts_Penalty[i]*temp[i];
	}

	free(sortIndex); sortIndex = NULL;
	free(temp); temp = NULL;
}
/************************************************************/
void TopOptSingleLoadMultiConstraints::augmentedTSFields(void) 
{
	int scaleTSField = 1;
	double softConsCoeff = 0.1;

	for (int elem = 0; elem < h_nElems; elem++)
		ts_topSensElem[elem] = ts_objective_TS[elem];

	for (int num = 0; num < ts_nConstraints; num++){
		if ((ts_Sigma[num]+ts_gFunction[num]*ts_Penalty[num]>0) && ts_isHardConstraint[num]){// hard-constraint approaching failure
			for (int elem = 0; elem < h_nElems; elem++)
				ts_topSensElem[elem] += (ts_Sigma[num]+ts_gFunction[num]*ts_Penalty[num])*ts_allConstraints_TS[elem+num*h_nElems];	
		}
		else if	((ts_Sigma[num]+ts_gFunction[num]*ts_Penalty[num]>0) && (!ts_isHardConstraint[num])){// soft-constraint inside in-feasible region
			for (int elem = 0; elem < h_nElems; elem++)
				ts_topSensElem[elem] += (ts_Sigma[num]+ts_gFunction[num]*ts_Penalty[num])*ts_allConstraints_TS[elem+num*h_nElems]*softConsCoeff;	
		}
	}
	ts_TSMin = HUGE_VAL;
	ts_TSMax = -HUGE_VAL;
	for (int elem = 0; elem < h_nElems; elem++){
		ts_TSMin = MIN(ts_TSMin,ts_topSensElem[elem]);
		ts_TSMax = MAX(ts_TSMax,ts_topSensElem[elem]);
	}

	for (int elem = 0; elem < h_nElems;elem++) {
		ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin);
		if (h_keepElem[elem])
			ts_topSensElem[elem] = 1.0;
		if (getElemPseudoDensity(elem)< 0.5) 
			ts_topSensElem[elem] = 0;
	}
	ts_TSMax = 1.0; ts_TSMin = 0;
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::onSuccess(void) 
{
	//update element density & constraints & multipliers
	for (int i = 0; i < ts_nConstraints; i++) {
		ts_Saved_gFunction[i] = ts_gFunction[i];
	}
	ts_complianceLastStep = computeCompliance();//in pareo convergent criteria 
	updateSigma();
	ts_StepNumber_ALM++; //update penalty
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::onFailure(void)
{
	//update multipliers & penalty
	updatePenalty();
	updateSigma();
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::updatePenalty(void) 
{
	//update penalty parameter
	double sai = 0.25;
	int r = 10;

	for (int i=0; i < ts_nConstraints; i++){
		if (MIN(ts_gFunction[i],0) <= sai*MIN(ts_Saved_gFunction[i],0)) 
			;//update nothing
		else
			ts_Penalty[i] = MAX(r*ts_Penalty[i],ts_StepNumber_ALM^2);
	}
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::updateSigma(void) 
{
	//update Lagrangian multiplier
	for (int i = 0; i < ts_nConstraints; i++) {
		ts_Sigma[i] = MAX(0.0, ts_Sigma[i]+ts_Penalty[i]*ts_gFunction[i]);
	}	
}
/**********************************************************************/
bool TopOptSingleLoadMultiConstraints::solve_FEA_TS(int* nFEA)
{
	//for single objective
	setTopoptQuantity(ts_objective);
	if(!solvePrimaryAndAdjointProblems(nFEA)) {//solve FEA for objective
		printf("FEA error for objective \n");
		return false;
	}
	if(ts_StepNumber_ALM == 0){
		ts_currentIsoCounter = -1;
		computeTopSensInitial(); //initialize objective TS
	}

	computeTopSensAtElems(); //solve for objective TS
	memcpy(ts_objective_TS,ts_topSensElem,h_nElems*sizeof(double));
	//for all constraints
	if (ts_topoptQuantity == VONMISES)  ts_complianceWt = 0.00001; 
	for(int i = 0; i < ts_nConstraints; i++){
		setTopoptQuantity(ts_constraintType[i]);
		if(!solvePrimaryAndAdjointProblems(nFEA)) {//solve different types of FEAs
			printf("FEA error for constriant %d \n",i);
			return false;
		}
		
		if(ts_StepNumber_ALM == 0) 
			computeTopSensInitial(); //initialize different TS
		//update g functions
		ts_gFunction[i] = compute_gfunc_perConstraint(i);
		computeTopSensAtElems(); //solve for various constraint TS
		for(int elem = 0; elem < h_nElems; elem++){
			ts_allConstraints_TS[elem+i*h_nElems] = ts_topSensElem[elem];
		}
	}

	if ((ts_StepNumber_ALM == 0) && (ts_initializeMultiplierFun == true)) {
		compute_InitialMultiplier(); 
		memcpy(ts_Saved_gFunction,ts_gFunction,ts_nConstraints*sizeof(double));
		ts_initializeMultiplierFun = false;
	}

	augmentedTSFields();
	connectivityTSSmoothener();
	return true;
}
/**********************************************************************/
bool TopOptSingleLoadMultiConstraints::constraintVerify(int *constrainingBCSet) 
{
	*constrainingBCSet = 0;
	for(int i = 0; i < ts_nConstraints; i++){
		if((ts_gFunction[i]>0) && ts_isHardConstraint[i]) //Deng
		{// failure of a hard constraint
			return false;
		}
	}
	return true;
}
/**********************************************************************/
double TopOptSingleLoadMultiConstraints::TraceParetoCurve_SLMC(double desiredVolFrac,double volDecrementMax, int maxFEA,int *nFEA, 
															   double *isoValueConverged, bool (*callBackFunction)(double isoValueConverged),
															   int drawDirection,int symmetryIndex,double *volDecrementStart,double allocatedTimeInSecs,
															   int *constraintType, int numConstraints,double (*currentConstraintsValue)[6], int topOptObjective) //deng
															   // drawDirection = 0 (no constraint)
															   // drawDirection = 1 (along x)
															   // drawDirection = 2 (along y)
															   // drawDirection = 3 (along z)
{//this is function is copied from previous function "TraceParetoCurve" //
	double J,JTemp,isoTemp,currentVolfrac,convergedVolFraction,S0,C0,S,C;
	int flag,nElemsToRemove,nAttempts,maxAttempts;
	double tempVolDecrement;

	if (numConstraints <= 0) {
		printf("TopOptSingleLoadMultiConstraints: Number of constraints invalid: %d \n",numConstraints);
		return 0;
	}
	// Initialize problem parameters
	ts_objective = topOptObjective;
	ts_nConstraints = numConstraints;
	for(int i = 0; i< numConstraints; i++){
		ts_isUpperBound[i] = (currentConstraintsValue[i][0]> 0.5)? true:false; 
		ts_isHardConstraint[i] = (currentConstraintsValue[i][1]> 0.5)? true:false;  //Deng 1:hard, 0:soft constraint
		ts_UpperOrLowerLimit[i] = currentConstraintsValue[i][2]; //Deng
		ts_constraintType[i] = constraintType[i];
	}

	if (desiredVolFrac < ts_minAllowableVolumeFraction) {
		printf("TopOptSLMC: Desired volume fraction %.3f is less than allowable %.3f\n",desiredVolFrac,ts_minAllowableVolumeFraction);
		desiredVolFrac = ts_minAllowableVolumeFraction;
	}
	if (ts_topSensElem == NULL) {
		printf("TopOptSLMC: Initialize Topopt!\n");
		return 1.0;
	}
	maxAttempts = 5;

	ts_timer.stop();
	ts_timer.start();

	//initialize algorithm parameters
	InitializeSLMCParameters(); 
	ts_drawDirection = drawDirection;
	ts_symmetryIndex = symmetryIndex;
	if (ts_verbose) printf("TopOptSLMC: Tracing Pareto Curve\n");
	currentVolfrac  = getCurrentVolumeFractionOfOptimizedParts();// we might be restarting from a previously converged result
	convergedVolFraction = currentVolfrac;
	if(ts_topoptQuantity != EIGENVALUE){
		if (getForceNorm() == 0) {
			printf("TopOptSLMC: Zero force vector detected\n");
			return -1;
		}
	}

	if (!solve_FEA_TS(nFEA)){ 
		printf("TopOptSLMC: Start solve_FEA_TS error\n");
		return FAILURE;
	}

	J = computeJ();// Quantity of Interest 
	tempVolDecrement = *volDecrementStart;
	computeIsovalue(currentVolfrac,&isoTemp,&nElemsToRemove);
	saveTopOptField(currentVolfrac,isoTemp, false);
	pushToSavedData();
	S0 = h_vonmisesMax;//initial stress
	C0 = computeCompliance();
	nAttempts = 0; // at current volume fraction

	while (currentVolfrac > (1+5*ts_volErrorFraction)*desiredVolFrac) {// small error at the end acceptable
		convergedVolFraction = currentVolfrac;
		tempVolDecrement = MIN(tempVolDecrement,currentVolfrac-desiredVolFrac);
		JTemp = J;
		if (*nFEA > maxFEA) {
			printf("TopOptSLMC: #FEA exceeded max allowed of %d \n",maxFEA);
			break;
		}
		if (ts_timer.getElapsedTimeInSec() > allocatedTimeInSecs) {
			printf("TopOptSLMC: Exceeded max allowed time of %.1f seconds \n",allocatedTimeInSecs);
			break;
		}

		if (ts_verbose) printf("-------------------------------------------------------\n");
		if (ts_objective == VONMISES) {
			C = ts_compliance;
			S = h_vonmisesMax;
			printf("TopOptSLMC: %.4f -> %.4f; C/C0 = %.2f; S/S0 = %.2f",currentVolfrac,currentVolfrac-tempVolDecrement,C/C0,S/S0);
		}
		else if (ts_objective == GEOMETRIC_ADVANTAGE) {
			printf("TopOptSLMC: %.4f -> %.4f; I/I0 = %.2f; O/O0 = %.2f; GA = %.2f",currentVolfrac,currentVolfrac-tempVolDecrement,ts_compliance/ts_compliance0,ts_outputcompliance/ts_outputcompliance0,ts_geometricAdvantage);
		}
		else {// default
			C = ts_compliance;
			S = h_vonmisesMax;
			printf("TopOptSLMC: %.4f -> %.4f; C/C0 = %.2f; S/S0 = %.2f\n",currentVolfrac,currentVolfrac-tempVolDecrement,C/C0,S/S0);
		}
		if (ts_verbose)  printf("\n");
		flag = IterateToVolume_SLMC(currentVolfrac-tempVolDecrement,&JTemp, nFEA,&isoTemp); 

		if (flag == ITER_TO_VOL_SUCCESS)  {//converged	
			removeDeadMaterial();
			if (ts_verbose) printf("TopOptSLMC: Iteration converged \n");
			currentVolfrac = getCurrentVolumeFractionOfOptimizedParts();
			if (currentVolfrac < 0.9*convergedVolFraction) nAttempts = 0; // reset at current volume fraction
			convergedVolFraction = currentVolfrac;
			saveTopOptField(convergedVolFraction,isoTemp,false);// this is for plotting
			pushToSavedData();// this is for recovery from failure
			J = JTemp;
			*isoValueConverged = isoTemp;
			tempVolDecrement = min(volDecrementMax,1.1*(tempVolDecrement));
			onSuccess(); 
			callBackFunction(*isoValueConverged);
			printf("\n");
		}
		else {
			nAttempts++;
			printf("* \n");
			retrieveFromSavedData();
			postprocess();
			currentVolfrac = convergedVolFraction;
			tempVolDecrement *= 0.5; 
			onFailure();
		}
		if ((nAttempts >= maxAttempts) || ( (currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac) && (tempVolDecrement < ts_volErrorFraction)) ){
			currentVolfrac = convergedVolFraction;
			break;
		}
	}
	//printf("Done: %.4e\n",isoTemp);
	saveTopOptField(convergedVolFraction,isoTemp, true);
	*volDecrementStart = tempVolDecrement;

	if (1)
	{ 
		printf("-------------------------------------------------------\n");
		for (int i=0; i < numConstraints; i++){
			if(!ts_isUpperBound[i]){
				printf("Final Ratio: %f\n",(1-ts_Saved_gFunction[i])*ts_UpperOrLowerLimit[i]);
			}
			else{
				printf("Final Ratio: %f\n",(1+ts_Saved_gFunction[i])*ts_UpperOrLowerLimit[i]);
			}
		}
	}
	return convergedVolFraction;
}
/**********************************************************************/
void TopOptSingleLoadMultiConstraints::setTopoptQuantity(int quantity)
{
	ts_topoptQuantity = (TOPOPT_QUANTITY) quantity;
}
/**********************************************************************/
int  TopOptSingleLoadMultiConstraints::IterateToVolume_SLMC(double desiredVolumeFraction, double *JConverged,
															int *nFEA, double *isoValueConverged)
															// Fixed-point iteration to desired volume fraction
{//this is function is copied from previous function "IterateToVolume" 
	double isoValue,JTemp,JStart,JPrevious;
	int nIter,nElemsToRemove;
	double JOld = (*JConverged);
	JStart = JOld;
	int constrainingBCSet;

	if (ts_verbose) printf("TopOptSLMC: Iterating to volume \n");
	nIter = 0;

	while (nIter < ts_max_topoptIterations) {// to avoid cycles
		if (computeIsovalue(desiredVolumeFraction,&isoValue,&nElemsToRemove) == FAILURE){	
			printf(" .. could not extract iso value");
			return ITER_TO_VOL_ISOVALUE_FAILURE;
		}
		setElementToVoid(ts_restartTopology,ts_topSensElem,isoValue,nElemsToRemove);
		if (!solve_FEA_TS(nFEA)){ 
			printf(" .. solve_FEA_TS error \n"); 
			return ITER_TO_VOL_FEA_FAILURE;
		}
		if(!constraintVerify(&constrainingBCSet)){ 
			printf("TopOptSLMC:  Constraints are violated\n");
			return FAILURE;
		}
		JTemp = computeJ();	
		if (ts_verbose)  printf("TopOptSLMC: JTemp = %e\n",JTemp);
		if (ts_verbose)  printf("TopOptSLMC: iter %d; J/J0= %0.2f\n",nIter,JTemp/(*JConverged));
		if ( (fabs(JTemp/(*JConverged)) > 5) || (fabs(JTemp/(*JConverged)) < 0.1) ){
			printf(" .. compliance divergence");
			return DIVERGENCE_FAILURE;
		}
		if (ts_verbose) printf("TopOptSLMC: Err = %e, ts_relativeJTolerance = %e\n",fabs((JTemp-JOld)/JOld),ts_relativeJTolerance);
		if ((nIter >= ts_min_topoptIterations) && (fabs((JTemp-JOld)/JOld) < ts_relativeJTolerance)  ){
			*isoValueConverged = isoValue;
			*JConverged = JTemp;
			return ITER_TO_VOL_SUCCESS;;//success
		}

		JPrevious = JOld;
		JOld = JTemp;
		nIter++;
	}
	if (fabs(JTemp-JOld)/JOld >= ts_relativeJTolerance){
		return ITER_TO_VOL_COMPLIANCE_FAILURE;
	}
	else {
		printf("Iteration error\n");
		return ITERATION_ERROR;
	}
}
/**********************************************************************/
TopOptSingleLoadMultiConstraints::~TopOptSingleLoadMultiConstraints()
{
	safeFree(ts_allConstraints_TS);	ts_allConstraints_TS = NULL;
	safeFree(ts_augmentedTS);		ts_augmentedTS = NULL;
	safeFree(ts_objective_TS);		ts_objective_TS = NULL; 
} 
