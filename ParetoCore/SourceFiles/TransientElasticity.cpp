#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <omp.h>
#endif
#include "TransientElasticity.h"
#include "Timer.h"


using namespace Eigen;
using namespace std;
typedef unsigned char byte;
#pragma warning(disable:4996)
#pragma warning( disable : 4101 )//disable warning with openmp

//extern void setPlotRoutine(int control);


/**********************************************************************/
TransientElasticity::TransientElasticity()
{
	tr_nodeBasedAssembly = false;
    tr_verbose = true;
	tr_a1 = 0.;
	tr_a2 = 0.;
	tr_a3 = 0.;
	tr_a4 = 0.;
	tr_a5 = 0.;
	tr_a6 = 0.;
	tr_a7 = 0.;
	tr_MDcoef = 0.;
	tr_MDdotcoef = 0.;
	tr_MDdotdotcoef = 0.;
	tr_KDcoef = 0.;
	tr_KDdotcoef = 0.;
	tr_KDdotdotcoef = 0.;
	tr_N_gamma = 0.;
	tr_N_beta = 0.;
	tr_Damp_alpha = 0.;
	tr_Damp_beta = 0.;
	tr_h = 0.;
	tr_CurrentTime = 0.;
	tr_TimeIndex = 0;
	tr_tolerance = 0.;
	tr_NumDOF = 0;
	tr_SolOld = NULL; 
	tr_SolDotOld = NULL; 
	tr_SolDotDotOld = NULL;
	tr_SolCurrent = NULL; 
	tr_SolDot = NULL;
	tr_SolDotDot = NULL;
	tr_MD = NULL; 
	tr_MDdot = NULL;
	tr_MDdotdot = NULL;
	tr_KD = NULL; 
	tr_KDdot = NULL;
	tr_KDdotdot = NULL;
	tr_staticForce = NULL;
	tr_Feff = NULL;
	tr_M_coef = NULL;
	tr_K_coef = NULL;
	tr_nTemplateElements = 0;
	tr_h_KEeffTemplates = NULL;
	tr_h_MEeffTemplates = NULL;
	tr_h_KEdummy = NULL;
	// Stress
	tr_vonMises = 0.0;
	// Weight
	tr_Weight = NULL;
	// Drop Test
	dr_Height = 0.0;
	dr_fallingZVelocity = 0.0;
	dr_Fcontact = NULL;
	// Force_Based
	dr_nContactNodes =0;
	dr_ContactNodes = NULL;
	dr_nBoundaryNodes = 0;
	dr_BoundaryNodes = NULL;
	dr_ContactRadius = 0;
	dr_VoxelKprime = 0.0;
	dr_E = 0.0;
	dr_Nu = 0.0;
	// Dirichlet-Based
	dr_tempFixed = NULL;


    //
    tr_nFrequencySteps = 0;
    tr_omegaRadiansIn = NULL;
    tr_deltaMaxOut = NULL;
    tr_phaseAngleRadiansOut = NULL;

}
/**********************************************************************/
int TransientElasticity::initializeTransientElasticity(void)
{
	tr_nodeBasedAssembly = isSpMVMethodNodeBased();
	InitializeTransientOnCPU();
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: initializeTransientElasticity()\n");
#ifdef COMPILE_TRANS_GPUCODE
	if (usingGPU) {
		InitializeTransElasticityOnGPU(h_nNodes,&gpuTr,tr_Track_DOF);
	}
#endif

	return 1;
}
/**********************************************************************/
void TransientElasticity::setDampingParams( double dampingM, double dampingK)
{
    tr_Damp_alpha = dampingM;
    tr_Damp_beta = dampingK;
}
/**********************************************************************/
void TransientElasticity::setTransientParams(double newmarkGamma, double newmarkBeta,
											 double dampingM, double dampingK,double transientDeltaTime,double transientTotalTime, 
											 double tolerance)
{
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: setTransientParams()\n");

	setTransientParamsOnCPU(newmarkGamma,newmarkBeta,dampingM,dampingK,transientDeltaTime,
		transientTotalTime,tolerance);

#ifdef COMPILE_TRANS_GPUCODE
	if (usingGPU) {
        if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: setTransientParams On GPU()\n");
		setTransientParamsOnGPU(&gpuTr,newmarkGamma,newmarkBeta,dampingM,dampingK,transientDeltaTime,
			transientTotalTime,tolerance);
	}
#endif	
}
/**********************************************************************/
void TransientElasticity::solveElasticTransientCPUGPU(bool(*callBackFunction)(double fractionComplete))
{
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: solveElasticTransientCPUGPU()\n");

#ifdef COMPILE_TRANS_GPUCODE
	if (usingGPU) {
		NewmarkSolverOnGPU(&gpuTr);
	}
	else
#endif
        NewmarkSolver(callBackFunction);
}
/**********************************************************************	TransientElasticity On CPU	**********************************************************************/
void TransientElasticity::NewmarkSolver(bool(*callBackFunction)(double fractionComplete))
{
    if (tr_TimeIndex == 0)
        tr_diff = 1.0;

    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: NewmarkSolver on CPU\n");
    while((tr_diff > tr_tolerance)&&(tr_CurrentTime< tr_totalTime))
    {
        callBackFunction(tr_CurrentTime/tr_totalTime);
        writeMessageToParetoLog("TimeIndex = %d; tr_CurrentTime = %.6f",tr_TimeIndex,tr_CurrentTime);
		if (tr_TimeIndex == 0){
			allocateSol();
			initializeTransientForceVector();
			computeTransientCoefficients();
			setSolutionAtAllNodes(tr_SolCurrent);
			findKEeffTemplate();
			findVelocityAcceleration();
			if (TR_GRAVITY) findWeight();
			if (tr_Track) initializeTracking();
		}
		updateSol();
		if (tr_Track) updateTracking();
		computeEffectiveF();
		setForceVector(tr_Feff);
		setKeTemplates(tr_h_KEeffTemplates);// copy voxel Keff to voxel K
		computeKeNodalFromKeTemplate();
		if (tr_TimeIndex == 0){
			ReleaseWorkSpace();
			InitializeDeflationForElasticity();
		}
		solveElasticityStatic();
		postprocess();
		getSolutionVector(tr_SolCurrent);
		findVelocityAcceleration();
		setKeTemplates(tr_h_KEdummy);// copy temp K to voxel K
		computeKeNodalFromKeTemplate();
		tr_TimeIndex++;
		tr_CurrentTime += tr_h; 
		tr_diff = normDisplacementChange();
        //setPlotRoutine(1);
    }
    //if (tr_Track)
        //normalizeTracked();
	//// reset the static force when done
    setForceVector(tr_staticForce);
}
/**********************************************************************/
void TransientElasticity::allocateSol(void)
{
	safeFree(tr_SolOld);
	safeFree(tr_SolDotOld);
	safeFree(tr_SolDotDotOld);
	safeFree(tr_SolCurrent);
	safeFree(tr_SolDot);
	safeFree(tr_SolDotDot);

	safeFree(tr_MD);
	safeFree(tr_MDdot);
	safeFree(tr_MDdotdot);
	safeFree(tr_KD);
	safeFree(tr_KDdot);
	safeFree(tr_KDdotdot);
	safeFree(tr_Feff);

	safeFree(tr_h_KEdummy);

	tr_SolOld = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_SolDotOld = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_SolDotDotOld = (double *) malloc(tr_NumDOF*sizeof(double));

	tr_SolCurrent = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_SolDot = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_SolDotDot = (double *) malloc(tr_NumDOF*sizeof(double));

	tr_MD = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_MDdot = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_MDdotdot = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_KD = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_KDdot = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_KDdotdot = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_Feff = (double *) malloc(tr_NumDOF*sizeof(double));

	int nTemplateElements = getNumTemplateElements();
	tr_h_KEdummy = (double *)malloc((nTemplateElements)*24*24*sizeof(double));

	memset(tr_SolOld,0,tr_NumDOF*sizeof(double));
	memset(tr_SolDotOld,0,tr_NumDOF*sizeof(double));
	memset(tr_SolDotDotOld,0,tr_NumDOF*sizeof(double));
	memset(tr_SolCurrent,0,tr_NumDOF*sizeof(double));
	memset(tr_SolDot,0,tr_NumDOF*sizeof(double));
	memset(tr_SolDotDot,0,tr_NumDOF*sizeof(double));
	memset(tr_MD,0,tr_NumDOF*sizeof(double));
	memset(tr_MDdot,0,tr_NumDOF*sizeof(double));
	memset(tr_MDdotdot,0,tr_NumDOF*sizeof(double));
	memset(tr_KD,0,tr_NumDOF*sizeof(double));
	memset(tr_KDdot,0,tr_NumDOF*sizeof(double));
	memset(tr_KDdotdot,0,tr_NumDOF*sizeof(double));
	memset(tr_Feff,0,tr_NumDOF*sizeof(double));
	memset(tr_h_KEdummy,0,(nTemplateElements)*24*24*sizeof(double));

	getKeTemplates(tr_h_KEdummy);// copy voxel K to temp K
}
/**********************************************************************/
int TransientElasticity::InitializeTransientOnCPU(void)
{
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: InitializeTransientOnCPU()\n");
	tr_N_gamma = 0.5;
	tr_N_beta = pow((2*tr_N_gamma+1),2.)/16.;

	tr_h = 1e-4;
	tr_totalTime = 1e-2;
	tr_CurrentTime = 0;
	tr_TimeIndex = 0;

	tr_Damp_alpha = 0.0;
	tr_Damp_beta =1e-4;

	tr_tolerance = 1E-10; 
	tr_NumDOF = 3*h_nNodes;

	tr_TrackPt[0] = 0.5; 
	tr_TrackPt[1] = 0.0; 
	tr_TrackPt[2] = 0.0; 
	setTrackingOn(tr_TrackPt,1); // setTrackingOn(point([x y z]),direction(x=0 y=1 z=2))

	TR_GRAVITY = false; // Gravity exists? true/false
	return 1;
}
/**********************************************************************/
void TransientElasticity::initializeTransientForceVector()
{
	safeFree(tr_staticForce);
	safeFree(tr_Weight);

	tr_staticForce = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_Weight = (double *) malloc(tr_NumDOF*sizeof(double));

	memset(tr_staticForce,0,tr_NumDOF*sizeof(double));
	memset(tr_Weight,0,tr_NumDOF*sizeof(double));

	getForceVector(tr_staticForce);
}
/**********************************************************************/
void TransientElasticity::setTransientParamsOnCPU(double newmarkGamma, double newmarkBeta,
												  double dampingM, double dampingK,double transientDeltaTime,double transientTotalTime, 
												  double tolerance)
{
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: setTransientParamsOnCPU()\n");
	tr_N_gamma = newmarkGamma;
	tr_N_beta = newmarkBeta;
	tr_Damp_alpha = dampingM;
	tr_Damp_beta = dampingK;

	tr_h = transientDeltaTime;
	tr_totalTime = transientTotalTime;
	tr_tolerance = tolerance;
	if (TR_GRAVITY) setGravity();
}
/**********************************************************************/
void TransientElasticity::setGravity(void)
{
	tr_gravity[0] = 0.0;
	tr_gravity[1] = 0.0;
	tr_gravity[2] = -9.81;
}
/**********************************************************************/
void TransientElasticity::KdTimesX(double *x, double *y, double w)
{
    //KD = K - w*w*M
    // y = KD*x;
    double *y1,*y2;

    y1 = (double *)malloc(3*h_nNodes*sizeof(double));
    y2 = (double *)malloc(3*h_nNodes*sizeof(double));
    MuCPUGPU(x,y1);
    KuCPUGPU(x,y2);

#pragma omp parallel for
    for (int i =0; i < 3*h_nNodes; i++)
        y[i] = y2[i]- w*w*y1[i] ;

    free(y1);
    free(y2);
}
/**********************************************************************/
void TransientElasticity::CdTimesX(double *x, double *y, double w)
{
    //CD = w*(alpha_M*M + beta_K*K)
    // y = CD*x;
    double *y1,*y2;

    y1 = (double *)malloc(3*h_nNodes*sizeof(double));
    y2 = (double *)malloc(3*h_nNodes*sizeof(double));
    MuCPUGPU(x,y1);
    KuCPUGPU(x,y2);

#pragma omp parallel for
    for (int i =0; i < 3*h_nNodes; i++)
        y[i] = w*tr_Damp_alpha*y1[i] + w*tr_Damp_beta*y2[i];

    free(y1);
    free(y2);
}
void TransientElasticity::saveFrequencyResponseData(int nFrequencySteps, double *omegaRadians,
                                                    double *deltaMax, double *phaseAngleRadians)
{
    // Store the frequency response for plotting
    tr_nFrequencySteps = nFrequencySteps;
    safeFree(tr_omegaRadiansIn);
    safeFree(tr_deltaMaxOut);
    safeFree(tr_phaseAngleRadiansOut);

    tr_omegaRadiansIn = (double *)malloc(nFrequencySteps*sizeof(double));
    tr_deltaMaxOut = (double *)malloc(nFrequencySteps*sizeof(double));
    tr_phaseAngleRadiansOut = (double *)malloc(nFrequencySteps*sizeof(double));


    for (int step = 0; step < nFrequencySteps; step++){
        tr_omegaRadiansIn[step] = omegaRadians[step];
        tr_deltaMaxOut[step] = deltaMax[step];
        tr_phaseAngleRadiansOut[step] = phaseAngleRadians[step];
    }

}

