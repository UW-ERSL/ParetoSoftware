#include <stdio.h>
#include <math.h>
#include <cuda.h>
#include "cublas.h"
#include <string.h>
#include <stdlib.h>
#include "TransientElasticityGPU.h"

#ifdef COMPILE_GPUCODE
/**********************************************************************/
int InitializeTransElasticityOnGPU(int h_nNodes,transientData *gpuTr,int tr_Track_DOF)
{
	gpuTr->dtr_a1 = 0;
	gpuTr->dtr_a2 = 0;
	gpuTr->dtr_a3 = 0;
	gpuTr->dtr_a4 = 0;
	gpuTr->dtr_a5 = 0;
	gpuTr->dtr_a6 = 0;
	gpuTr->dtr_a7 = 0;
	gpuTr->dtr_MDcoef = 0;
	gpuTr->dtr_MDdotcoef = 0;
	gpuTr->dtr_MDdotdotcoef = 0;
	gpuTr->dtr_KDcoef = 0;
	gpuTr->dtr_KDdotcoef = 0;
	gpuTr->dtr_KDdotdotcoef = 0;
	gpuTr->dtr_N_gamma = 0;
	gpuTr->dtr_N_beta = 0;
	gpuTr->dtr_Damp_alpha = 0;
	gpuTr->dtr_Damp_beta = 0;
	gpuTr->dtr_h = 0;
	gpuTr->dtr_CurrentTime = 0;
	gpuTr->dtr_TimeIndex = 0;
	gpuTr->dtr_tolerance = 0;
	gpuTr->dtr_NumDOF = 0;
	gpuTr->dtr_SolOld = NULL; 
	gpuTr->dtr_SolDotOld = NULL; 
	gpuTr->dtr_SolDotDotOld = NULL;
	gpuTr->dtr_SolCurrent = NULL; 
	gpuTr->dtr_SolDot = NULL;
	gpuTr->dtr_SolDotDot = NULL;
	gpuTr->dtr_MD = NULL; 
	gpuTr->dtr_MDdot = NULL;
	gpuTr->dtr_MDdotdot = NULL;
	gpuTr->dtr_KD = NULL; 
	gpuTr->dtr_KDdot = NULL;
	gpuTr->dtr_KDdotdot = NULL;
	gpuTr->dtr_staticForce = NULL;
	gpuTr->dtr_Feff = NULL;
	gpuTr->dtr_M_coef = NULL;
	gpuTr->dtr_K_coef = NULL;
	gpuTr->dtr_h_KEeffTemplates = NULL;
	gpuTr->dtr_h_KEdummy = NULL;
	gpuTr->dtr_f = NULL;
	gpuTr->initial_dtr_f = NULL;
	gpuTr->dtr_f_value = 0;
	gpuTr->dtr_f = NULL;
	gpuTr->dtr_vonMises = 0;
	gpuTr->dtr_Weight = NULL;

	//
	gpuTr->dtr_N_gamma = 0.5;
	gpuTr->dtr_N_beta = pow((2*gpuTr->dtr_N_gamma+1),2.)/16.;
	//
	gpuTr->dtr_h = 1e-4;
	gpuTr->dtr_totalTime = 1e-2;
	gpuTr->dtr_CurrentTime = 0;
	gpuTr->dtr_TimeIndex = 0;
	//
	gpuTr->dtr_Damp_alpha = 0.0;
	gpuTr->dtr_Damp_beta =1e-4;
	//
	gpuTr->dtr_tolerance = 1E-10; 
	gpuTr->dtr_NumDOF = 3*h_nNodes;
	return 1;
}
/**********************************************************************/
__global__ void vectorMemset(int NumDOF, double *v,double value)
{
	register int dof;
	dof =  blockIdx.x*blockDim.x + threadIdx.x;
	if (dof<NumDOF)
		v[dof] = value;
	__syncthreads();
}
/**********************************************************************/
void initializeTransientForceVectorOnGPU(GPUData *h_gpuData,transientData *gpuTr)
{
	// GPU Memory allocation
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_f) );
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(gpuTr->initial_dtr_f) );
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_staticForce) );
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_Feff) );
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_Weight) );
	// set to zero
	dim3 dimBlock(THREADS);
	int nBlocks = gpuTr->dtr_NumDOF/(THREADS) + ((gpuTr->dtr_NumDOF%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);
	vectorMemset<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,gpuTr->dtr_f,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,gpuTr->initial_dtr_f,0.0);
	cublasDcopy (gpuTr->dtr_NumDOF,h_gpuData->d_f,1,gpuTr->dtr_staticForce,1);
	vectorMemset<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,gpuTr->dtr_Feff,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,gpuTr->dtr_Weight,0.0);

	checkCUDAStatus("GPU Force Initialization ");
}
/**********************************************************************/
void SetTransientFeffVectorOnGPU(GPUData *h_gpuData,transientData *gpuTr)
{
	cublasDcopy (gpuTr->dtr_NumDOF,gpuTr->dtr_Feff,1,h_gpuData->d_f,1);
	checkCUDAStatus("SetTransientFeffVectorOnGPU error \n");
}
/**********************************************************************/
void setTransientParamsOnGPU(transientData *gpuTr,double newmarkGamma, double newmarkBeta,
							 double dampingM, double dampingK,double transientDeltaTime,
							 double transientTotalTime,double tolerance)
{
	gpuTr->dtr_N_gamma = newmarkGamma;
	gpuTr->dtr_N_beta = newmarkBeta;
	gpuTr->dtr_Damp_alpha = dampingM;
	gpuTr->dtr_Damp_beta = dampingK;

	gpuTr->dtr_h = transientDeltaTime;
	gpuTr->dtr_totalTime = transientTotalTime;
	gpuTr->dtr_tolerance = tolerance;

}

