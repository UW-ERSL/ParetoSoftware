#include "SIMP.h" // include the pertaining header files
#include<iostream>
#include<math.h>
#include<string>
#include<stdio.h>
#include<fstream>
#include<cmath>
#include <time.h>
#include<Timer.h>
#include<vector>
#include<algorithm>

bool dummyCallBackSIMP(float x){
    return true;
}

/*****************************************************************************/
SIMP::SIMP()
{ 
	if(SIMP_VERBOSE){
		std::cout<<"Initializing SIMP problem Constructor";
	}
	simp_targetVolumeFraction = 0.5;
    simp_minDensity = 0.001;// don't allow density below this value
    simp_densityChangeTerminate = 0.01;// change in density for convergence
    simp_lambdaPrev = 100;
	setEexponent(3); 
}
/**************************************************************************************************************************/
void SIMP::allocateMemory(void)
{
    writeMessageToParetoLog("Allocating Memory for SIMP");
	
	simp_pseudoDensity = NULL;
	simp_sensitivity = NULL;
	simp_pseudoDensity = (double *)malloc((h_nElems)*sizeof(double));
	simp_sensitivity = (double *)malloc((h_nElems)*sizeof(double));;
}
/**************************************************************************************************************************/
void SIMP::setSIMPTargetVolumeFraction(double volfrac)
{
	simp_targetVolumeFraction = volfrac;
}
/**************************************************************************************************************************/
double SIMP::getSIMPMinDensity(void)
{
    return simp_minDensity;
}
/**************************************************************************************************************************/
void SIMP::setSIMPMinDensity(double minDensity)
{
	simp_minDensity = minDensity;
}
/**************************************************************************************************************************/
double SIMP::getSIMPTimerInSeconds(void)
{
    return timer.getElapsedTimeInSec();
}
/**************************************************************************************************************************/
void SIMP::solveSIMPPareto(bool (*callbackFunction)( float percentCompleted))
{
    double targetVolFraction,step;

    timer.start();
    step = 0.025;
	targetVolFraction = simp_targetVolumeFraction;//save
    for (simp_targetVolumeFraction = 1.0; simp_targetVolumeFraction >= targetVolFraction*0.99; simp_targetVolumeFraction -=0.05){
        if (!solveSIMP(NULL))
            break;
        callbackFunction(100*(1.0-simp_targetVolumeFraction)/(1.0-targetVolFraction));
	}
    timer.stop();

}
/**************************************************************************************************************************/
void SIMP::getSIMPDensity(double *density)
{
    memcpy(density,simp_pseudoDensity,h_nElems*sizeof(double));
}