/**********************************************************************/
bool TransientElasticity::frequencyResponseForLowFrequencies(int nFrequencySteps,double *omegaRadiansIn,
                                                             double *deltaMaxOut, double *phaseAngleRadiansOut,
                                                             bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // This is an iterative process where the real and imaginary components are computed alernately

    // Algorithm

    // 1. x_0 = K\f;
    // 2. x_R = x_0;
    // 2. x_I = 0;
    // 2. x_R = K\(f + M*w*w*x_R + (alpha*M*w+beta*K*w)*x_I);
    // 3. x_I = K\(    M*w*w*x_I - (alpha*M*w+beta*K*w)*x_R);
    // Repeat steps 2 and 3 until convergence

    int i,j,k,maxIter,iter,index;
    double *x_0, *x_R, *x_I, *x,w,*Mx_R,*Kx_R,*Mx_I,*Kx_I;
    double deltaMax,deltaMaxOld;// stiffness and mass
    double *fx_R,*fx_I,*f0;

    if (getNumTemplateElements() > 1){
        writeMessageToGUIWindow("Error: frequencyResponse only works for single template");
        return false;
    }

    x_0 = (double *)malloc(3*h_nNodes*sizeof(double));
    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));
    x = (double *)malloc(3*h_nNodes*sizeof(double));
    Mx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Mx_I = (double *)malloc(3*h_nNodes*sizeof(double));
    Kx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Kx_I = (double *)malloc(3*h_nNodes*sizeof(double));


    fx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    fx_I = (double *)malloc(3*h_nNodes*sizeof(double));
    f0 = (double *)malloc(3*h_nNodes*sizeof(double));


    memset(x_I,0,3*h_nNodes*sizeof(double));
    memset(x_R,0,3*h_nNodes*sizeof(double));
    memset(x,0,3*h_nNodes*sizeof(double));
    memset(f0,0,3*h_nNodes*sizeof(double));


    getForceVector(f0);

    computeKeNodalFromKeTemplate();// for node-based solver
    ReleaseWorkSpace();
    InitializeDeflationForElasticity();

    bool success;
    maxIter = 5;
    for (int step = 0; step < nFrequencySteps; step++){
        w = omegaRadiansIn[step];
        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        if (step ==0){
            memset(x_I,0,3*h_nNodes*sizeof(double));
            memset(x_R,0,3*h_nNodes*sizeof(double));
            solveElasticityStatic();//x_R = K\f
            getSolutionVector(x_R);
        }
        // else recyclce previous solution

        deltaMaxOld = 1.0;
        double relaxationFactor;
        success = false;
        relaxationFactor = 0.9;
        for ( iter = 0; iter < maxIter; iter++){
            KuCPUGPU(x_R,Kx_R);
            KuCPUGPU(x_I,Kx_I);
            MuCPUGPU(x_R,Mx_R);
            MuCPUGPU(x_I,Mx_I);

            #pragma omp parallel for
            for (int i =0; i < 3*h_nNodes; i++){
                fx_R[i] = f0[i] + relaxationFactor*(w*w*Mx_R[i] + tr_Damp_alpha*w*Mx_I[i] + tr_Damp_beta*w*Kx_I[i]);
                fx_I[i] =         relaxationFactor*(w*w*Mx_I[i] - tr_Damp_alpha*w*Mx_R[i] - tr_Damp_beta*w*Kx_R[i]);
            }

            setForceVector(fx_R);
            solveElasticityStatic();//x_R = K\(f + CD*x_I)
            getSolutionVector(x_R);

            setForceVector(fx_I);
            solveElasticityStatic();
            getSolutionVector(x_I);

            deltaMax = 0;
            index = 0;
            for (int i =0; i < 3*h_nNodes; i++){
                x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
                if (x[i] > deltaMax){
                    deltaMax = MAX(deltaMax,x[i]);
                    index = i;
                }
            }
            phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);

            if ( fabs(deltaMaxOld-deltaMax)/deltaMax < 0.1){
                success = true;
                break;

            }
            deltaMaxOld = deltaMax;

        }
        if (success)
             writeMessageToGUIWindow(" f = %.2f; Iter: %d, deltaMax = %e",w/(2*3.1415), iter,deltaMax);
        else
             writeMessageToGUIWindow(" f = %.2f; Iter: %d, deltaMax = %e ***",w/(2*3.1415), iter,deltaMax);

        deltaMaxOut[step] =deltaMax;
    }


    setForceVector(f0);// set back to static force




    free(x_0);
    free(x_R);
    free(x_I);

    free(Mx_R);
    free(Mx_I);
    free(Kx_R);
    free(Kx_I);

    free(f0);
    free(fx_R);
    free(fx_I);
    free(x);

    return true;

}
/**********************************************************************/
bool TransientElasticity::frequencyResponseDirectOld(int nFrequencySteps,double *omegaRadiansIn,double *deltaMaxOut,
                                                  double *phaseAngleRadiansOut,bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // Uses a direct solver
    // Ax = b

    // Matrix A:
    // [K-w*w*M             -alpha*w*M-beta*w*K;
    //  alpha*w*M+beta*w*K      K-w*w*M]

    int i,j,k,index;
    double *x_R, *x_I,*x,w,deltaMax;
    double *Ke,*Me,maxValue;
    int freeEncoded, wfree,vfree,ufree;
    Timer tempTimer;

    tempTimer.start();
    // for direct solver
    VectorXd                            xVector;
    VectorXd                            bVector;
    MatrixXd                            denseAMatrix;
    Eigen::SparseMatrix<double>         sparseAMatrix;
    SimplicialLDLT<SparseMatrix<double>> sparseAMatrixSolver;

    if (getNumTemplateElements() > 1){
        writeMessageToGUIWindow("Error: frequencyResponse only works for single template");
        return false;
    }
    if (h_nNodes > 5000){
        writeMessageToGUIWindow("Error: #Nodes (%d) too large for direct solver",h_nNodes);
        return false;
    }
    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));
    x = (double *)malloc(3*h_nNodes*sizeof(double));
    Ke =	(double *)malloc(24*24*sizeof(double));
    Me =	(double *)malloc(24*24*sizeof(double));

    bVector.resize(6*h_nNodes);
    bVector.setZero(6*h_nNodes);

    xVector.resize(6*h_nNodes);
    xVector.setZero(6*h_nNodes);

    for (int dof = 0; dof < 3*h_nNodes; dof++)
        bVector(dof) = h_f[dof];

     // code crashes for the large matrices below
    denseAMatrix.resize(6*h_nNodes,6*h_nNodes);


    getKeTemplates(Ke);
    getMeTemplates(Me);

    int node,globalDof[24],elemNodes[8],offset;

    for (int step = 0; step < nFrequencySteps; step++){
        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];

        denseAMatrix.setZero(6*h_nNodes,6*h_nNodes);

        offset = 3*h_nNodes;
        // Assemble the A matrix
        for (int elem = 0; elem < h_nElems; elem++){// assemble
            getElemNodes(elem,elemNodes);
            for (int i = 0; i < 8; i++){
                node = elemNodes[i];
                globalDof[3*i] = 3*node;
                globalDof[3*i+1] = 3*node+1;
                globalDof[3*i+2] = 3*node+2;
            }
            k = 0;
            for ( i = 0; i < 24; i++){
                for ( j = 0; j < 24; j++){
                    denseAMatrix(globalDof[i],globalDof[j]) += (Ke[k] - w*w*Me[k]); // top left
                    denseAMatrix(globalDof[i]+offset,globalDof[j]+offset) += (Ke[k] - w*w*Me[k]); // bottom right
                    denseAMatrix(globalDof[i]+offset,globalDof[j]) += (tr_Damp_alpha*w*Me[k] + tr_Damp_beta*w*Ke[k]); // bottom left
                    denseAMatrix(globalDof[i],globalDof[j]+offset) -= (tr_Damp_alpha*w*Me[k] + tr_Damp_beta*w*Ke[k]); // bottom right
                    k++;
                }
            }
        }

        maxValue = denseAMatrix.maxCoeff();


        // for all fixed nodes, make the diagonals huge
        for (int node = 0; node < h_nNodes; node++){
            freeEncoded = h_free[node];
            wfree = freeEncoded/4;// 0 (fixed) or 1 (free)
            vfree = (freeEncoded - wfree*4)/2;// 0 or 1
            ufree = (freeEncoded - wfree*4 -vfree*2);// 0 or 1
            if (ufree == 0){
                bVector(3*node) = 0;
                denseAMatrix(3*node,3*node) = 1e5*maxValue;
            }
            if (vfree == 0){
                bVector(3*node+1) = 0;
                denseAMatrix(3*node+1,3*node+1) = 1e5*maxValue;
            }
            if (wfree == 0){
                bVector(3*node+2) = 0;
                denseAMatrix(3*node+2,3*node+2) = 1e5*maxValue;
            }
        }
        // Zero diagonals
        for (int dof = 0; dof < 3*h_nNodes; dof++){
            if (fabs(denseAMatrix(dof,dof)) < TOL){
                denseAMatrix(dof,dof) = 1e5*maxValue;
                bVector(dof) = 0;
            }
        }
        sparseAMatrix = denseAMatrix.sparseView();

        sparseAMatrixSolver.compute(sparseAMatrix);
        if (sparseAMatrixSolver.info() != 0) {
            writeMessageToParetoLog("Sparse LLT decomposition failed");
            continue;
        }
        xVector = sparseAMatrixSolver.solve(bVector);

        for (int dof = 0; dof < 3*h_nNodes; dof++){
            x_R[dof] = xVector(dof);
            x_I[dof] = xVector(3*h_nNodes+dof);
        }

        deltaMax = 0;
        for (int i =0; i < 3*h_nNodes; i++){
            x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
            if (x[i] > deltaMax){
                deltaMax = x[i];
                index = i;
            }
        }
        phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
        setSolutionAtAllNodes(x);
        computeDeformationMax();
        deltaMaxOut[step] = getDeformationMax();

    }

    free(x);
    free(x_R);
    free(x_I);
}