/**********************************************************************/
void AllocateSolOnGPU(int h_nTemplateElements,transientData *gpuTr)
{
	int tr_NumDOF = gpuTr->dtr_NumDOF;
	// GPU Memory allocation
	printf("In AllocateSolOnGPU\n");
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolOld) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolDotOld) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolDotDotOld) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolCurrent) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolDot) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_SolDotDot) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_MD) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_MDdot) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_MDdotdot) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_KD) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_KDdot) );
	cublasAlloc( (tr_NumDOF), sizeof(double), (void **) &(gpuTr->dtr_KDdotdot) );
	
	cublasAlloc( ((h_nTemplateElements)*24*24), sizeof(double), (void **) &(gpuTr->dtr_h_KEdummy) );
	checkCUDAStatus("GPU Memory allocation ");
	
	// Set to zero
	dim3 dimBlock(THREADS);
	int nBlocks = tr_NumDOF/(THREADS) + ((tr_NumDOF%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	checkCUDAStatus("GPU Data Initialization 1 ");
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolOld,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolDotOld,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolDotDotOld,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolCurrent,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolDot,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_SolDotDot,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_MD,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_MDdot,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_MDdotdot,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_KD,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_KDdot,0.0);
	vectorMemset<<<dimGrid,dimBlock,0>>>(tr_NumDOF,gpuTr->dtr_KDdotdot,0.0);
	checkCUDAStatus("GPU Data Initialization 2 ");
	printf("here 1 \n");
	cudaMemset(gpuTr->dtr_h_KEdummy, 0,(h_nTemplateElements)*24*24);
	printf("here 2 \n");
	checkCUDAStatus("GPU Data Initialization 3 ");
}
/**********************************************************************/
__global__ void vectorDiffKernel(int NumDOF, double *v1,double *v2,double *diff)
{
	register int dof;
	dof =  blockIdx.x*blockDim.x + threadIdx.x;
	if (dof<NumDOF)
		diff[dof] = v1[dof]-v2[dof];
	__syncthreads();
}
/**********************************************************************/
double diffNormOnGPU(transientData *gpuTr)
{
	double *soldiff;
	cublasAlloc( (gpuTr->dtr_NumDOF), sizeof(double), (void **) &(soldiff) );
	dim3 dimBlock(THREADS);
	int nBlocks = gpuTr->dtr_NumDOF/(THREADS) + ((gpuTr->dtr_NumDOF%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);
	vectorMemset<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,soldiff,0.0);
	double temp1 = 0., temp2 = 0.;
	vectorDiffKernel <<< dimGrid,dimBlock,0 >>> (gpuTr->dtr_NumDOF,gpuTr->dtr_SolCurrent,gpuTr->dtr_SolOld,soldiff);
	temp1 = cublasDdot (gpuTr->dtr_NumDOF,soldiff, 1,soldiff, 1);
	temp2 = cublasDdot (gpuTr->dtr_NumDOF,gpuTr->dtr_SolCurrent, 1,gpuTr->dtr_SolCurrent, 1);

	cublasFree(soldiff);

	return (sqrt(temp1/temp2));
}
/**********************************************************************/
void UpdateSolOnGPU(transientData *gpuTr)
{
	cublasDcopy (gpuTr->dtr_NumDOF,gpuTr->dtr_SolCurrent,1,gpuTr->dtr_SolOld,1);
	cublasDcopy (gpuTr->dtr_NumDOF,gpuTr->dtr_SolDot,1,gpuTr->dtr_SolDotOld,1);
	cublasDcopy (gpuTr->dtr_NumDOF,gpuTr->dtr_SolDotDot,1,gpuTr->dtr_SolDotDotOld,1);
	checkCUDAStatus("GPU Update Solution ");
}
/**********************************************************************/
void setSolOnGPU(GPUData *h_gpuData,transientData *gpuTr)
{
	// SolCurrent to d_sol
	cublasDcopy (gpuTr->dtr_NumDOF,gpuTr->dtr_SolCurrent,1,h_gpuData->d_sol,1);
	checkCUDAStatus("GPU setSolOnGPU ");
}
/**********************************************************************/
void getSolOnGPU(GPUData *h_gpuData,transientData *gpuTr)
{
	// d_sol to SolCurrent
	cublasDcopy (gpuTr->dtr_NumDOF,h_gpuData->d_sol,1,gpuTr->dtr_SolCurrent,1);
	checkCUDAStatus("GPU getSolOnGPU ");
}
/**********************************************************************/
void copySol_GPUtoCPU(transientData *gpuTr,double *tr_SolCurrent)
{
	// SolCurrent GPU to SolCurrent CPU
	cublasGetVector (gpuTr->dtr_NumDOF,sizeof(double), gpuTr->dtr_SolCurrent,
		1,tr_SolCurrent, 1);
	checkCUDAStatus("GPU copySol_GPUtoCPU ");
}
/**********************************************************************/
__global__ void findVelocityAccelerationKernel(int h_nNodes,double tr_a0,double tr_a1,double tr_a2,double tr_a3,double tr_a4,double tr_a5,double tr_a6,double tr_a7,double *tr_SolCurrent,double *tr_SolDot,double *tr_SolDotDot,double *tr_SolOld,double *tr_SolDotOld,double *tr_SolDotDotOld,int *d_free)
{
	register int node,ufree,vfree,wfree;
	node =  blockIdx.x*blockDim.x + threadIdx.x;
	if (node<h_nNodes){
		wfree = d_free[node]/4;
		vfree = (d_free[node] - wfree*4)/2;
		ufree = (d_free[node] - wfree*4 -vfree*2);
		if (ufree){
			tr_SolDotDot[3*node] = tr_a0*(tr_SolCurrent[3*node]-tr_SolOld[3*node])-tr_a2*tr_SolDot[3*node]-tr_a3*tr_SolDotDotOld[3*node];
			tr_SolDot[3*node] = tr_SolDotOld[3*node]+tr_a6*tr_SolDotDotOld[3*node]+tr_a7*tr_SolDotDot[3*node];
		}
		if (vfree){
			tr_SolDotDot[3*node+1] = tr_a0*(tr_SolCurrent[3*node+1]-tr_SolOld[3*node+1])-tr_a2*tr_SolDot[3*node+1]-tr_a3*tr_SolDotDotOld[3*node+1];
			tr_SolDot[3*node+1] = tr_SolDotOld[3*node+1]+tr_a6*tr_SolDotDotOld[3*node+1]+tr_a7*tr_SolDotDot[3*node+1];
		}
		if (wfree){
			tr_SolDotDot[3*node+2] = tr_a0*(tr_SolCurrent[3*node+2]-tr_SolOld[3*node+2])-tr_a2*tr_SolDot[3*node+2]-tr_a3*tr_SolDotDotOld[3*node+2];
			tr_SolDot[3*node+2] = tr_SolDotOld[3*node+2]+tr_a6*tr_SolDotDotOld[3*node+2]+tr_a7*tr_SolDotDot[3*node+2];
		}
	}

	__syncthreads();
}
/**********************************************************************/
void findVelocityAccelerationOnGPU(GPUData *h_gpuData,transientData *gpuTr)
{
	int nNodes =  h_gpuData->h_nNodes;
	dim3 dimBlock(THREADS);
	int nBlocks =nNodes/(THREADS) + ((nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	findVelocityAccelerationKernel<<< dimGrid,dimBlock,0 >>>(nNodes,gpuTr->dtr_a0,gpuTr->dtr_a1,gpuTr->dtr_a2,gpuTr->dtr_a3,gpuTr->dtr_a4,gpuTr->dtr_a5,gpuTr->dtr_a6,gpuTr->dtr_a7,gpuTr->dtr_SolCurrent,gpuTr->dtr_SolDot,gpuTr->dtr_SolDotDot,gpuTr->dtr_SolOld,gpuTr->dtr_SolDotOld,gpuTr->dtr_SolDotDotOld,h_gpuData->d_free);
	checkCUDAStatus("findVelocityAccelerationOnGPU error");
}	
/**********************************************************************/
void findDampingCoefficientsOnGPU(transientData *gpuTr,double tr_Damp_alpha,double tr_Damp_beta)
{
	gpuTr->dtr_Damp_alpha = tr_Damp_alpha;
	gpuTr->dtr_Damp_beta = tr_Damp_beta;
}
/**********************************************************************/
void setMKCoefficientsOnGPU(transientData *gpuTr,double tr_a0,double tr_a1,double tr_a2,double tr_a3,double tr_a4,double tr_a5,double tr_a6,double tr_a7,double tr_MDcoef,double tr_MDdotcoef,double tr_MDdotdotcoef,double tr_KDcoef,double tr_KDdotcoef,double tr_KDdotdotcoef)
{
	gpuTr->dtr_a0 = tr_a0;
	gpuTr->dtr_a1 = tr_a1;
	gpuTr->dtr_a2 = tr_a2;
	gpuTr->dtr_a3 = tr_a3;
	gpuTr->dtr_a4 = tr_a4;
	gpuTr->dtr_a5 = tr_a5;
	gpuTr->dtr_a6 = tr_a6;
	gpuTr->dtr_a7 = tr_a7;
	// Feff
	gpuTr->dtr_MDcoef = tr_MDcoef;
	gpuTr->dtr_MDdotcoef = tr_MDdotcoef;
	gpuTr->dtr_MDdotdotcoef = tr_MDdotdotcoef;
	gpuTr->dtr_KDcoef = tr_KDcoef;
	gpuTr->dtr_KDdotcoef = tr_KDdotcoef;
	gpuTr->dtr_KDdotdotcoef = tr_KDdotdotcoef;
}
/**********************************************************************/
void setGravityOnGPU(transientData *gpuTr,double *gravity)
{
cublasAlloc(3, sizeof(double), (void **) &(gpuTr->dtr_gravity) );
cudaMemset(gpuTr->dtr_gravity,0,3*sizeof(double));
cublasSetVector (3,sizeof(double),gravity,1,gpuTr->dtr_gravity,1);
checkCUDAStatus("setGravityOnGPU error");
}
/**********************************************************************/
__global__ void setWeightKernel(int h_nNodes,double* dtr_Weight, double *dtr_gravity)
{
	int register node;
	node =  blockIdx.x*blockDim.x + threadIdx.x;
	if (node<h_nNodes){
		dtr_Weight[3*node] = dtr_gravity[0];
		dtr_Weight[3*node+1] = dtr_gravity[1];
		dtr_Weight[3*node+2] = dtr_gravity[2];
	}
	__syncthreads();
}
/**********************************************************************/
void findWeightOnGPU(GPUData *h_gpuData,transientData *gpuTr,bool h_nodeBasedAssembly)
{
	dim3 dimBlock(THREADS);
	int nBlocks = h_gpuData->h_nNodes/(THREADS) + ((h_gpuData->h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	setWeightKernel<<<dimGrid,dimBlock,0>>>(h_gpuData->h_nNodes,gpuTr->dtr_Weight,gpuTr->dtr_gravity);
	MuGPU(h_gpuData, gpuTr->dtr_Weight,gpuTr->dtr_Weight,h_nodeBasedAssembly);
}
/**********************************************************************/
__global__ void setFeffComponentsKernel(int dtr_NumDOF,double *dtr_MD,double *dtr_MDdot,double *dtr_MDdotdot,double *dtr_KD,double *dtr_KDdot,double *dtr_KDdotdot,double *dtr_staticForce,double *dtr_Weight,double dtr_MDcoef,double dtr_MDdotcoef,double dtr_MDdotdotcoef,double dtr_KDcoef,double dtr_KDdotcoef,double dtr_KDdotdotcoef,double *dtr_Feff)
{
	register int dof;
	dof =  blockIdx.x*blockDim.x + threadIdx.x;
	if (dof<dtr_NumDOF){
		dtr_MD[dof] *= dtr_MDcoef;
		dtr_MDdot[dof] *= dtr_MDdotcoef;
		dtr_MDdotdot[dof] *= dtr_MDdotdotcoef;
		dtr_KD[dof] *= dtr_KDcoef;
		dtr_KDdot[dof] *= dtr_KDdotcoef;
		dtr_KDdotdot[dof] *= dtr_KDdotdotcoef;	
		dtr_Feff[dof] = dtr_staticForce[dof]+dtr_MD[dof]+dtr_MDdot[dof]+dtr_MDdotdot[dof]+dtr_KD[dof]+dtr_KDdot[dof]+dtr_KDdotdot[dof]+dtr_Weight[dof];
	}
	__syncthreads();
} 
/**********************************************************************/
void EffeciveFOnGPU(GPUData *h_gpuData,transientData *gpuTr,bool h_nodeBasedAssembly)
{
	MuGPU(h_gpuData, gpuTr->dtr_SolOld,gpuTr->dtr_MD,h_nodeBasedAssembly);
	MuGPU(h_gpuData, gpuTr->dtr_SolDotOld,gpuTr->dtr_MDdot,h_nodeBasedAssembly);
	MuGPU(h_gpuData, gpuTr->dtr_SolDotDotOld,gpuTr->dtr_MDdotdot,h_nodeBasedAssembly);
	KuGPU(h_gpuData, gpuTr->dtr_SolOld,gpuTr->dtr_KD,h_nodeBasedAssembly);
	KuGPU(h_gpuData, gpuTr->dtr_SolDotOld,gpuTr->dtr_KDdot,h_nodeBasedAssembly);
	KuGPU(h_gpuData, gpuTr->dtr_SolDotDotOld,gpuTr->dtr_KDdotdot,h_nodeBasedAssembly);
	checkCUDAStatus("EffeciveFOnGPU error");

	dim3 dimBlock(THREADS);
	int nBlocks = gpuTr->dtr_NumDOF/(THREADS) + ((gpuTr->dtr_NumDOF%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	setFeffComponentsKernel<<<dimGrid,dimBlock,0>>>(gpuTr->dtr_NumDOF,gpuTr->dtr_MD,gpuTr->dtr_MDdot,gpuTr->dtr_MDdotdot,gpuTr->dtr_KD,gpuTr->dtr_KDdot,gpuTr->dtr_KDdotdot,gpuTr->dtr_staticForce,gpuTr->dtr_Weight,gpuTr->dtr_MDcoef,gpuTr->dtr_MDdotcoef,gpuTr->dtr_MDdotdotcoef,gpuTr->dtr_KDcoef,gpuTr->dtr_KDdotcoef,gpuTr->dtr_KDdotdotcoef,gpuTr->dtr_Feff);
	checkCUDAStatus("setFeffComponentsKernel error");
}
/**********************************************************************/
void setGPUTrackingOn(transientData *gpuTr,int tr_Track_DOF)
{
	gpuTr->dtr_Track = true;
	gpuTr->dtr_Track_DOF = tr_Track_DOF;
	printf("------------------------------------------------\n");
	printf("Tracking on GPU\n");
	printf("Status: On\n");
	printf("DOF = %i\n",gpuTr->dtr_Track_DOF);
	printf("------------------------------------------------\n");
}
/**********************************************************************/
void setGPUTrackingOff(transientData *gpuTr)
{
	gpuTr->dtr_Track = false;
	printf("------------------------------------------------\n");
	printf("Tracking on GPU\n");
	printf("Status: Off\n");
	printf("------------------------------------------------\n");
}
/**********************************************************************/
__global__ void TrackMemset(int NumSteps, double *v,double value)
{
	register int step;
	step =  blockIdx.x*blockDim.x + threadIdx.x;
	if (step<NumSteps)
		v[step] = value;
	__syncthreads();
}
/**********************************************************************/
void initializeTrackingOnGPU(transientData *gpuTr)
{
	// GPU Memory allocation
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_time) );
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_force) );
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_displacement) );
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_velocity) );
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_acceleration) );
	cublasAlloc( (MAX_TIME_STEPS), sizeof(double), (void **) &(gpuTr->dtr_Track_stress) );

	checkCUDAStatus("initializeTrackingOnGPU allocation error");
	// set to zero
	dim3 dimBlock(THREADS);
	int nBlocks = MAX_TIME_STEPS/(THREADS) + ((MAX_TIME_STEPS%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 

	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_time,0.0);
	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_force,0.0);
	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_displacement,0.0);
	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_velocity,0.0);
	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_acceleration,0.0);
	TrackMemset<<<dimGrid,dimBlock,0>>>(MAX_TIME_STEPS, gpuTr->dtr_Track_stress,0.0);

	checkCUDAStatus("initializeTrackingOnGPU error");
}
/**********************************************************************/
__global__ void updateTrackingKernel(int dof,int timeID,double time,double *disp,double Max_stress,double *dtr_time,double *dtr_disp,double *dtr_stress)
{
		dtr_time[timeID] = time;
		dtr_disp[timeID] = disp[dof];
		dtr_stress[timeID] = Max_stress;
}
/**********************************************************************/
void updateTrackingOnGPU(transientData *gpuTr,double Max_stress)
{
	updateTrackingKernel<<<1,1>>>(gpuTr->dtr_Track_DOF,gpuTr->dtr_TimeIndex,gpuTr->dtr_CurrentTime,gpuTr->dtr_SolCurrent,Max_stress,gpuTr->dtr_Track_time,gpuTr->dtr_Track_displacement,gpuTr->dtr_Track_stress);
	checkCUDAStatus("updateTrackingOnGPU error");

}
/**********************************************************************/
__global__ void NormalizeTrackedOnGPUkernel(double *d_dispScale,double *d_stressScale,double *d_timeScale,int dtr_max_displacement,int dtr_max_stress,int dtr_max_time,double *dtr_Track_displacement,double *dtr_Track_stress,double *dtr_Track_time)
{

		*d_dispScale = 1./dtr_Track_displacement[dtr_max_displacement];
		*d_stressScale = 1./dtr_Track_stress[dtr_max_stress];
		*d_timeScale = 1./dtr_Track_time[dtr_max_time];
}
/**********************************************************************/
void NormalizeTrackedOnGPU(transientData *gpuTr)
{
	int NumSteps = MAX_TIME_STEPS;
	int dtr_max_displacement = cublasIdamax(NumSteps,gpuTr->dtr_Track_displacement,1)-1;
	int dtr_max_stress = cublasIdamax(NumSteps,gpuTr->dtr_Track_stress,1)-1;
	int dtr_max_time = cublasIdamax(NumSteps,gpuTr->dtr_Track_time,1)-1;
	double *d_dispScale,*d_stressScale,*d_timeScale;
	double dispScale,stressScale,timeScale;
	cudaMalloc((void**)&d_dispScale,sizeof(double));
	cudaMalloc((void**)&d_stressScale,sizeof(double));
	cudaMalloc((void**)&d_timeScale,sizeof(double));
	NormalizeTrackedOnGPUkernel<<<1,1>>>(d_dispScale,d_stressScale,d_timeScale,dtr_max_displacement,dtr_max_stress,dtr_max_time,gpuTr->dtr_Track_displacement,gpuTr->dtr_Track_stress,gpuTr->dtr_Track_time);
	cudaMemcpy(&dispScale,d_dispScale,sizeof(double),cudaMemcpyDeviceToHost);
	cudaMemcpy(&stressScale,d_stressScale,sizeof(double),cudaMemcpyDeviceToHost);
	cudaMemcpy(&timeScale,d_timeScale,sizeof(double),cudaMemcpyDeviceToHost);
	cublasDscal (NumSteps,dispScale,gpuTr->dtr_Track_displacement,1);
	cublasDscal (NumSteps,stressScale,gpuTr->dtr_Track_stress,1);
	cublasDscal (NumSteps,timeScale,gpuTr->dtr_Track_time,1);
	cudaFree(d_dispScale);
	cudaFree(d_stressScale);
	cudaFree(d_timeScale);
	checkCUDAStatus("NormalizeTrackedOnGPU error");
}
/**********************************************************************/
int getNumberOfTimeStepsTrackedOnGPU(transientData *gpuTr)
{
	return gpuTr->dtr_TimeIndex;
}
/**********************************************************************/
void getTimeStepsTrackedOnGPU(transientData *gpuTr,double *time)
{
	cublasGetVector (MAX_TIME_STEPS,sizeof(double),gpuTr->dtr_Track_time,1,time,1);
	checkCUDAStatus("getTimeStepsTrackedOnGPU error");
}
/**********************************************************************/
void getDisplacementTrackedOnGPU(transientData *gpuTr,double *disp)
{
	cublasGetVector (MAX_TIME_STEPS,sizeof(double),gpuTr->dtr_Track_displacement,1,disp,1);
	checkCUDAStatus("getDisplacementTrackedOnGPU error");
}
/**********************************************************************/
void getStressTrackedOnGPU(transientData *gpuTr,double *stress)
{
	cublasGetVector (MAX_TIME_STEPS,sizeof(double),gpuTr->dtr_Track_stress,1,stress,1);
	checkCUDAStatus("getStressTrackedOnGPU error");
}
/**********************************************************************/
void updateTimeOnGPU(transientData *gpuTr,int tr_TimeIndex,double tr_CurrentTime)
{
	gpuTr->dtr_TimeIndex = tr_TimeIndex;
	gpuTr->dtr_CurrentTime = tr_CurrentTime; 

	checkCUDAStatus("updateTimeOnGPU error");
}
/**********************************************************************/
void FreeGPU(transientData *gpuTr)
{
	cublasFree(gpuTr->dtr_SolOld); 
	cublasFree(gpuTr->dtr_SolDotOld); 
	cublasFree(gpuTr->dtr_SolDotDotOld);
	cublasFree(gpuTr->dtr_SolCurrent); 
	cublasFree(gpuTr->dtr_SolDot);
	cublasFree(gpuTr->dtr_SolDotDot);
	cublasFree(gpuTr->dtr_MD); 
	cublasFree(gpuTr->dtr_MDdot);
	cublasFree(gpuTr->dtr_MDdotdot);
	cublasFree(gpuTr->dtr_KD); 
	cublasFree(gpuTr->dtr_KDdot);
	cublasFree(gpuTr->dtr_KDdotdot);
	cublasFree(gpuTr->dtr_Feff);
	cublasFree(gpuTr->dtr_h_KEeffTemplates);
	cublasFree(gpuTr->dtr_f);

	cublasFree(gpuTr->dtr_Track_time);
	cublasFree(gpuTr->dtr_Track_force);
	cublasFree(gpuTr->dtr_Track_displacement);
	cublasFree(gpuTr->dtr_Track_velocity);
	cublasFree(gpuTr->dtr_Track_acceleration);
	cublasFree(gpuTr->dtr_Track_stress);

	cublasFree(gpuTr->dtr_Weight);
	cublasFree(gpuTr->dtr_staticForce);

}
/**********************************************************************/
#endif