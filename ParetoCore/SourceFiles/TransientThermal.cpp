#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <omp.h>
#endif
#include "TransientThermal.h"
using namespace std;
#pragma warning(disable:4996)

/**********************************************************************/
TransientThermal::TransientThermal()
{
    //writeMessageToParetoLog("TransientThermal: Constructing TransientThermal object");
    tt_beta = 1.0;//Trapezoidal Rule Factor
	tt_Keff = NULL;
	tt_Feff = NULL;
	tt_KPrimeEff = NULL;
	tt_sol = NULL;
	tt_x = NULL;// 
	tt_min = 0;
	tt_max = 0;
	tt_deltaTime = 0;
	h_isThereRadiation = false;
	tt_verbose = false;
}
/**********************************************************************/
void  TransientThermal::enableTransientThermalVerbose(void)
{
	tt_verbose = true;
}
/**********************************************************************/
void  TransientThermal::disableTransientThermalVerbose(void)
{
	tt_verbose = false;
}
/**********************************************************************/
void  TransientThermal::releaseCPUMemoryTransientThermal(void)
{
    if (tt_verbose) writeMessageToParetoLog("TransientThermal: Releasing memory");
	// CPU memory deallocation

	safeFree(tt_sol);tt_sol = NULL; 
	safeFree(tt_Keff);tt_Keff = NULL;
	safeFree(tt_KPrimeEff);tt_KPrimeEff = NULL;
	safeFree(tt_Feff);tt_Feff = NULL;
	safeFree(tt_Aeff);tt_Aeff=NULL;
	safeFree(tt_forcePrevious);tt_forcePrevious=NULL;
	safeFree(tt_forceCurrent);tt_forceCurrent=NULL;
	safeFree(tt_forceCurrentWithRadiation);tt_forceCurrentWithRadiation=NULL;

#ifdef COMPILE_GPUCODE 
	releaseHexmeshGPUMemory(&h_gpuData);
#endif
}
/**********************************************************************/
void TransientThermal::allocateTransientThermalMemory(void)
{
	releaseCPUMemoryTransientThermal();

    if (tt_verbose) writeMessageToParetoLog("TransientThermal: Allocating memory");
	tt_Feff =   (double *)malloc((h_nNodes)*sizeof(double));
	tt_sol = (double *)malloc((h_nNodes)*sizeof(double));
	tt_x = (double *)malloc((h_nNodes)*sizeof(double));// 

	tt_Aeff = (double *)malloc((h_nNodes)*sizeof(double));
	tt_forcePrevious = (double *)malloc((h_nNodes)*sizeof(double));
	tt_forceCurrent = (double *)malloc((h_nNodes)*sizeof(double));
	tt_forceCurrentWithRadiation = (double *)malloc((h_nNodes)*sizeof(double));

	memset(tt_Feff,0,h_nNodes*sizeof(double));
	memset(tt_sol,0,h_nNodes*sizeof(double));
	memset(tt_x,0,h_nNodes*sizeof(double));// 
	memset(tt_Aeff,0,h_nNodes*sizeof(double));
	memset(tt_forcePrevious,0,h_nNodes*sizeof(double));
	memset(tt_forceCurrent,0,h_nNodes*sizeof(double));
	memset(tt_forceCurrentWithRadiation,0,h_nNodes*sizeof(double));

}
/**********************************************************/
void TransientThermal::ComputeCE_TT(double dx, double dy, double dz, double p, double c, double Ce[8][8])
{
	// From Matlab
	for(int i = 0; i<8;i++)
		for(int j = 0 ; j<8;j++)
			Ce[i][j] = 0.0;//initialize values

	Ce[0][0] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[0][1] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[0][2] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[0][3] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[0][4] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[0][5] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[0][6] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[0][7] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[1][0] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[1][1] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[1][2] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[1][3] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[1][4] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[1][5] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[1][6] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[1][7] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[2][0] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[2][1] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[2][2] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[2][3] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[2][4] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[2][5] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[2][6] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[2][7] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[3][0] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[3][1] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[3][2] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[3][3] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[3][4] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[3][5] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[3][6] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[3][7] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[4][0] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[4][1] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[4][2] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[4][3] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[4][4] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[4][5] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[4][6] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[4][7] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[5][0] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[5][1] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[5][2] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[5][3] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[5][4] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[5][5] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[5][6] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[5][7] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[6][0] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[6][1] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[6][2] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[6][3] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[6][4] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[6][5] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[6][6] = c*dx*dy*dz*p*(1.0/2.7E1);
	Ce[6][7] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[7][0] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[7][1] = c*dx*dy*dz*p*(1.0/2.16E2);
	Ce[7][2] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[7][3] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[7][4] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[7][5] = c*dx*dy*dz*p*(1.0/1.08E2);
	Ce[7][6] = c*dx*dy*dz*p*(1.0/5.4E1);
	Ce[7][7] = c*dx*dy*dz*p*(1.0/2.7E1);
}
/**********************************************************************/
double TransientThermal::computeThermalFluxAtTimeStep(int timeStep)
{
	//user to customize
    return 0;//constant flux
}
/**********************************************************************/
bool TransientThermal::computeKEffKPrimeEffThermal(double deltaTime) //done only once
{
	double Ke[8][8];
	double Ce[8][8],thermalConductivity,density,specificHeat;
	int k,templateElem,nTemplateIDs,templateID;

    if (tt_verbose) writeMessageToParetoLog("TransientThermal: Computing Keff");
	if (!h_voxelgrid){
        writeMessageToParetoLog("TransientThermal: computeKEffKPrimeEffThermal not implemented for non voxel");
		return false;
	}
	tt_deltaTime = deltaTime;
	if (tt_deltaTime == 0) {
        writeMessageToParetoLog("TransientThermal: Please initialize tt_deltaTime");
		return false;
	}

	nTemplateIDs = getNumTemplateIDs();
	safeFree(tt_Keff);
	safeFree(tt_KPrimeEff);
	tt_Keff = (double *)malloc((nTemplateIDs)*8*8*sizeof(double));// = (C/deltaT + beta*K)
	tt_KPrimeEff = (double *)malloc((nTemplateIDs)*8*8*sizeof(double));// = (C/deltaT - (1-beta)*K)

	for (templateElem = 0; templateElem < nTemplateIDs; templateElem++) {
		for (int elem = 0; elem < h_nElems; elem++){
			templateID = getTemplateId(elem);
			if (templateID == templateElem){
				thermalConductivity = getThermalConductivityOfElem(elem);
				density = getDensityOfElem(elem);
				specificHeat = getSpecificHeatOfElem(elem);
				break;
			}
		}

		ComputeCE_TT(h_dx,h_dy,h_dz,density,specificHeat,Ce);
		ComputeKe_thermal(h_dx,h_dy,h_dz,thermalConductivity,Ke);
		k =0;
		for (int i =0; i < 8; i++) 	{
			for(int j = 0; j < 8;j++) 	{
				tt_Keff[k*(nTemplateIDs) + templateElem] = (Ce[i][j])/tt_deltaTime + tt_beta*Ke[i][j]; 
				tt_KPrimeEff[k*(nTemplateIDs) + templateElem] = (Ce[i][j])/tt_deltaTime - (1-tt_beta)*Ke[i][j]; 
                k++;
			}
		}
	}
	return true;
}
/**********************************************************************/
void TransientThermal::ComputeTransientThermalFeff(int timeStep,bool Secondtime)// done every time step
{	

    if (tt_verbose) writeMessageToParetoLog("TransientThermal: ComputeTransientThermalFeff ");

    if(!Secondtime) {// for the first time step in nonlinear,and for all linear problems
		//	Computing tt_Aeff
		double *Kestore;
		double q1,q2,Tref;
		int nodeLabel=0;
		int nTemplateIDs = getNumTemplateIDs();
		Kestore = (double *)malloc(nTemplateIDs*8*8*sizeof(double));
		memcpy(Kestore,h_KETemplates_thermal,nTemplateIDs*8*8*sizeof(double));

		setKeTemplatesThermal(tt_KPrimeEff);
		KuCPU_thermal(tt_sol,tt_Aeff,tt_beta-1);
		setKeTemplatesThermal(Kestore);
		safeFree(Kestore);Kestore=NULL;

		//	Computing Force Previous		
		if(timeStep > 1)
			if(h_isThereRadiation)
				memcpy(tt_forcePrevious,tt_forceCurrentWithRadiation,h_nNodes*sizeof(double));
			else
				memcpy(tt_forcePrevious,tt_forceCurrent,h_nNodes*sizeof(double));

		//  Computing Force Current
		q1 = computeThermalFluxAtTimeStep(timeStep-1);
		q2 = computeThermalFluxAtTimeStep(timeStep);		
		if(timeStep ==1 && h_isThereRadiation){
			memcpy(tt_forceCurrent,h_f_thermal,h_nNodes*sizeof(double));
			memcpy(tt_forceCurrentWithRadiation,h_f_thermalWithRadiation,h_nNodes*sizeof(double));
		}
		else if(timeStep == 1)
			memcpy(tt_forceCurrent,h_f_thermal,h_nNodes*sizeof(double));
        else {
			for (int i = 0; i<h_nNodes; i++) {
				if(!h_convection[i])//only flux
					tt_forceCurrent[i] = (tt_forceCurrent[i]/(q1))*(q2);
				else {
					nodeLabel = h_nodeLabels[i];
					Tref = h_convectionTRef[nodeLabel];
					if(fabs((h_convection[i]*Tref)-tt_forceCurrent[i])>1.0E-10)//both convection and flux on same node
						tt_forceCurrent[i] = ((tt_forceCurrent[i]-(h_convection[i]*Tref))*(q2)/(q1)) + (h_convection[i]*Tref);
					//If there is only convection, its already taken care of// 
				}
				if(h_isThereRadiation){
					nodeLabel = h_nodeLabels[i];
					Tref = h_radiationTRef[nodeLabel];
					tt_forceCurrentWithRadiation[i] = tt_forceCurrent[i] + h_radiationNonLinear[i]*Tref;
				}
			}
		}
	}
	else{ // second time
		double Tref;
		int nodeLabel=0;
#pragma omp parallel for 
		for (int i = 0; i<h_nNodes; i++){
			nodeLabel = h_nodeLabels[i];
			Tref = h_radiationTRef[nodeLabel];
			tt_forceCurrentWithRadiation[i] = tt_forceCurrent[i] + h_radiationNonLinear[i]*Tref;
		}
	}
	//Computing tt_Feff
	if(h_isThereRadiation)
#pragma omp parallel for 
		for(int i = 0; i<h_nNodes; i++)
			tt_Feff[i] = tt_Aeff[i] + (1-tt_beta)*tt_forcePrevious[i] + tt_beta*tt_forceCurrentWithRadiation[i];
	else
#pragma omp parallel for 
		for(int i = 0; i<h_nNodes; i++)
			tt_Feff[i] = tt_Aeff[i] + (1-tt_beta)*tt_forcePrevious[i] + tt_beta*tt_forceCurrent[i];

}
/**********************************************************************/
void TransientThermal::ComputeTransientThermalFeffLinear(int timeStep)
{
    if (tt_verbose) writeMessageToParetoLog("TransientThermal: ComputeTransientThermalFeffLinear ");

    double *Kestore;
    int nTemplateIDs = getNumTemplateIDs();
    Kestore = (double *)malloc(nTemplateIDs*8*8*sizeof(double));
    memcpy(Kestore,h_KETemplates_thermal,nTemplateIDs*8*8*sizeof(double));

    setKeTemplatesThermal(tt_KPrimeEff);// when beta = 1, tt_KPrimeEff = [C]/delta_t
    KuCPU_thermal(tt_sol,tt_Aeff,tt_beta-1);
    setKeTemplatesThermal(Kestore);
    safeFree(Kestore);Kestore=NULL;

    memcpy(tt_forceCurrent,h_f_thermal,h_nNodes*sizeof(double));

    //Computing tt_Feff
#pragma omp parallel for
    for(int i = 0; i<h_nNodes; i++)
        tt_Feff[i] = tt_Aeff[i] + (1-tt_beta)*tt_forcePrevious[i] + tt_beta*tt_forceCurrent[i];

}
/**********************************************************************/
void TransientThermal::setTransientThermalSolution(double *tSol)
{
	memcpy(tt_sol,tSol,h_nNodes*sizeof(double));
}
/**********************************************************************/
void TransientThermal::setInitialTemperature(double T0)
{
	#pragma omp parallel for
	for(int i = 0; i<h_nNodes;i++) 
		tt_sol[i] = T0;
	enforceDirichlet_thermal(tt_sol);
}
/**********************************************************************/
void TransientThermal::getMinMaxTempTransient(double *tmax, double *tmin)
{
	*tmax = tt_max;
	*tmin = tt_min;
}
/**********************************************************************/
int TransientThermal::solveTransientThermoElasticProblem(double initialTemperature, double *initialTemperatureDistribution, double deltaTime,double totalTime,bool solveElastic, bool (*userFunction)(double))
{
	int nIter,totalTimeSteps,nTemplateIDs,nNodes;
	double *storeKe;
	double *elasticForceSave;
	double transientFactor = 1;

    initializeKEThermal();
	allocateTransientThermalMemory();
	nNodes = getNumNodes();
	if (solveElastic) {
		elasticForceSave = (double *)malloc(3*nNodes*sizeof(double));
		getForceVector(elasticForceSave);// this is the pure elastic force
		initializeSolution();//elastic
        setThermoElasticReferenceTemperature(initialTemperature);
	}
	setInitialTemperature(initialTemperature);
    if (initialTemperatureDistribution != NULL)
        setTransientThermalSolution(initialTemperatureDistribution);// optional
	set_thermalStaticSol(tt_sol);
    tt_deltaTime = deltaTime;//time step
    tt_time = totalTime;

	if ((h_voxelgrid) &&  (h_nodeBasedAssembly)) 
		computeKeNodalThermal();

	computeKEffKPrimeEffThermal(deltaTime);

	nTemplateIDs = getNumTemplateIDs();
	storeKe= (double *)malloc(nTemplateIDs*8*8*sizeof(double));
	memcpy(storeKe,h_KETemplates_thermal,nTemplateIDs*8*8*sizeof(double));// store the static KE 
	setKeTemplatesThermal(tt_Keff);//overwrite the Ke static with the transient Ke

    writeMessageToParetoLog("ws_deflationType = %d ",ws_deflationType);
	if (ws_deflationType !=  DEFLATION_NONE) {// deflation
		CreateDeflationGroups();
		sparseWKWThermal(transientFactor);
	}
	totalTimeSteps = tt_time/tt_deltaTime;
	PhaseChangeInitialization();// 			

	for(int timeStep = 1; timeStep<= totalTimeSteps;timeStep++) {
        //writeMessageToParetoLog("TransientThermal: Step %d/%d ",timeStep,totalTimeSteps);
        if (!userFunction(1.0*timeStep/totalTimeSteps)){// can place other data in user function
            writeMessageToParetoLog("TransientThermal: Terminating on user request");
            break;
		}
		if(h_isThereRadiation){
            //writeMessageToParetoLog("TransientThermal: Solving with radiation");
			computeRadiationNonLinear();
			ComputeTransientThermalFeff(timeStep);
			performSimpleCGThermal(tt_Feff,h_sol_thermal,&nIter,tt_beta);
			solveNonlinearThermalNewtonRaphson(50,timeStep);
		}
		else{
            //writeMessageToParetoLog("TransientThermal: Solving ");
            ComputeTransientThermalFeffLinear(timeStep);
			performCGThermal(tt_Feff,h_sol_thermal,&nIter,tt_beta);
		}
		computeTemperatureMax();
        computeElementTemperature();
		tt_min = h_temperatureMin;
		tt_max = h_temperatureMax;
        //cout << "TMin " << tt_min << "  TMax " << tt_max << endl;
		if(tt_max <= tt_Tms)// no phase change 
			;//
		else if(abs(tt_Tms-tt_Tml) <= 0.1){//isothermal phase change
			PhaseChangeCorrectionIsothermal();
			computeTemperatureMax();
			tt_min = h_temperatureMin;
			tt_max = h_temperatureMax;
		}
		else{//nonisothermal phase change
			PhaseChangeCorrectionNonIsothermal();
			computeTemperatureMax();
			tt_min = h_temperatureMin;
			tt_max = h_temperatureMax;
		}
		setTransientThermalSolution(h_sol_thermal);
		if (solveElastic) {
			addThermalForce();
			solveElasticityStatic();
			setForceVector(elasticForceSave);// else the thermal force will accumulate
		}
	}
	setKeTemplatesThermal(storeKe);// recover back the static thermal Ke
	free(storeKe);
	releaseCPUMemoryTransientThermal();
    if (solveElastic)
		free(elasticForceSave);
	return nIter;
}
/**********************************************************************/
bool	TransientThermal::solveNonlinearThermalNewtonRaphson(int maxIter,int timeStep)
{
	double normErr,*residual,*deltaSol;
	int nCGIter;

	deltaSol = (double *)malloc((h_nNodes)*sizeof(double));
	residual = (double *)malloc((h_nNodes)*sizeof(double));

	for (int iter =1; iter < maxIter ; iter++){
		computeRadiationNonLinear();// compute the nonlinear h_radiation term and nonlinear thermal force 
		ComputeTransientThermalFeff(timeStep,true);
		KuCPU_thermal(h_sol_thermal,residual,tt_beta);// residual = K*sol -f, includes convection and radiation
		axpy_thermal(-1,tt_Feff,residual);
		computeRadiationNonLinear(true);// Jacobian 
		performSimpleCGThermal(residual,deltaSol,&nCGIter,tt_beta); //using Jacobian
		axpy_thermal(-1,deltaSol,h_sol_thermal);
		enforceDirichlet_thermal(h_sol_thermal);
		normErr = Norm_thermal(deltaSol)/(Norm_thermal(h_sol_thermal)+1e-16);
		if ((iter > 1) && (normErr < h_relAccuracyThermal)) break;
	}
	free(deltaSol);
	free(residual);

	return true;
}
/**********************************************************************/
void TransientThermal::PhaseChangeInitialization(void)// 
{
	//Phase Change Terms
	//Material Ti6Al4V
	Cs = 562; //J/kgK
	Cl = 572; //J/kgK
	tt_Tms = 1878; //K
	tt_Tml = 1933; //K
	tt_Latent = 365000; //J/kg
	Hm = 0.0;//J/kg
	Hl = 0.0;//J/kg
	if(abs(tt_Tms-tt_Tml) <= 0.1){//isothermal phase change//
		Hm = Cs*tt_Tms;
		Hl = Hm + tt_Latent;
	}
	else{//nonisothermal phase change
		Hm = Cs*tt_Tms;
		Hl = Hm + tt_Latent + (Cs+Cl)*(tt_Tml-tt_Tms)/2;
	}
}
/**********************************************************************/
void TransientThermal::PhaseChangeCorrectionIsothermal(void)// 
{
	double H1 = 0.0;
	double H2 = 0.0;
	for (int k = 0; k < h_nNodes;k++) {// for all nodes
		if(tt_sol[k] < tt_Tms) //Started as solid
			H1 = Cs*tt_sol[k];
		else if(tt_sol[k] >tt_Tms) //Started as liquid
			H1 = Hl + Cl*(tt_sol[k] - tt_Tms);
		else //Started as melt
			H1 = (1-tt_x[k])*Hm + tt_x[k]*Hl;

		H2 = H1 + Cs*(h_sol_thermal[k]-tt_sol[k]);

		if(H2 <= Hm) //Ended as solid
			h_sol_thermal[k] = H2/Cs;
		else if(H2 <= Hl){ //Ended as melt
			h_sol_thermal[k] = tt_Tms;
			tt_x[k] = (H2-Hm)/tt_Latent;
		}
		else//Ended as liquid
			h_sol_thermal[k] = (H2-Hl)/Cl + tt_Tms;
	}
}
/**********************************************************************/
void TransientThermal::PhaseChangeCorrectionNonIsothermal(void)// 
{
	double H1 = 0.0;
	double H2 = 0.0;
	double A = ((Cl-Cs)/(2*(tt_Tml-tt_Tms)));
	double B = Cs+(tt_Latent/(tt_Tml-tt_Tms));
	double C,Sol1,Sol2;
	for (int k =0; k < h_nNodes; k++){
		if(tt_sol[k] <= tt_Tms) //Started as solid
			H1 = Cs*tt_sol[k];
		else if(tt_sol[k] >= tt_Tml) //Started as liquid
			H1 = Hl + Cl*(tt_sol[k] - tt_Tml);
		else //Started as melt
			H1 = Cs*(tt_sol[k] - tt_Tms) + ((Cl - Cs)*pow((tt_sol[k] - tt_Tms),2)/(2*(tt_Tml - tt_Tms))) + Hm + tt_Latent*(tt_sol[k] - tt_Tms)/(tt_Tml-tt_Tms);

		H2 = H1 + Cs*(h_sol_thermal[k]-tt_sol[k]);

		if(H2 <= Hm) //Ended as solid
			h_sol_thermal[k] = H2/Cs;
		else if(H2 <= Hl){ //Ended as melt
			if(Cl == Cs)
				h_sol_thermal[k] = tt_Tms + (H2-Hm)/(Cs+(tt_Latent/(tt_Tml-tt_Tms))) ;
			else{
				C = Hm-H2;
				Sol1 = (-B + sqrt((pow(B,2))-(4*A*C)))/(2*A)+tt_Tms;
				Sol2 = (-B - sqrt((pow(B,2))-(4*A*C)))/(2*A)+tt_Tms;
				if(Sol1 < tt_Tms)
					h_sol_thermal[k] = Sol2;
				else
					h_sol_thermal[k] = Sol1;
			}
		}
		else//Ended as liquid
			h_sol_thermal[k] = (H2-Hl)/Cl + tt_Tml;
	}
}
/**********************************************************************/
TransientThermal::~TransientThermal()
{
    //writeMessageToParetoLog("TransientThermal: Destructing TransientThermal object");
	
	releaseCPUMemoryTransientThermal();
}