double addValues(const double &oldValue,const double &newValue)
{
    return oldValue+newValue;
}
/**********************************************************************/
bool TransientElasticity::frequencyResponseDirect(int nFrequencySteps,double *omegaRadiansIn,double *deltaMaxOut,
                                                  double *phaseAngleRadiansOut,bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // Uses a direct solver
    // Ax = b

    // Matrix A:
    // [K-w*w*M             -alpha*w*M-beta*w*K;
    //  alpha*w*M+beta*w*K      K-w*w*M]

    int i,j,k,index;
    double *x_R, *x_I,*x,w,deltaMax;
    double *Ke,*Me;
    int freeEncoded, wfree,vfree,ufree;
    Timer tempTimer;

    // for direct solver
    VectorXd                            xVector;
    VectorXd                            bVector;
    Eigen::SparseMatrix<double>         sparseAMatrix(6*h_nNodes,6*h_nNodes);
    typedef Eigen::Triplet<double> T;
    std::vector<T> coefficients;            // list of non-zeros coefficients (row,column,value)
    SimplicialCholesky<SparseMatrix<double>> sparseAMatrixSolver;

    if (getNumTemplateElements() > 1){
        writeMessageToGUIWindow("Error: frequencyResponse only works for single template");
        return false;
    }

    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));
    x = (double *)malloc(3*h_nNodes*sizeof(double));
    Ke =	(double *)malloc(24*24*sizeof(double));
    Me =	(double *)malloc(24*24*sizeof(double));

    bVector.resize(6*h_nNodes);
    bVector.setZero(6*h_nNodes);

    xVector.resize(6*h_nNodes);
    xVector.setZero(6*h_nNodes);

    for (int dof = 0; dof < 3*h_nNodes; dof++)
        bVector(dof) = h_f[dof];

    getKeTemplates(Ke);
    getMeTemplates(Me);

    int node,globalDof[24],elemNodes[8],offset;

    FILE *fptr;
    bool dumpData;// this is for open box model
    dumpData = true;
    if (dumpData){
        fptr = fopen("OpenBoxResults.txt","w");
    }

    for (int step = 0; step < nFrequencySteps; step++){
        tempTimer.start();
        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];

        coefficients.clear();
        offset = 3*h_nNodes;
        // Assemble the A matrix
        for (int elem = 0; elem < h_nElems; elem++){// assemble
            getElemNodes(elem,elemNodes);
            for (int i = 0; i < 8; i++){
                node = elemNodes[i];
                globalDof[3*i] = 3*node;
                globalDof[3*i+1] = 3*node+1;
                globalDof[3*i+2] = 3*node+2;
            }
            k = 0;
            for ( i = 0; i < 24; i++){
                for ( j = 0; j < 24; j++){
                    coefficients.push_back(T(globalDof[i], globalDof[j],(Ke[k] - w*w*Me[k])));
                    coefficients.push_back(T(globalDof[i]+offset, globalDof[j]+offset,(Ke[k] - w*w*Me[k])));
                    coefficients.push_back(T(globalDof[i]+offset, globalDof[j],(tr_Damp_alpha*w*Me[k] + tr_Damp_beta*w*Ke[k])));
                    coefficients.push_back(T(globalDof[i], globalDof[j]+offset,-(tr_Damp_alpha*w*Me[k] + tr_Damp_beta*w*Ke[k])));

                    k++;
                }
            }
        }
        double maxValue;
        maxValue = 0;
        for (std::vector<T>::iterator it = coefficients.begin() ; it != coefficients.end(); ++it)
            maxValue = MAX(maxValue,(*it).value());

        // for all fixed nodes, make the diagonals huge
        for (int node = 0; node < h_nNodes; node++){
            freeEncoded = h_free[node];
            wfree = freeEncoded/4;// 0 (fixed) or 1 (free)
            vfree = (freeEncoded - wfree*4)/2;// 0 or 1
            ufree = (freeEncoded - wfree*4 -vfree*2);// 0 or 1
            if (ufree == 0){
                bVector(3*node) = 0;
                coefficients.push_back(T(3*node,3*node,maxValue*1e2));
            }
            if (vfree == 0){
                bVector(3*node+1) = 0;
                coefficients.push_back(T(3*node+1,3*node+1,maxValue*1e2));
            }
            if (wfree == 0){
                bVector(3*node+2) = 0;
                coefficients.push_back(T(3*node+2,3*node+2,maxValue*1e2));
            }
        }
        sparseAMatrix.setZero();
        sparseAMatrix.setFromTriplets(coefficients.begin(), coefficients.end());


        sparseAMatrixSolver.compute(sparseAMatrix);
        if (sparseAMatrixSolver.info() != 0) {
            writeMessageToParetoLog("Sparse decomposition failed");
            continue;
        }

        xVector = sparseAMatrixSolver.solve(bVector);
        for (int dof = 0; dof < 3*h_nNodes; dof++){
            x_R[dof] = xVector(dof);
            x_I[dof] = xVector(3*h_nNodes+dof);
        }

        deltaMax = 0;
        for (int i =0; i < 3*h_nNodes; i++){
            x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
            if (x[i] > deltaMax){
                deltaMax = x[i];
                index = i;
            }
        }
        phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
        deltaMaxOut[step] = deltaMax;

        if (dumpData){
            int counter,actualCounter;
            int skip;
            double xyzNode[3];
            fprintf(fptr,"%.3g \n",w/(2*PI));
            counter = 0;
            actualCounter = 0;
            skip = 60;
            for (int node =0; node < h_nNodes; node++){
                getNodeXYZ(node,xyzNode);
                if ((fabs(xyzNode[1]-0)<1e-6) && (counter%skip == 0)){// on XZ plane
                    fprintf(fptr,"%d %.3g %.3g %.3g   %.3g %.3g %.3g   %.3g %.3g %.3g\n",actualCounter,xyzNode[0],xyzNode[1],xyzNode[2],
                            x_R[3*node],x_R[3*node+1],x_R[3*node+2],
                            x_I[3*node],x_I[3*node+1],x_I[3*node+2]);
                    actualCounter++;
                }
                counter++;
            }


        }
    }
    if (dumpData){
        fclose(fptr);
    }

    free(x);
    free(x_R);
    free(x_I);
}
/**********************************************************************/
bool TransientElasticity::frequencyResponseModalSuperPosition(int nFrequencySteps,double *omegaRadiansIn,double *deltaMaxOut,
                                                  double *phaseAngleRadiansOut,bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied
    // Uses Modal super position

    int i,j,k,index;
    double *x_R, *x_I,*x,w,deltaMax,*y1;
    double xeta[1000],lambda[1000],scaling[1000];

    if (getNumTemplateElements() > 1){
        writeMessageToGUIWindow("Error: frequencyResponse only works for single template");
        return false;
    }

    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));
    x = (double *)malloc(3*h_nNodes*sizeof(double));

    y1 = (double *)malloc(3*h_nNodes*sizeof(double));

    for (int i = 0; i < h_nEigenModes;i++){
        lambda[i] = h_eigenValues[i]*2*3.1415;
        KuCPUGPU(h_eigenModes[i],y1);
        scaling[i] = sqrt(lambda[i]*lambda[i]/dotprod(y1,h_eigenModes[i]));
        //cout << pow(scaling[i],2.0)*dotprod(h_eigenModes[i],y1) <<  " " << lambda[i]*lambda[i] << endl;// check for scaling
        xeta[i] = pow(scaling[i],2.0)*tr_Damp_beta*dotprod(y1,h_eigenModes[i])/(2*lambda[i]);
        MuCPUGPU(h_eigenModes[i],y1);
        xeta[i] += pow(scaling[i],2.0)*tr_Damp_alpha*dotprod(y1,h_eigenModes[i])/(2*lambda[i]);
        //cout << pow(scaling[i],2.0)*dotprod(h_eigenModes[i],y1) << endl;// check for scaling

    }

    double a,b;
    for (int step = 0; step < nFrequencySteps; step++){
        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];

        memset(x_R,0,3*h_nNodes*sizeof(double));
        memset(x_I,0,3*h_nNodes*sizeof(double));
        for (int i = 0; i < h_nEigenModes;i++){
            a = scaling[i]*dotprod(h_eigenModes[i],h_f);
            b = pow(lambda[i]*lambda[i] - w*w,2.0) + pow(2*xeta[i]*lambda[i]*w,2.0);
            axpy(scaling[i]*(lambda[i]*lambda[i] - w*w)*a/b,h_eigenModes[i],x_R);
            axpy(-2*scaling[i]*xeta[i]*lambda[i]*w*a/b,h_eigenModes[i],x_I);
        }
        deltaMax = 0;
        for (int i =0; i < 3*h_nNodes; i++){
            x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
            if (x[i] > deltaMax){
                deltaMax = x[i];
                index = i;
            }
        }
        phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
        deltaMaxOut[step] = deltaMax;
    }

    free(x);
    free(x_R);
    free(x_I);
    free(y1);
}
/**********************************************************************/
bool TransientElasticity::frequencyResponseForHighFrequencies(int nFrequencySteps,double *omegaRadiansIn,
                                                              double *deltaMaxOut, double *phaseAngleRadiansOut,
                                                              bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // This is an iterative process where the real and imaginary components are computed alernately

    // Algorithm

    // 1. x_0 = Kd\f;
    // 2. x_R = 0;
    // 2. x_I = 0;
    // 2. x_R = Kd\(Kd*x_R - relax*(Kd*x_R + Cd*x_I - f));
    // 3. x_I = Kd\(Kd*x_I - relax*(-Cd*x_R + Kd*x_I));
    // Repeat steps 2 and 3 until convergence

    int i,j,k,maxIter,iter,index;
    double *x_0, *x_R, *x_I, *x,w,*Kdx_R,*Cdx_R,*Kdx_I,*Cdx_I, *Ke, *Me, *Ke_D;
    double deltaMax,deltaMaxPrev;
    double *fx_R,*fx_I,*f0;
    double relaxationFactor;
    bool success;

    if (getNumTemplateElements() > 1){
        writeMessageToGUIWindow("Error: frequencyResponse only works for single template");
        return false;
    }

    x_0 = (double *)malloc(3*h_nNodes*sizeof(double));
    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));


    x = (double *)malloc(3*h_nNodes*sizeof(double));
    Kdx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Kdx_I = (double *)malloc(3*h_nNodes*sizeof(double));
    Cdx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Cdx_I = (double *)malloc(3*h_nNodes*sizeof(double));

    Ke =	(double *)malloc(24*24*sizeof(double));
    Me =	(double *)malloc(24*24*sizeof(double));
    Ke_D =	(double *)malloc(24*24*sizeof(double));

    fx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    fx_I = (double *)malloc(3*h_nNodes*sizeof(double));
    f0 = (double *)malloc(3*h_nNodes*sizeof(double));


    memset(x_I,0,3*h_nNodes*sizeof(double));
    memset(x_R,0,3*h_nNodes*sizeof(double));
    memset(x,0,3*h_nNodes*sizeof(double));
    memset(f0,0,3*h_nNodes*sizeof(double));

    getForceVector(f0);
    getKeTemplates(Ke);
    getMeTemplates(Me);
    maxIter = 100;
    relaxationFactor = 1.001;
    for (int step = 0; step < nFrequencySteps; step++){
       deltaMaxOut[step] = 0.0;
        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];


        k =0;
        for (i =0; i < 24; i++) {
            for( j = 0; j < 24;j++) {
                Ke_D[k] =  Ke[k] - w*w*Me[k];
                k++;
            }
        }
        setKeTemplates(Ke_D);// copy Ke_D to voxel K
        computeKeNodalFromKeTemplate();// for node-based solver


        ReleaseWorkSpace();
        InitializeDeflationForElasticity();// Since Ke has changed

        memset(x_I,0,3*h_nNodes*sizeof(double));
        memset(x_R,0,3*h_nNodes*sizeof(double));
        success = false;
        deltaMaxPrev = 1.0;
        for ( iter = 1; iter < maxIter; iter++){
            CdTimesX(x_I, Cdx_I,w);
            KdTimesX(x_R, Kdx_R, w);

            for (int i =0; i < 3*h_nNodes; i++)
                fx_R[i] = Kdx_R[i] - relaxationFactor*( Kdx_R[i] - Cdx_I[i]-f0[i]);

            setForceVector(fx_R);
            solveElasticityStatic();//x_R = Kd\(Kd*xR - relax(Kd*x_R + Cd*X_I -f)
            getSolutionVector(x_R);

            CdTimesX(x_R, Cdx_R,w);
            KdTimesX(x_I, Kdx_I, w);

            for (int i =0; i < 3*h_nNodes; i++)
                fx_I[i] = Kdx_I[i] - relaxationFactor*( Cdx_R[i] + Kdx_I[i]);

            setForceVector(fx_I);
            solveElasticityStatic();
            getSolutionVector(x_I);

            deltaMax = 0;
            index = 0;
            for (int i =0; i < 3*h_nNodes; i++){
                x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
                if (x[i] > deltaMax){
                    deltaMax = MAX(deltaMax,x[i]);
                    index = i;
                }
            }
            //writeMessageToGUIWindow(" f = %.2f; Iter: %d, deltaMax = %e",w/(2*3.1415), iter,deltaMax);
            if ((fabs(deltaMaxPrev-deltaMax)/deltaMaxPrev < 1e-4)){// convergence
                success = true;
                deltaMaxOut[step] =deltaMax;
                phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
                break;
            }       
            deltaMaxPrev = deltaMax;

        }
        if (success)
             writeMessageToGUIWindow(" f = %.2f; Iter: %d, deltaMax = %e",w/(2*3.1415), iter,deltaMax);
        else{
            writeMessageToGUIWindow(" f = %.2f; Iter: %d, deltaMax = %e ***",w/(2*3.1415), iter,deltaMax);
            //omegaRadiansIn[step] = omegaRadiansIn[step] +1.0/(2*3.1415);
            //step = step -1;
            if (deltaMax <10) deltaMaxOut[step] =deltaMax;
        }
    }
    setForceVector(f0);// set back to static force

    free(x_0);
    free(x_R);
    free(x_I);

    free(Kdx_R);
    free(Kdx_I);
    free(Cdx_R);
    free(Cdx_I);

    free(f0);
    free(fx_R);
    free(fx_I);
    free(x);

    free(Ke);
    free(Me);
    free(Ke_D);

    return true;

}
/**********************************************************************/
int TransientElasticity::getNumFrequencyResponsePoints(void)
{
    return tr_nFrequencySteps;
}