/**************************************************************************************************************************/
bool SIMP::solveSIMP(bool (*callBackFunction)(float percentCompleted))
{
    int maxIter,part;
    double *simp_densityOld,startCompliance,compliance;
	double changeDensity =0.0;
	double avgDensity = 0.0;
    float percentCompleted;

    writeMessageToParetoLog("-------------------------------------------");
    writeMessageToParetoLog("SIMP: Target Vol fraction = %f",simp_targetVolumeFraction);
	enableSimpMode();//always needed for SIMP
	disableNodalSpMV();// cannot have node based for SIMP
	enablePreconditioner();// need to have preconditioner for SIMP

	allocateMemory();
	simp_densityOld =  (double *)malloc((h_nElems)*sizeof(double));

	for(int i=0;i<h_nElems;i++)
		simp_pseudoDensity[i]=simp_targetVolumeFraction;

    setPseudoDensity(simp_pseudoDensity);
    setEexponent(3.0);
    disableDeflation();
    enableDeflation();
    maxIter = 100;
    part = 0;

    solveElasticityStatic();
    startCompliance =  computeCompliance();
    for(int i =0; i < maxIter; i++){
		if (!solveElasticityStatic()) {
            writeMessageToGUIWindow("SIMP: Failed to converge ");
			free(simp_densityOld);
			return false;
		}

        if (isDesignPastYield(&part)){
            writeMessageToGUIWindow("SIMP: Approaching yield ");
            free(simp_densityOld);
            return false;
        }
        compliance = computeCompliance();
        if (compliance > 2*startCompliance){
            writeMessageToGUIWindow("SIMP: Large increase in compliance ");
            free(simp_densityOld);
            return false;
        }

		memcpy(simp_densityOld,simp_pseudoDensity,h_nElems*sizeof(double));
		updateSIMPDensity();
		changeDensity =0.0;
		for (int i=0;i<h_nElems;i++){// computing the error norm
			changeDensity += abs(simp_densityOld[i]-simp_pseudoDensity[i]) ;
			avgDensity+=simp_pseudoDensity[i];
		}
		changeDensity /= (h_nElems);     
		avgDensity/=h_nElems;
        //writeMessageToParetoLog("SIMP: change in density: %.2e; vol frac = %.3f",changeDensity,avgDensity);
        percentCompleted =  MIN(100.0,100*simp_densityChangeTerminate/changeDensity);
        if ((i > 5) && (changeDensity < simp_densityChangeTerminate)) break;
        if (callBackFunction != NULL)
            if (!callBackFunction(percentCompleted))
                break;
	}
	disableSimpMode();
	free(simp_densityOld);
	return true;
}
/**************************************************************/
void SIMP::updateSIMPDensity()
{
	computeSIMPSensitivity();
	smoothenElemField(simp_sensitivity,2);
	updateDensitySIMP();
}
/*************************************************************/
void SIMP::computeSIMPSensitivity()
{
	double sol[24];
	double penal =h_Eexponent; 
	double elemCompliance =0.0,meanSensitivity;
	meanSensitivity = 0;
	for(int i =0;i<h_nElems;i++){
		getElemSolution(i,sol);
		elemCompliance = computeElemInnerProduct(i,sol,sol);	
		simp_sensitivity[i] = -penal*(pow (simp_pseudoDensity[i],penal-1))* elemCompliance;
		meanSensitivity += abs(simp_sensitivity[i]);//keep the sign positive
	}
	meanSensitivity /=h_nElems;
	for(int i =0;i<h_nElems;i++)
		simp_sensitivity[i] /= meanSensitivity;// removes the scaling due to Young's Modulus, force applied, etc
}
/****************************************************************/
void SIMP::updateDensitySIMP()
{
    double lambda1 = 0, lambda2 = 1e4,move = 0.2,lmid;
	double *newpseudoDensity = (double *)malloc((h_nElems)*sizeof(double));
	double sensitivityMean,averageDensity;

	sensitivityMean=0;
	for(int i=0;i<h_nElems;i++)
		sensitivityMean += fabs(simp_sensitivity[i]);

	sensitivityMean /=h_nElems;

    lmid = simp_lambdaPrev;
    while (lambda2-lambda1 > 1e-6){
		averageDensity = 0;
		for(int i=0;i<h_nElems;i++){
			newpseudoDensity[i] = MAX(simp_pseudoDensity[i]-move,MIN(simp_pseudoDensity[i]+move,simp_pseudoDensity[i]*sqrt(-simp_sensitivity[i]/(sensitivityMean*lmid))));
			newpseudoDensity[i]  = MIN(1,MAX(simp_minDensity,newpseudoDensity[i]));
            if (h_keepElem[i])
                newpseudoDensity[i]  = 1.0;
			averageDensity += newpseudoDensity[i];
		}
		averageDensity /=h_nElems;
		if (averageDensity > simp_targetVolumeFraction)
			lambda1 = lmid;
		else
			lambda2 = lmid;
        lmid = 0.5*(lambda2+lambda1);
	}
    simp_lambdaPrev = lmid;

    for(int i=0;i<h_nElems;i++){
        if (h_keepElem[i])
            newpseudoDensity[i]  = 1.0;
		simp_pseudoDensity[i] = newpseudoDensity[i];
    }

	setPseudoDensity(simp_pseudoDensity);
	free(newpseudoDensity);
} 
/****************************************************************/
SIMP::~SIMP()
{ 
	safeFree(simp_pseudoDensity);
	safeFree(simp_sensitivity);
}