/**********************************************************************/
int TransientElasticity::getFrequencyData(double *frequencyHz,double *deltaMax, double *phaseAngleDegrees)
{
    if (tr_nFrequencySteps == 0) return 0;

    for (int step = 0; step < tr_nFrequencySteps; step++){
        frequencyHz[step] = tr_omegaRadiansIn[step]/(2*PI);
        deltaMax[step] = tr_deltaMaxOut[step];
        phaseAngleDegrees[step] = tr_phaseAngleRadiansOut[step]*180/PI;
    }
    return tr_nFrequencySteps;
}
/**********************************************************************/
void TransientElasticity::KDynamicX(double *x, double w,double alpha, double beta, double *y)
{

    // Matrix :
    // [K-w*w*M             -alpha*w*M-beta*w*K;
    //  alpha*w*M+beta*w*K      K-w*w*M]

    // x and y are of size (6*h_nNodes);

    double *x_R, *x_I, *Mx_R,*Kx_R,*Mx_I,*Kx_I;
    x_R = (double *)malloc(3*h_nNodes*sizeof(double));
    x_I = (double *)malloc(3*h_nNodes*sizeof(double));

    for (int i = 0; i < 3*h_nNodes; i++){
        x_R[i] = x[i];
        x_I[i] = x[i+3*h_nNodes];
    }
    Mx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Mx_I = (double *)malloc(3*h_nNodes*sizeof(double));
    Kx_R = (double *)malloc(3*h_nNodes*sizeof(double));
    Kx_I = (double *)malloc(3*h_nNodes*sizeof(double));

    KuCPUGPU(x_R,Kx_R);
    KuCPUGPU(x_I,Kx_I);
    MuCPUGPU(x_R,Mx_R);
    MuCPUGPU(x_I,Mx_I);


    for (int i = 0; i < 3*h_nNodes; i++){
        y[i] =  Kx_R[i] - w*w*Mx_R[i] - alpha*w*Mx_I[i] -beta*w*Kx_I[i]; // (K*x_R-w*w*M*x_R) -alpha*w*M*x_I-beta*w*K*x_I;
        y[i+3*h_nNodes] = alpha*w*Mx_R[i]+beta*w*Kx_R[i]+Kx_I[i]-w*w*Mx_I[i];//alpha*w*Mx_R+beta*w*Kx_R      Kx_I-w*w*Mx_I]
    }

    free(Mx_R);
    free(Mx_I);
    free(Kx_R);
    free(Kx_I);
    free(x_R);
    free(x_I);
}
/**********************************************************************/
bool TransientElasticity::frequencyResponseSimpleCG(int nFrequencySteps,double *omegaRadiansIn,double *deltaMaxOut,
                                                    double *phaseAngleRadiansOut,bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // Plain CG code
    // Matrix A:
    // [K-w*w*M             -alpha*w*M-beta*w*K;
    //  alpha*w*M+beta*w*K      K-w*w*M]



    double fractionCompleted,tempDouble;
    Timer tempTimer;
    double rho,alpha,beta,rho_new,normr,normr0,w, deltaMax;
    double *r,*p,*a,*sol,*rhs, *x_R, *x_I, *x;
    int niter, index;
    int nDOF;
    bool success = true;

    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Performing CG on CPU");

    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Allocating temporary memory");
    if (tr_verbose) {
        if ((h_nodeBasedAssembly) && (h_voxelgrid))
            writeMessageToParetoLog("TransientElasticity: Node-based KU\n");
        else
            writeMessageToParetoLog("TransientElasticity: Elem-based KU\n");
    }
    // use temporary CPU variables
    nDOF = 6*h_nNodes;
    r = (double *) malloc(nDOF*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for r ");

    p = (double *) malloc(nDOF*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for p ");

    a = (double *) malloc(nDOF*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for a ");

    sol = (double *) malloc(nDOF*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for sol ");

    x_R = (double *) malloc(3*h_nNodes*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for x_R ");

    x_I = (double *) malloc(3*h_nNodes*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for x_I ");

    x = (double *) malloc(3*h_nNodes*sizeof(double));
    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for x ");

    w = 100*2*3.1415;
    for (int step = 0; step < nFrequencySteps; step++){
        tempTimer.start();
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Timer starts for step %d", step);

        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: frequency set ");

        rhs = (double *) malloc(nDOF*sizeof(double));
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Memory allocated for rhs ");


        memset(rhs,0,nDOF*sizeof(double));
        memcpy(rhs,h_f,3*h_nNodes*sizeof(double));// only copy to first half

        memset(x_R,0,3*h_nNodes*sizeof(double));
        memset(x_I,0,3*h_nNodes*sizeof(double));
        memset(x,0,3*h_nNodes*sizeof(double));

        for (int i=0; i< nDOF; i++){
             r[i]=0; p[i]=0; a[i]=0; sol[i]=0;
        }

        safeFree(h_cgNorm);
        h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
        memset(h_cgNorm,0,h_CGMaxIter*sizeof(double));
        // Assume u  has been initialized (see CPU initialization)


        tempTimer.start();
        KDynamicX(sol,w,tr_Damp_alpha,tr_Damp_beta,a);// a = K*sol
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Norm of sol = %e ",Norm(sol,nDOF));
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Time taken for one Kx: %f (msec) ",tempTimer.getElapsedTimeInMilliSec());
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: normf = %e, norm(sol) =%e ",Norm(rhs,nDOF),Norm(sol,nDOF));

        zeaxpy(-1,1,r,a,rhs,nDOF);// r = -a + f
        free(rhs);
        normr = sqrt(dotprod(r,r,nDOF));
        normr0 = normr;

        copy(r,a,nDOF);// Set a = r
        copy(a,p,nDOF);// Set p = a
        rho = dotprod(a,r,nDOF);
        niter = 1;     // Init counter for number of iterations
        if (tr_verbose) {
            if (h_nodeBasedAssembly) writeMessageToParetoLog("TransientElasticity: Starting node-based CG on CPU");
            else writeMessageToParetoLog("TransientElasticity: Starting elem-based CG on CPU");
            writeMessageToParetoLog("TransientElasticity: normr0 = %e, rho =%e \n",normr0,rho);
        }
        fractionCompleted = 0;
        success = true;
        while ((niter < 10) || (normr > h_CGRelativeResErr*normr0))  {//% Test break condition
            tempDouble = ((h_CGRelativeResErr*normr0)/normr);
            fractionCompleted = MAX(0,1+0.1*log(tempDouble));// no negative values

            h_cgNorm[niter-1] = log10(normr/normr0);
            KDynamicX(p,w,tr_Damp_alpha,tr_Damp_beta,a);// a = K*p;
            beta = dotprod(a,p,nDOF);
            alpha = rho/beta;
            enforceDirichlet0_frequencyResponse(sol);
            axpy(alpha,p,sol,nDOF);//u = u + alpha*p;
            enforceDirichlet0_frequencyResponse(sol);
            axpy(-alpha,a,r,nDOF);//r = r - alpha*a;
            normr = sqrt(dotprod(r,r,nDOF));
            if (niter == 1) normr0 = normr;//update for consistent scaling convergence
            copy(r,a,nDOF);// Set a = r
            rho_new =  dotprod(a,r,nDOF);//rho = a'*r
            zeaxpy(1,(rho_new/rho),p,a,p,nDOF);//p = a + rho_new/rho * p;
            rho = rho_new;
            (niter)++;
            h_nCGItersActual = niter;

           // writeMessageToParetoLog(" %d: %e ",niter,normr/normr0);
            if (normr >1e3*normr0)  {
                success = false;
                break;
            }
            if ((tr_verbose) && ((niter)%500 ==0)) writeMessageToParetoLog(" %d: %e ",niter,normr/normr0);
            if (niter == h_CGMaxIter)  {
                success = false;
                break;
            }
        } //CG ends for this step
        if (tr_verbose)  writeMessageToParetoLog("TransientElasticity: CG on CPU; nIter = %d, resErr = %e ",niter,normr/normr0);

        for (int dof = 0; dof < 3*h_nNodes; dof++){
            x_R[dof] = sol[dof];
            x_I[dof] = sol[3*h_nNodes+dof];
        }

        deltaMax = 0;
        for (int i =0; i < 3*h_nNodes; i++){
            x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
            if (x[i] > deltaMax){
                deltaMax = x[i];
                index = i;
            }
        }
        if (!success){
            writeMessageToParetoLog(" CG failed for step %d ",step);
            break;
        }
        phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
        deltaMaxOut[step] = deltaMax;
    }


    free(a);
    free(p);
    free(r);
    free(sol);
    free(x_R);
    free(x_I);
    free(x);
    return success;

}
/**********************************************************************/
bool TransientElasticity::frequencyResponseBiCGStab(int nFrequencySteps,double *omegaRadiansIn,double *deltaMaxOut,
                                                    double *phaseAngleRadiansOut,bool (*callBackFunction)(double fractionCompleted))
{
    // Find the frequency response of a dynamical system
    // The objective is to find the real and imaginary components of the displacement
    // when a harmonic load is applied

    // Plain CG code
    // Matrix A:
    // [K-w*w*M             -alpha*w*M-beta*w*K;
    //  alpha*w*M+beta*w*K      K-w*w*M]



    double fractionCompleted,tempDouble;
    Timer tempTimer;
    double rho,alpha,beta,rho_new,normr,normr0, w, deltaMax;
    double *r,*p,*a,*sol,*rhs, *x_R, *x_I, *x;
    int niter, index;
    int nDOF;
    bool success = true;

    //Additional variables for BiCGStab algorithm
    double *b,*s, *r0dash, *tempSol, omega, norms;

    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Performing CG on CPU");

    if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Allocating temporary memory");
    if (tr_verbose) {
        if ((h_nodeBasedAssembly) && (h_voxelgrid))
            writeMessageToParetoLog("TransientElasticity: Node-based KU\n");
        else
            writeMessageToParetoLog("TransientElasticity: Elem-based KU\n");
    }
    // use temporary CPU variables
    nDOF = 6*h_nNodes;
    r = (double *) malloc(nDOF*sizeof(double));
    p = (double *) malloc(nDOF*sizeof(double));
    a = (double *) malloc(nDOF*sizeof(double));
    sol = (double *) malloc(nDOF*sizeof(double));
    x_R = (double *) malloc(3*h_nNodes*sizeof(double));
    x_I = (double *) malloc(3*h_nNodes*sizeof(double));
    x = (double *) malloc(3*h_nNodes*sizeof(double));
    //For BiCGSTAB
    b = (double *) malloc(nDOF*sizeof(double));
    s = (double *) malloc(nDOF*sizeof(double));
    r0dash = (double *) malloc(nDOF*sizeof(double));
    tempSol = (double *) malloc(nDOF*sizeof(double));
    rhs = (double *) malloc(nDOF*sizeof(double));

    w = 100*2*3.1415;
    for (int step = 0; step < nFrequencySteps; step++){
        tempTimer.start();
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Timer starts for step %d", step);

        if (!callBackFunction(1.0*step/nFrequencySteps))
            break;
        w = omegaRadiansIn[step];


        memset(rhs,0,nDOF*sizeof(double));
        memcpy(rhs,h_f,3*h_nNodes*sizeof(double));// only copy to first half

        memset(x_R,0,3*h_nNodes*sizeof(double));
        memset(x_I,0,3*h_nNodes*sizeof(double));
        memset(x,0,3*h_nNodes*sizeof(double));

        memset(r,0,nDOF*sizeof(double));
        memset(p,0,nDOF*sizeof(double));
        memset(a,0,nDOF*sizeof(double));
        memset(sol,0,nDOF*sizeof(double));
        memset(b,0,nDOF*sizeof(double));
        memset(s,0,nDOF*sizeof(double));
        memset(r0dash,0,nDOF*sizeof(double));
        memset(tempSol,0,nDOF*sizeof(double));

        safeFree(h_cgNorm);
        h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
        memset(h_cgNorm,0,h_CGMaxIter*sizeof(double));
        // Assume u  has been initialized (see CPU initialization)

        tempTimer.start();
        KDynamicX(sol,w,tr_Damp_alpha,tr_Damp_beta,a);// a = K*sol
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Norm of sol = %e ",Norm(sol,nDOF));
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: Time taken for one Kx: %f (msec) ",tempTimer.getElapsedTimeInMilliSec());
        if (tr_verbose) writeMessageToParetoLog("TransientElasticity: normf = %e, norm(sol) =%e ",Norm(rhs,nDOF),Norm(sol,nDOF));

        zeaxpy(-1,1,r,a,rhs,nDOF);// r = -a + f

        normr = sqrt(dotprod(r,r,nDOF));
        normr0 = normr;
        copy(r,r0dash,nDOF);// Set r0dash = r //For BiCGSTAB
        copy(r,a,nDOF);// Set a = r
        copy(a,p,nDOF);// Set p = a
        rho = dotprod(r0dash,r,nDOF); //For BiCGSTAB
        niter = 1;     // Init counter for number of iterations
        if (tr_verbose) {
            if (h_nodeBasedAssembly) writeMessageToParetoLog("TransientElasticity: Starting node-based CG on CPU");
            else writeMessageToParetoLog("TransientElasticity: Starting elem-based CG on CPU");
            writeMessageToParetoLog("TransientElasticity: normr0 = %e, rho =%e \n",normr0,rho);
        }
        fractionCompleted = 0;
        success = true;
        while ((niter < 10) || (normr > h_CGRelativeResErr*normr0))  {//% Test break condition
            tempDouble = ((h_CGRelativeResErr*normr0)/normr);
            fractionCompleted = MAX(0,1+0.1*log(tempDouble));// no negative values

            h_cgNorm[niter-1] = log10(normr/normr0);
            KDynamicX(p,w,tr_Damp_alpha,tr_Damp_beta,a);// a = K*p;
            //beta = dotprod(a,r0dash,nDOF);//r0dash instead of p....For BiCGSTAB
            alpha = rho/dotprod(a,r0dash,nDOF);

            zeaxpy(1,-alpha,s,r,a,nDOF);//s = r - alpha * a; For BiCGSTAB
            norms = sqrt(dotprod(s,s,nDOF));
            /* BiCGSTAB with restart
             if (norms <1e-6){
                axpy(alpha,p,sol,nDOF);//u = u + alpha*p;
                break;
            }*/
            KDynamicX(s,w,tr_Damp_alpha,tr_Damp_beta,b);// b = K*s;
            omega = (dotprod(b,s,nDOF))/(dotprod(b,b,nDOF));//omega = (b.s)/(b.b); For BiCGSTAB
            enforceDirichlet0_frequencyResponse(tempSol);
            zeaxpy(1,alpha,tempSol,sol,p,nDOF);//tempSol = sol + alpha*p; //sol replaced by tempSol For BiCGSTAB
            enforceDirichlet0_frequencyResponse(tempSol);
            zeaxpy(1,omega,sol,tempSol,s,nDOF);//sol = tempSol +omega*s; For BiCGSTAB
            enforceDirichlet0_frequencyResponse(sol);
            zeaxpy(1,-omega,r,s,b,nDOF);//r = s - omega*b; For BiCGSTAB
            normr = sqrt(dotprod(r,r,nDOF));
            if (niter == 1) normr0 = normr;//update for consistent scaling convergence
            zeaxpy(1,-omega,a,p,a,nDOF);//a = p- omega*a; For BiCGSTAB
            rho_new =  dotprod(r, r0dash,nDOF);//rho_new = r'*r0dash
            beta= alpha*rho_new/(omega*rho);
            zeaxpy(1,beta,p,r,a,nDOF);//p = r + (alpha/omega)*(rho_new/rho) * a;
            if (abs(dotprod(r,r0dash,nDOF)) <1e-6){
                    copy(r,r0dash,nDOF);// Set r0dash = r
                    copy(r,p,nDOF);// Set p = r
            }
            rho = rho_new;
            (niter)++;
            h_nCGItersActual = niter;

            writeMessageToParetoLog(" %d: %e ",niter,normr/normr0);
            if (normr >1e6*normr0)  {
                success = false;
                break;
            }
            if ((tr_verbose) && ((niter)%500 ==0)) writeMessageToParetoLog(" %d: %e ",niter,normr/normr0);
            if (niter == h_CGMaxIter)  {
                success = false;
                break;
            }
        } //CG ends for this step
        if (tr_verbose)  writeMessageToParetoLog("TransientElasticity: CG on CPU; nIter = %d, resErr = %e ",niter,normr/normr0);

        for (int dof = 0; dof < 3*h_nNodes; dof++){
            x_R[dof] = sol[dof];
            x_I[dof] = sol[3*h_nNodes+dof];
        }

        deltaMax = 0;
        for (int i =0; i < 3*h_nNodes; i++){
            x[i] = sqrt(x_R[i]*x_R[i] + x_I[i]*x_I[i]);
            if (x[i] > deltaMax){
                deltaMax = x[i];
                index = i;
            }
        }
        if (!success){
            writeMessageToParetoLog(" CG failed for step %d ",step);
            break;
        }
        phaseAngleRadiansOut[step] = atan2(x_I[index], x_R[index]);
        deltaMaxOut[step] = deltaMax;
    }


    free(a);
    free(p);
    free(r);
    free(sol);
    free(x_R);
    free(x_I);
    free(x);
    safeFree(rhs);

    free(b);
    free(tempSol);
    free(s);
    free(r0dash);
    return success;

}
/**********************************************************************/
void TransientElasticity::enforceDirichlet0_frequencyResponse(double *x)
{
    int i,freeEncoded,wfree,ufree,vfree;

//#pragma omp parallel for private(i,freeEncoded,wfree,ufree,vfree)
    for (i =0; i < h_nNodes; i++) {
        if (!doesNodeExist(i)) continue;
        freeEncoded = h_free[i];
        wfree = freeEncoded/4;// 0 (fixed) or 1 (free)
        vfree = (freeEncoded - wfree*4)/2;// 0 or 1
        ufree = (freeEncoded - wfree*4 -vfree*2);// 0 or 1
        x[3*i] *= ufree;
        x[3*i+1] *= vfree;
        x[3*i+2] *= wfree;
        x[3*i + 3*h_nNodes] *= ufree;
        x[3*i+1 + 3*h_nNodes] *= vfree;
        x[3*i+2 + 3*h_nNodes] *= wfree;
    }
}
/**********************************************************************/
void TransientElasticity::findKEeffTemplate(void)
{
	int i,j,k,templateElem;
	double *tr_KeTemp,*tr_MeTemp;


	tr_nTemplateElements = getNumTemplateElements();
	safeFree(tr_h_KEeffTemplates);
	tr_h_KEeffTemplates =	(double *)malloc((tr_nTemplateElements)*24*24*sizeof(double));
	tr_KeTemp =	(double *)malloc((tr_nTemplateElements)*24*24*sizeof(double));
	tr_MeTemp =	(double *)malloc((tr_nTemplateElements)*24*24*sizeof(double));

	getKeTemplates(tr_KeTemp);
	getMeTemplates(tr_MeTemp);
	for (templateElem = 0; templateElem < tr_nTemplateElements;templateElem++) {
		k =0;
		for (i =0; i < 24; i++) {
			for( j = 0; j < 24;j++) {
				tr_h_KEeffTemplates[k*(tr_nTemplateElements) + templateElem] =  tr_K_coef*tr_KeTemp[k*(tr_nTemplateElements) + templateElem] + tr_M_coef*tr_MeTemp[k*(tr_nTemplateElements) + templateElem];
				k++;
			}
		}
	}
	free(tr_KeTemp);
	free(tr_MeTemp);
}
/**********************************************************************/
void TransientElasticity::findDampingCoefficients(double *d_M, double *d_K)
{
    writeMessageToParetoLog("-------------------------------------------------\n");
    writeMessageToParetoLog("Finding the damping coefficients \n");
	// find 2 highest eigenvalues
	int tr_NumModes = 1;
	double **tr_X,*tr_Damp_omega;
	allocMatrix(tr_NumModes,3*h_nNodes,&tr_X);
	allocVector(tr_NumModes,&tr_Damp_omega); //K*x = omega*M*x

	for (int i = 0; i < tr_NumModes;i++) 
		initializeRandomVector(tr_X[i]);

	solveElasticityModal(tr_NumModes,tr_Damp_omega,tr_X,0.05);// low accuracy is sufficient
	double tr_w1 = sqrt(tr_Damp_omega[0]);
	double tr_w2 = 500*tr_w1;
	double tr_xi1 = 1;
	double tr_xi2 = 1;
	*d_M  = 2*tr_w1*tr_w2*(tr_xi1*tr_w2-tr_xi2*tr_w1)/(pow(tr_w2,2.)-pow(tr_w1,2.));
	*d_K  = 2*(tr_xi2*tr_w2-tr_xi1*tr_w1)/(pow(tr_w2,2.)-pow(tr_w1,2.));

	*d_M *=0.15;
    writeMessageToParetoLog("Damp_alpha (M) = %e, Damp_beta (K) = %e \n",*d_M, *d_K);

	free(tr_Damp_omega);
	freeMatrix(tr_NumModes,tr_X);
    writeMessageToParetoLog("-------------------------------------------------\n");
}
/**********************************************************************/
void TransientElasticity::computeTransientCoefficients(void)
{
	tr_a0 = 1./(tr_N_beta*tr_h*tr_h);
	tr_a1 = tr_N_gamma/(tr_N_beta*tr_h);
	tr_a2 = 1./(tr_N_beta*tr_h);
	tr_a3 = 1./(2*tr_N_beta)-1;
	tr_a4 = tr_N_gamma/tr_N_beta-1;
	tr_a5= (tr_h/2.)*(tr_N_gamma/tr_N_beta-2);
	tr_a6 = tr_h*(1-tr_N_gamma);
	tr_a7 = tr_h*tr_N_gamma;
	// Keff
	tr_M_coef = tr_a0+tr_a1*tr_Damp_alpha;
	tr_K_coef = 1+tr_a1*tr_Damp_beta;
	// Feff
	tr_MDcoef = tr_a0+tr_a1*tr_Damp_alpha;
	tr_MDdotcoef = tr_a2+tr_a4*tr_Damp_alpha;
	tr_MDdotdotcoef = tr_a3+tr_Damp_alpha*tr_a5;
	tr_KDcoef = tr_a1*tr_Damp_beta;
	tr_KDdotcoef = tr_a4*tr_Damp_beta;
	tr_KDdotdotcoef = tr_a5*tr_Damp_beta;
}
/**********************************************************************/
double TransientElasticity::normDisplacementChange(void)
{
	double temp =0,temp2 = 0;;
	for (int i=0;i<tr_NumDOF;i++){
		temp += pow(tr_SolCurrent[i]-tr_SolOld[i],2.0);
		temp2 += pow(tr_SolCurrent[i],2.0);
	}
	return (sqrt(temp/temp2));
}
/**********************************************************************/
void TransientElasticity::updateSol(void)
{
	memcpy(tr_SolOld,tr_SolCurrent,tr_NumDOF*sizeof(double));
	memcpy(tr_SolDotOld,tr_SolDot,tr_NumDOF*sizeof(double));
	memcpy(tr_SolDotDotOld,tr_SolDotDot,tr_NumDOF*sizeof(double));

    //if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: Solution updated\n");
}
/**********************************************************************/
void TransientElasticity::findVelocityAcceleration(void)
{
	int node;
	int i1,i2,i3;
	int ufree,vfree,wfree;
#pragma omp parallel for private(i1,i2,i3,ufree,vfree,wfree)  
	for (node=0;node<h_nNodes;node++) {
		wfree = h_free[node]/4;
		vfree = (h_free[node] - wfree*4)/2;
		ufree = (h_free[node] - wfree*4 -vfree*2);
		i1 = 3*node;
		i2 = 3*node + 1;
		i3 = 3*node + 2;
		if (ufree){
			tr_SolDotDot[i1] = tr_a0*(tr_SolCurrent[i1]-tr_SolOld[i1])-tr_a2*tr_SolDot[i1]-tr_a3*tr_SolDotDotOld[i1];
			tr_SolDot[i1] = tr_SolDotOld[i1]+tr_a6*tr_SolDotDotOld[i1]+tr_a7*tr_SolDotDot[i1];
		}
		if(vfree){
			tr_SolDotDot[i2] = tr_a0*(tr_SolCurrent[i2]-tr_SolOld[i2])-tr_a2*tr_SolDot[i2]-tr_a3*tr_SolDotDotOld[i2];
			tr_SolDot[i2] = tr_SolDotOld[i2]+tr_a6*tr_SolDotDotOld[i2]+tr_a7*tr_SolDotDot[i2];
		}
		if(wfree){
			tr_SolDotDot[i3] = tr_a0*(tr_SolCurrent[i3]-tr_SolOld[i3])-tr_a2*tr_SolDot[i3]-tr_a3*tr_SolDotDotOld[i3];
			tr_SolDot[i3] = tr_SolDotOld[i3]+tr_a6*tr_SolDotDotOld[i3]+tr_a7*tr_SolDotDot[i3];
		}
	}
}
/**********************************************************************/
double TransientElasticity::computeKineticEnergy(void)
{
	double KE;
	double *temp = (double *)malloc(tr_NumDOF*sizeof(double));
	MuCPUGPU(tr_SolDot,temp);
	KE = 0.5*dotprod(tr_SolDot,temp);
	free(temp);
	return KE;
}
/**********************************************************************/
void TransientElasticity::computeEffectiveF(void)
{
	MuCPUGPU(tr_SolOld,tr_MD);
	MuCPUGPU(tr_SolDotOld,tr_MDdot);
	MuCPUGPU(tr_SolDotDotOld,tr_MDdotdot);
	KuCPUGPU(tr_SolOld,tr_KD);
	KuCPUGPU(tr_SolDotOld,tr_KDdot);
	KuCPUGPU(tr_SolDotDotOld,tr_KDdotdot);
	int node;
	int i1,i2,i3;
	int ufree,vfree,wfree;
	memset(tr_Feff,0,tr_NumDOF*sizeof(double));
#pragma omp parallel for private(i1,i2,i3,ufree,vfree,wfree)
	for (node=0;node<h_nNodes;node++){
		wfree = h_free[node]/4;
		vfree = (h_free[node] - wfree*4)/2;
		ufree = (h_free[node] - wfree*4 -vfree*2);
		i1 = 3*node;
		i2 = 3*node + 1;
		i3 = 3*node + 2;
		if (ufree){
			tr_MD[i1] *= tr_MDcoef;
			tr_MDdot[i1] *= tr_MDdotcoef;
			tr_MDdotdot[i1] *= tr_MDdotdotcoef;
			tr_KD[i1] *= tr_KDcoef;
			tr_KDdot[i1] *= tr_KDdotcoef;
			tr_KDdotdot[i1] *= tr_KDdotdotcoef;
			tr_Feff[i1] = tr_staticForce[i1]+tr_MD[i1]+tr_MDdot[i1]+tr_MDdotdot[i1]+tr_KD[i1]+tr_KDdot[i1]+tr_KDdotdot[i1]+tr_Weight[i1];
		}
		if (vfree){
			tr_MD[i2] *= tr_MDcoef;
			tr_MDdot[i2] *= tr_MDdotcoef;
			tr_MDdotdot[i2] *= tr_MDdotdotcoef;
			tr_KD[i2] *= tr_KDcoef;
			tr_KDdot[i2] *= tr_KDdotcoef;
			tr_KDdotdot[i2] *= tr_KDdotdotcoef;
			tr_Feff[i2] = tr_staticForce[i2]+tr_MD[i2]+tr_MDdot[i2]+tr_MDdotdot[i2]+tr_KD[i2]+tr_KDdot[i2]+tr_KDdotdot[i2]+tr_Weight[i2];
		}
		if (wfree){
			tr_MD[i3] *= tr_MDcoef;
			tr_MDdot[i3] *= tr_MDdotcoef;
			tr_MDdotdot[i3] *= tr_MDdotdotcoef;
			tr_KD[i3] *= tr_KDcoef;
			tr_KDdot[i3] *= tr_KDdotcoef;
			tr_KDdotdot[i3] *= tr_KDdotdotcoef;
			tr_Feff[i3] = tr_staticForce[i3]+tr_MD[i3]+tr_MDdot[i3]+tr_MDdotdot[i3]+tr_KD[i3]+tr_KDdot[i3]+tr_KDdotdot[i3]+tr_Weight[i3];
		}
	}
}
/**********************************************************************/
void TransientElasticity::findWeight(void)
{
	double *temp;
	setGravity();
	temp = (double *)malloc(3*h_nNodes*sizeof(double));
	for (int node=0;node<h_nNodes;node++){
		tr_Weight[3*node] = tr_gravity[0];
		tr_Weight[3*node+1] = tr_gravity[1];
		tr_Weight[3*node+2] = tr_gravity[2];
	}
	MuCPU(tr_Weight,temp);
	memcpy(tr_Weight,temp,3*h_nNodes*sizeof(double));
	free(temp);
}
/**********************************************************************	TransientElasticity On GPU	**********************************************************************/
#ifdef COMPILE_TRANS_GPUCODE

void TransientElasticity::NewmarkSolverOnGPU(transientData *gpuTr)
{
    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: NewmarkSolverOnGPU\n");
	double tr_diff = 1.0;
	int whichNode;
	transferForcesBCToGPU();
	while ((tr_diff > tr_tolerance)&&(tr_CurrentTime< tr_totalTime))
	{
        writeMessageToParetoLog("TimeIndex = %i; diff = %e \n",tr_TimeIndex,tr_diff);
		if (tr_TimeIndex == 0){
			allocateCPUforGPU();
			AllocateSolOnGPU(getNumTemplateElements(),gpuTr);
			initializeTransientForceVectorOnGPU(&h_gpuData,gpuTr);
			computeTransientCoefficients();
			findKEeffTemplate();
			setMKCoefficientsOnGPU(gpuTr,tr_a0,tr_a1,tr_a2,tr_a3,tr_a4,tr_a5,tr_a6,tr_a7,tr_MDcoef,tr_MDdotcoef,tr_MDdotdotcoef,tr_KDcoef,tr_KDdotcoef,tr_KDdotdotcoef);
			setSolutionAtAllNodes(tr_SolCurrent); // on CPU for plotting
			setSolOnGPU(&h_gpuData,gpuTr); // just in case SolCurrent to d_sol
			findVelocityAccelerationOnGPU(&h_gpuData,gpuTr);
			if (TR_GRAVITY){
				setGravityOnGPU(gpuTr,tr_gravity);
				findWeightOnGPU(&h_gpuData,gpuTr,tr_nodeBasedAssembly);
			}
			if (tr_Track) {
				setGPUTrackingOn(gpuTr,tr_Track_DOF);
				initializeTrackingOnGPU(gpuTr);
			}
		}
		UpdateSolOnGPU(gpuTr);
		updateTimeOnGPU(gpuTr,tr_TimeIndex,tr_CurrentTime);
		if (tr_Track) {
			tr_vonMises = getMaxVonMisesStress(&whichNode);
			updateTrackingOnGPU(gpuTr,tr_vonMises);
		}
		EffeciveFOnGPU(&h_gpuData,gpuTr,tr_nodeBasedAssembly);
		SetTransientFeffVectorOnGPU(&h_gpuData,gpuTr); 
		getKeTemplates(tr_h_KEdummy); // copy voxel K to temp K
		setKeTemplates(tr_h_KEeffTemplates);// copy voxel Keff to voxel K
		computeKeNodalFromKeTemplate();
		if (tr_TimeIndex == 0){
			ReleaseWorkSpace();
			InitializeDeflationForElasticity();
		}
		solveElasticityStatic();
		getSolOnGPU(&h_gpuData,gpuTr);
		findVelocityAccelerationOnGPU(&h_gpuData,gpuTr);
		postprocess();
		setKeTemplates(tr_h_KEdummy);// copy temp K to voxel K To find Stress
		computeKeNodalFromKeTemplate();
		tr_TimeIndex++;
		tr_CurrentTime += tr_h; 
		tr_diff = diffNormOnGPU(gpuTr);
		copySol_GPUtoCPU(gpuTr,tr_SolCurrent);
		setSolutionAtAllNodes(tr_SolCurrent);
        //setPlotRoutine(1);
	}
	if (tr_Track){
		NormalizeTrackedOnGPU(gpuTr);
		getTimeStepsTrackedOnGPU(gpuTr,tr_Track_time);
		getDisplacementTrackedOnGPU(gpuTr,tr_Track_displacement);
		getStressTrackedOnGPU(gpuTr,tr_Track_stress);
	}
	FreeGPU(gpuTr);
	return;
}
#endif
/**********************************************************************/
void TransientElasticity::allocateCPUforGPU()
{
	int nTemplateElements = getNumTemplateElements();
	initializeTracking();
	safeFree(tr_SolCurrent);
	safeFree(tr_h_KEdummy);

	tr_SolCurrent = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_h_KEdummy = (double *)malloc((nTemplateElements)*24*24*sizeof(double));

	memset(tr_SolCurrent,0,tr_NumDOF*sizeof(double));
	memset(tr_h_KEdummy,0,(nTemplateElements)*24*24*sizeof(double));
    writeMessageToParetoLog("AllocatedCPUforGPU\n");
}

/**********************************************************************	Drop Test Simulation On CPU	**********************************************************************/
void TransientElasticity::DropTestSimulation()
{
	// Assumptions:
	// - One Geometry and Ground
	// - No extrenal force or Dirichlet BC.
	// - Only surface nodes make contact
	// - initial acc = gravity

    if (TRANSIENT_ELASTICITY_VERBOSE) writeMessageToParetoLog("TransientElasticity: Drop Test Simulation\n");
	double tr_diff = 1.0;
	double KE;
	memset(tr_staticForce,0,tr_NumDOF*sizeof(double));

	KE = computeKineticEnergy();
	double delta = findTheoreticalIndentation(KE);
    writeMessageToParetoLog("After free fall, Kinetic Energy = %e, indentation = %e \n",KE,delta);
	while ((tr_CurrentTime< tr_totalTime))//((tr_diff > tr_tolerance)&&(tr_CurrentTime< tr_totalTime))
	{
		//getchar();
        //writeMessageToParetoLog("TimeIndex = %i; diff = %e \n",tr_TimeIndex,tr_diff);
        writeMessageToParetoLog("KE = %e \n",computeKineticEnergy());
		updateSol();
		if (tr_Track) updateTracking();
		computeEffectiveF();
		setForceVector(tr_Feff);
		setKeTemplates(tr_h_KEeffTemplates);// copy voxel Keff to voxel K
		computeKeNodalFromKeTemplate();
		if (tr_TimeIndex == 0){
			ReleaseWorkSpace();
			InitializeDeflationForElasticity();
		}
		solveElasticityStatic();
		postprocess();
		getSolutionVector(tr_SolCurrent);
		findVelocityAcceleration();
		if (DR_METHOD == DROP_FORCE){
			findContactNodes();
			findContactForces();
		}
		else {
			findDirichletReactionForces();
			findTemporaryDirichletNodes();
			applyTemporaryDirichletToDisplacement();
			applyTemporaryDirichletToOldVelocityAndAcceleration();
			applyTemporaryDirichletToCurrentVelocityAndAcceleration();
		}
		setKeTemplates(tr_h_KEdummy);// copy temp K to voxel K
		computeKeNodalFromKeTemplate();
		tr_TimeIndex++;
		tr_CurrentTime += tr_h; 
		tr_diff = normDisplacementChange();
		setSolutionAtAllNodes(tr_SolCurrent);
        //setPlotRoutine(1);
	}
	if (tr_Track)
		normalizeTracked();
	// reset the static force when done
	setForceVector(tr_staticForce);
}
/**********************************************************************/
void TransientElasticity::FreeFallSimulation(double ZPlaneLocation)
{
	double delta;
	findWeight();
	dr_Height = abs(ZPlaneLocation);
    writeMessageToParetoLog("dr_Height = %e\n",dr_Height);
	findZofFloor();
	delta = 0.001*dr_Height;

	dr_fallingZVelocity = -sqrt(fabs(2*tr_gravity[2]*(dr_Height-delta)));
	
	for (int i=0;i<h_nNodes;i++){
		if(dr_Height>tr_tolerance) {
			tr_SolDot[3*i+2] = dr_fallingZVelocity;
			tr_SolCurrent[3*i+2] -= (dr_Height-delta); 
		}
		tr_SolDotDot[3*i+2] = tr_gravity[2];
	}
	
	setSolutionAtAllNodes(tr_SolCurrent);
    //setPlotRoutine(1);
}
/**********************************************************************/
void TransientElasticity::initializeDropTest (void)
{
	DR_DROPTEST = true;
	DR_METHOD = DROP_FORCE;//DROP_FORCE , DROP_DIRICHLET;
	if ((h_voxelgrid) &&  (isSpMVMethodNodeBased()))
		computeKeAndMeNodal(1);
	findBoundaryNodes();
	allocateSol();
	initializeDropTestForceVector();
	getKeTemplates(tr_h_KEdummy);// copy voxel K to temp K
	computeTransientCoefficients();
	setSolutionAtAllNodes(tr_SolCurrent);
	findKEeffTemplate();
	if (DR_METHOD==DROP_FORCE)
		initializeDropTestForceBased();
	else
		initializeDropTestDirichletBased();

	if (tr_Track)
		initializeTracking();
}
/**********************************************************************/
void TransientElasticity::initializeDropTestForceBased()
{
	dr_ContactRadius = findVoxelContactRadius();
	dr_E = 2.1e11;
	dr_Nu = 0.28;

	dr_VoxelKprime = (4./3.)*dr_E/(1-dr_Nu*dr_Nu);

	safeFree(dr_ContactNodes);
	safeFree(dr_Indentation);

	dr_ContactNodes = (int *) malloc(dr_nBoundaryNodes*sizeof(int));
	dr_Indentation = (double *) malloc(dr_nBoundaryNodes*sizeof(double));

	memset(dr_ContactNodes,0,dr_nBoundaryNodes*sizeof(int));
	memset(dr_Indentation,0,dr_nBoundaryNodes*sizeof(double));
}
/**********************************************************************/
void TransientElasticity::initializeDropTestDirichletBased()
{
	safeFree(dr_tempFixed);
	dr_tempFixed = (int *) malloc(dr_nBoundaryNodes*sizeof(int));
	memset(dr_tempFixed,0,dr_nBoundaryNodes*sizeof(int));
}
/**********************************************************************/
void TransientElasticity::initializeDropTestForceVector()
{
	safeFree(tr_staticForce);
	safeFree(tr_Weight);
	safeFree(dr_Fcontact);

	tr_staticForce = (double *) malloc(tr_NumDOF*sizeof(double));
	tr_Weight = (double *) malloc(tr_NumDOF*sizeof(double));
	dr_Fcontact = (double *) malloc(tr_NumDOF*sizeof(double));

	memset(tr_staticForce,0,tr_NumDOF*sizeof(double));
	memset(tr_Weight,0,tr_NumDOF*sizeof(double));
	memset(dr_Fcontact,0,tr_NumDOF*sizeof(double));
}
/**********************************************************************/
void TransientElasticity::findZofFloor(void)
{
	// find closest node to ground OR Min(z)
	// Assuming and z(t=0)-zg>0

	double minZ = h_z[0];
	int closeNode = 0;
	for (int node=1;node<h_nNodes;node++){
		if (isNodeOnBoundary(node) && h_z[node]<minZ)
			minZ = h_z[node];
	}
	dr_zg = minZ-dr_Height;
}
/**********************************************************************/
void TransientElasticity::findBoundaryNodes(void)
{
	int *BoundaryNodes;
	BoundaryNodes = NULL;
	safeFree(BoundaryNodes);
	BoundaryNodes = (int *) malloc(h_nNodes*sizeof(int));
	memset(BoundaryNodes,0,h_nNodes*sizeof(int));
	dr_nBoundaryNodes = 0;
	for (int node=0;node<h_nNodes;node++){
		if(isNodeOnBoundary(node)){
			BoundaryNodes[dr_nBoundaryNodes] = node;
			dr_nBoundaryNodes++;
		}
	}
    writeMessageToParetoLog("dr_nBoundaryNodes = %i\n",dr_nBoundaryNodes);
	dr_BoundaryNodes = (int *) malloc(dr_nBoundaryNodes*sizeof(int));
	memset(dr_BoundaryNodes,0,dr_nBoundaryNodes*sizeof(int));
	for (int node=0;node<dr_nBoundaryNodes;node++) dr_BoundaryNodes[node] = BoundaryNodes[node];
	free(BoundaryNodes);
}
// Force-Based
/**********************************************************************/
double TransientElasticity::findTheoreticalIndentation(double KE)
{
	double Estar = dr_E/(1-dr_Nu*dr_Nu);
	double radius = 0.1;
	return pow(KE*3/(Estar*4*sqrt(radius)),2.0);
}
/**********************************************************************/
void TransientElasticity::findContactForces(void)
{
	memset(dr_Fcontact,0,tr_NumDOF*sizeof(double));
	double radius = 0.1;
	Sphere_PlaneContact(dr_Fcontact,dr_Indentation,dr_E,dr_Nu,radius,dr_ContactNodes,dr_nContactNodes);
	memcpy(tr_staticForce,dr_Fcontact,tr_NumDOF*sizeof(double));
}
/**********************************************************************/
double TransientElasticity::findVoxelContactRadius(void)
{
	double minDim = min(h_dz,min(h_dy,h_dx));
	double Rmin = minDim/2.;
	double Rmax = 0.5*sqrt(h_dx*h_dx + h_dy*h_dy + h_dz*h_dz);
	double Ravg = 0.5*(Rmin + Rmax);
	return Ravg;
}
/**********************************************************************/
void TransientElasticity::findContactNodes(void)
{
	dr_nContactNodes = 0;
	memset(dr_ContactNodes,0,dr_nBoundaryNodes*sizeof(int));
	memset(dr_Indentation,0,dr_nBoundaryNodes*sizeof(double));
	int node;
	double gap;
	double CurrentPosition;
	for (int i=0;i<dr_nBoundaryNodes;i++){
		node = dr_BoundaryNodes[i];
		CurrentPosition = h_z[node] + tr_SolCurrent[3*node+2]; 	
		gap = CurrentPosition - dr_zg;
		if ( gap < 0.0)
		{
			dr_ContactNodes[dr_nContactNodes] = node;
			dr_Indentation[dr_nContactNodes] = -gap; //-gap
			dr_nContactNodes++;
		}
	}
}
/**********************************************************************/
int TransientElasticity::findCenterOfContact(double *Indentation,int nNodes)
{
	int centerID = 0;
	double centerIndentation = Indentation[0];
	for (int i=1;i<nNodes;i++)
	{
		if (centerIndentation<Indentation[i]) {
			centerID = i;
			centerIndentation = Indentation[i];
		}
	}
	return centerID;
}
/**********************************************************************/
double TransientElasticity::findDistanceToCenterNode(int node,int center)
{
	double distance = sqrt(pow(h_x[center]-h_x[node],2.)+pow(h_y[center]-h_y[node],2.)+pow(h_z[center]-h_z[node],2.));
	return distance;
}
/**********************************************************************/
void TransientElasticity::Sphere_PlaneContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,int *Nodes,int nNodes)
{
	// Sphere half-space contact
	double d; // depth (max indentation) 
	double a; // contact area radius 
	int node;
	int centerID,center;
	double P0; // max. pressure
	double r; // relative distance to center
	double Estar = E/(1-Nu*Nu);
	double F;
	centerID = findCenterOfContact(Indentation,nNodes);
	center = Nodes[centerID];
	d = Indentation[centerID];
	F = (4./3.)*Estar*sqrt(radius*pow(d,3.));
	a = sqrt(2.*radius*d);
	P0 = 3.*F/(2.*PI*a*a);
	if (nNodes != 0) {
		for (int i=0;i<nNodes;i++)
		{
			node = Nodes[i];
			r = findDistanceToCenterNode(node,center);
			Fcontact[3*node+2] = F/nNodes;  
		}
	}
}
/**********************************************************************/
void TransientElasticity::Sphere_SphereContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,int *Nodes,int nNodes)
{
	// Both Spheres have equal radii

	double d; // depth (max indentation) 
	double a; // contact area radius 
	int node;
	int centerID,center;
	double P0; // max. pressure
	double r; // relative distance to center
	double Reff = radius/2.;
	double Estar = E/(1-Nu*Nu);
	double F;
	centerID = findCenterOfContact(Indentation,nNodes);
	center = Nodes[centerID];
	d = Indentation[centerID];
	a = sqrt(Reff*d);
	F = (4./3.)*Estar*sqrt(pow(a,3))/Reff;
	P0 = 3.*F/(2.*PI*a*a);
	for (int i=0;i<nNodes;i++)
	{
		node = Nodes[i];
		r = findDistanceToCenterNode(node,center);
		// ?
	}
}
/**********************************************************************/
void TransientElasticity::Cylinder_CylinderContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,double length,int *Nodes,int nNodes)
{
	// Both Cylinders have equal radii and length

	double d; // depth (max indentation) 
	double a; // contact area radius 
	int node;
	double P0; // max. pressure
	double Estar = E/(1-Nu*Nu);
	double F;
	double Reff = radius/2.;
	d = Indentation[0];
	F = PI/4.*Estar*length*d;
	a = sqrt(Reff*d);
	P0 = sqrt(Estar*F/(PI*length*Reff));
	for (int i=0;i<nNodes;i++)
	{
		node = Nodes[i];
		Fcontact[3*node+2] = P0; 
	}
}
/**********************************************************************/
// Drop Test Dirichlet-based
void TransientElasticity::findDirichletReactionForces(void)
{
	memset(dr_Fcontact,0,tr_NumDOF*sizeof(double));
	setForceVector(tr_Feff);
	setSolutionAtAllNodes(tr_SolCurrent);
	getReactionForces(dr_Fcontact);
}
/**********************************************************************/
void TransientElasticity::findTemporaryDirichletNodes(void)
{
	resetBC();
	dr_nTempFixed = 0;
	dr_nNewFixed = 0;
	dr_nOldFixed = 0;
	dr_nStaticFixed = 0;
	memset(dr_tempFixed,0,dr_nBoundaryNodes*sizeof(int));
	double CurrentPosition;
	double gap;
	bool NewNodeCond,OldNodeCond,StaticNodeCond;
	int node;
	for (int i=0;i<dr_nBoundaryNodes;i++){
		node = dr_BoundaryNodes[i];
		NewNodeCond = (fabs(dr_Fcontact[3*node+2])<1e-6 && tr_SolDot[3*node+2]<tr_tolerance);
		OldNodeCond = (dr_Fcontact[3*node+2]>-1e-6 && fabs(tr_SolDot[3*node+2])<1e-6);
		StaticNodeCond = (fabs(tr_SolDot[3*node+2])<1e-6 && fabs(dr_Fcontact[3*node+2])<1e-6);
		CurrentPosition = h_z[node] + tr_SolCurrent[3*node+2]; 	
		gap = CurrentPosition - dr_zg;
		if (gap<tr_tolerance && (NewNodeCond || OldNodeCond || StaticNodeCond))
		{
			dr_tempFixed[dr_nTempFixed] = node;
			dr_nTempFixed++;
			if (NewNodeCond) dr_nNewFixed++;
			if (OldNodeCond) dr_nOldFixed++;
			if (StaticNodeCond) dr_nStaticFixed++;
		}
	}
}
/**********************************************************************/
void TransientElasticity::applyTemporaryDirichletToDisplacement(void)
{
	int BCType[3];
	double BCValue[3];
	BCType[0] = ELASTICITY_NEUMANN;
	BCType[1] = ELASTICITY_NEUMANN;
	BCType[2] = ELASTICITY_DIRICHLET;
	int node;
	double CurrentPosition;
	double gap;
	for (int i=0;i<dr_nTempFixed;i++){
		node = dr_tempFixed[i];
		CurrentPosition = h_z[node] + tr_SolCurrent[3*node+2]; 	
		gap = CurrentPosition - dr_zg;
		BCValue[0] = 0.;
		BCValue[1] = 0.;
		BCValue[2] = -gap;
		setBCOnSingleNode(node,BCType,BCValue);
	}
}
/**********************************************************************/
void TransientElasticity::applyTemporaryDirichletToCurrentVelocityAndAcceleration(void)
{
	int node;
	for (int i=0;i<dr_nTempFixed;i++){
		node = dr_tempFixed[i];
		tr_SolDot[3*node+2] = 0.0;
		tr_SolDotDot[3*node+2] = 0.0;
	}
}
/**********************************************************************/
void TransientElasticity::applyTemporaryDirichletToOldVelocityAndAcceleration(void)
{
	int node;
	for (int i=0;i<dr_nTempFixed;i++){
		node = dr_tempFixed[i];
		tr_SolDotOld[3*node+2] = 0.0;
		tr_SolDotDotOld[3*node+2] = 0.0;
	}
}
/**********************************************************************	LDC **********************************************************************/
double TransientElasticity::getTransientCurrentTime()
{
	return tr_CurrentTime;
}
/**********************************************************************/
void TransientElasticity::setStaticForceVector()
{
	setForceVector(tr_staticForce);
}
/**********************************************************************	Tracking **********************************************************************/
void TransientElasticity::setTrackingOn(double *point,int dir)
{
	// dir: x=0,y=1,z=3
	tr_Track = true;
	tr_TrackminDist = 1e8;
    tr_ClosestNode =  getClosestNode(point[0],point[1],point[2]);
	tr_Track_DOF = 3*tr_ClosestNode+dir;
    writeMessageToParetoLog("Closese Node = %i\n",tr_ClosestNode);
    writeMessageToParetoLog("DOF = %i\n",tr_Track_DOF);
    writeMessageToParetoLog("Min. Distance = %e\n",tr_TrackminDist);
}
/**********************************************************************/
void TransientElasticity::setTrackingOff()
{
	tr_Track = false;
}
/**********************************************************************/
void TransientElasticity::initializeTracking()
{

	safeFree(tr_Track_time);
	safeFree(tr_Track_force);
	safeFree(tr_Track_displacement);
	safeFree(tr_Track_velocity);
	safeFree(tr_Track_acceleration);
	safeFree(tr_Track_stress);


	tr_Track_time = (double *) malloc(MAX_TIME_STEPS*sizeof(double));
	tr_Track_force = (double *) malloc(MAX_TIME_STEPS*sizeof(double));
	tr_Track_displacement = (double *) malloc(MAX_TIME_STEPS*sizeof(double));
	tr_Track_velocity = (double *) malloc(MAX_TIME_STEPS*sizeof(double));
	tr_Track_acceleration = (double *) malloc(MAX_TIME_STEPS*sizeof(double));
	tr_Track_stress = (double *) malloc(MAX_TIME_STEPS*sizeof(double));

	memset(tr_Track_time,0,MAX_TIME_STEPS*sizeof(double));
	memset(tr_Track_force,0,MAX_TIME_STEPS*sizeof(double));
	memset(tr_Track_displacement,0,MAX_TIME_STEPS*sizeof(double));
	memset(tr_Track_velocity,0,MAX_TIME_STEPS*sizeof(double));
	memset(tr_Track_acceleration,0,MAX_TIME_STEPS*sizeof(double));
	memset(tr_Track_stress,0,MAX_TIME_STEPS*sizeof(double));
}
/**********************************************************************/
void TransientElasticity::updateTracking()
{
	int whichNode;
	tr_Track_time[tr_TimeIndex] = tr_CurrentTime;
	tr_Track_displacement[tr_TimeIndex] = tr_SolCurrent[tr_Track_DOF];
	tr_Track_velocity[tr_TimeIndex] = tr_SolDot[tr_Track_DOF];
	tr_Track_acceleration[tr_TimeIndex] = tr_SolDotDot[tr_Track_DOF];
	tr_Track_stress[tr_TimeIndex] = getMaxVonMisesStress(&whichNode); 
}
/**********************************************************************/
void TransientElasticity::normalizeTracked()
{
	double tr_max_displacement = 0.0;
	double tr_max_velocity = 0.0;
	double tr_max_acceleration = 0.0;
	double tr_max_stress = 0.0;
	double tr_max_time = 0.0;

	findMaxTrackedValue(&tr_max_time,tr_Track_time);
	findMaxTrackedValue(&tr_max_displacement,tr_Track_displacement);
	findMaxTrackedValue(&tr_max_velocity,tr_Track_velocity);
	findMaxTrackedValue(&tr_max_acceleration,tr_Track_acceleration);
	findMaxTrackedValue(&tr_max_stress,tr_Track_stress);

	for (int i = 0; i < tr_TimeIndex;i++){
		tr_Track_time[i] /= tr_max_time;
		tr_Track_displacement[i] /= tr_max_displacement;
		tr_Track_velocity[i] /= tr_max_velocity;
		tr_Track_acceleration[i] /= tr_max_acceleration;
		tr_Track_stress[i] /= tr_max_stress;
	}
}
/**********************************************************************/
void TransientElasticity::findMaxTrackedValue(double *maxval,double *TrackedVector)
{
	for (int i = 0; i < tr_TimeIndex;i++){
		if (fabs(TrackedVector[i])>fabs(*maxval)){
			*maxval = TrackedVector[i];
		}
	}
}
/**********************************************************************/
int TransientElasticity::getNumberOfTimeStepsTracked()
{
	return tr_TimeIndex;
}
/**********************************************************************/
void TransientElasticity::getTimeStepsTracked(int N,double *time)
{
	N = min(N,tr_TimeIndex);
	for (int i = 0; i < N;i++)
		time[i] = tr_Track_time[i];
}
/**********************************************************************/
void TransientElasticity::getDisplacementTracked(int N,double *disp)
{
	N = min(N,tr_TimeIndex);
	for (int i = 0; i < N;i++)
		disp[i] = tr_Track_displacement[i];
}
/**********************************************************************/
void TransientElasticity::getStressTracked(int N,double *stress)
{
	N = min(N,tr_TimeIndex);
	for (int i = 0; i < N;i++)
		stress[i] = tr_Track_stress[i];
}
/**********************************************************************/
TransientElasticity::~TransientElasticity()
{
	safeFree(tr_SolOld); 
	safeFree(tr_SolDotOld); 
	safeFree(tr_SolDotDotOld);
	safeFree(tr_SolCurrent); 
	safeFree(tr_SolDot);
	safeFree(tr_SolDotDot);
	safeFree(tr_MD); 
	safeFree(tr_MDdot);
	safeFree(tr_MDdotdot);
	safeFree(tr_KD); 
	safeFree(tr_KDdot);
	safeFree(tr_KDdotdot);
	safeFree(tr_Feff);
	safeFree(tr_h_KEeffTemplates);
	safeFree(tr_h_MEeffTemplates);

	safeFree(tr_Track_time);
	safeFree(tr_Track_force);
	safeFree(tr_Track_displacement);
	safeFree(tr_Track_velocity);
	safeFree(tr_Track_acceleration);
	safeFree(tr_Track_stress);

	safeFree(dr_Fcontact);
	safeFree(tr_Weight);
}
