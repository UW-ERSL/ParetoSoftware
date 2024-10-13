#include <stdio.h>
#include <math.h>
#include <cuda.h>
#include "../inc/hexmesh/Timer.h"
#include "cublas.h"
#include <string.h>
#include <stdlib.h>
#include "../inc/hexmesh/AFElasticityGPUDefines.h"

#define E_RHO_PENALTY 3
#define M_RHO_PENALTY 3
static bool	h_hexmeshGPU_verbose;// if set to true, print progress for debugging stuff
void setGPUVerbose(bool val)
{
	h_hexmeshGPU_verbose = val;
}

bool getGPUVerbose(void)
{
	return h_hexmeshGPU_verbose;
}

/**********************************************************************/

#ifdef COMPILE_GPUCODE

__device__ __constant__ double KeConstantMem[24*24];// can use, but not worth it
/**********************************************************************/
void dumpGPUInfo()
{
	size_t free, total;
	int gpuCount, i;
	CUresult res;
	CUdevice dev;
	CUcontext ctx;
	cudaDeviceProp devProp;

	cuInit(0);

	cuDeviceGetCount(&gpuCount);
	printf("AFElasticityGPU: Detected %d GPU\n",gpuCount);

	for (i=0; i<gpuCount; i++) {
		cuDeviceGet(&dev,i);
		cuCtxCreate(&ctx, 0, dev);
		res = cuMemGetInfo(&free, &total);
		if(res != CUDA_SUCCESS)
			printf("AFElasticityGPU: cuMemGetInfo failed! (%x)", res);
		printf("AFElasticityGPU: Device: %d\n",i);
        cudaGetDeviceProperties(&devProp, i);
		printf("AFElasticityGPU: Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
		printf("AFElasticityGPU: Major revision number:         %d\n", devProp.major);
		printf("AFElasticityGPU: Minor revision number:         %d\n", devProp.minor);
		printf("AFElasticityGPU: Name:                          %s\n", devProp.name);
        printf("AFElasticityGPU: Total global memory:           %zu\n", devProp.totalGlobalMem);
        printf("AFElasticityGPU: Total shared memory per block: %zu\n", devProp.sharedMemPerBlock);
		printf("AFElasticityGPU: Total registers per block:     %d\n", devProp.regsPerBlock);
		printf("AFElasticityGPU: Warp size:                     %d\n", devProp.warpSize);
        printf("AFElasticityGPU: Maximum memory pitch:          %zu\n", devProp.memPitch);
		printf("AFElasticityGPU: Maximum threads per block:     %d\n", devProp.maxThreadsPerBlock);
		for (int i = 0; i < 3; ++i)
			printf("AFElasticityGPU: Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
		for (int i = 0; i < 3; ++i)
			printf("AFElasticityGPU: Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
		printf("AFElasticityGPU: Clock rate:                    %d\n", devProp.clockRate);
        printf("AFElasticityGPU: Total constant memory:         %zu\n", devProp.totalConstMem);
        printf("AFElasticityGPU: Texture alignment:             %zu\n", devProp.textureAlignment);
		printf("AFElasticityGPU: Concurrent copy and execution: %s\n", (devProp.deviceOverlap ? "Yes" : "No"));
		printf("AFElasticityGPU: Number of multiprocessors:     %d\n", devProp.multiProcessorCount);
		printf("AFElasticityGPU: Kernel execution timeout:      %s\n", (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
		printf("AFElasticityGPU: Free :%lu MB\n", (free/1024/1024));
		cuCtxDetach(ctx);
   }
}
/**********************************************************************/
int isThereValidCudaDevice(void) 
{
    int deviceCount,dev = 0;
	cudaDeviceProp deviceProp;

	cuInit(0);
	dumpGPUInfo();
    cuDeviceGetCount(&deviceCount);
	if (deviceCount == 0) 
		return -1;
	
    cudaGetDeviceProperties(&deviceProp, dev);
    
	if (deviceProp.major == 9999 && deviceProp.minor == 9999)   return -1;
	if (deviceProp.major == 1 && deviceProp.minor < 3) 	return -1;
	cudaFree(0);
	cublasStatus status = cublasInit();// for Cublas Vector operations
	
	if (status != CUBLAS_STATUS_SUCCESS) {
		printf("AFElasticityGPU: Cublas Init failure\n");
		return -1;
	}
	cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);

	return 1;
}

/**********************************************************************/
// Simple utility function to check for CUDA runtime errors
bool checkCUDAStatus(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) {
        fprintf(stderr, "%s: %s.\n", msg,cudaGetErrorString( err) );
		return false;
    }               
	return true;
}

/**********************************************************************/
bool createAFElasticityGPUMemory(int h_nNodes,int h_nElems, int h_maxNeighboringElements, int h_nTemplateElements, GPUData *h_gpuData)
{
	size_t freeInitial, freeFinal, total;

	if (h_nNodes == 0) return false;
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Allocating memory in GPU \n");
	cuMemGetInfo(&freeInitial, &total);
	h_gpuData->h_nNodes = h_nNodes;
	h_gpuData->h_nElems = h_nElems;
	h_gpuData->h_maxNeighboringElements = h_maxNeighboringElements;
	h_gpuData->h_nTemplateElements = h_nTemplateElements;

	// GPU Memory allocation
	cublasAlloc( (h_nNodes), sizeof(int), (void **) &(h_gpuData->d_free) );
	cublasAlloc( (3*h_nNodes), sizeof(double), (void **) &(h_gpuData->d_sol) );			//  displacement solution
	cublasAlloc( (3*h_nNodes), sizeof(double), (void **) &(h_gpuData->d_f) );			// force
	cublasAlloc( (h_nElems), sizeof(double), (void **) &(h_gpuData->d_pseudodensity) );
	cublasAlloc( (h_nNodes), sizeof(bool), (void **) &(h_gpuData->d_nodeExists) );

	cublasAlloc( (h_nNodes), sizeof(int), (void **) &(h_gpuData->d_nodeSignature) );	
	cublasAlloc( (27*h_nNodes), sizeof(int), (void **) &(h_gpuData->d_neighborNodes) );
	cublasAlloc( 256*3*81, sizeof(double), (void **) &(h_gpuData->d_KeNode) );	// for each node, its row entries for all possible combinations
	cublasAlloc( 256*3*81, sizeof(double), (void **) &(h_gpuData->d_MeNode) );	// for each node, its row entries for all possible combinations

	cublasAlloc( 8*(h_nElems), sizeof(int), (void **) &(h_gpuData->d_elems) );
	cublasAlloc( h_maxNeighboringElements*(h_nNodes), sizeof(int), (void **) &(h_gpuData->d_neighborElems) );
	cublasAlloc( (h_nElems), sizeof(int), (void **) &(h_gpuData->d_templateID) );
	cublasAlloc( (h_nNodes), sizeof(int), (void **) &(h_gpuData->d_nNeighboringElems) );
	cublasAlloc( (h_nTemplateElements)*24*24, sizeof(double), (void **) &(h_gpuData->d_KETemplates) );	// Ke templates
	cublasAlloc( (h_nTemplateElements)*24*24, sizeof(double), (void **) &(h_gpuData->d_METemplates) );	// Me templates

	if (!checkCUDAStatus("AFElasticityGPU: GPU Memory allocation ")) return false;
	if (h_hexmeshGPU_verbose)  printf("AFElasticityGPU: Memory allocation successful \n");
	cuMemGetInfo(&freeFinal, &total);
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Allocated %f MBytes \n",1.0*(freeInitial-freeFinal)/1024.0/1024.0);

	return true;
}
/**********************************************************************/
void initializeGPUHexmeshData(int h_nNodes,int h_nElems, int h_maxNeighboringElements, int h_nTemplateElements, 
							int *h_elems,double *h_pseudoDensity,bool *h_nodeExists,int *h_templateID,
							double *h_sol,int *h_free,double *h_f,int *h_neighborElems, int *h_nNeighboringElems,double *h_KETemplates,
							double *h_METemplates,int *h_nodeSignature,int *h_neighborNodes, double *h_KeNode, double *h_MeNode,GPUData *h_gpuData)
{

	if ((h_gpuData == NULL) || (h_gpuData->h_nNodes == 0)) {
		printf("AFElasticityGPU: GPU memory has not been allocated\n");
	}

	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Copying hexmesh data from CPU to GPU \n");
	//	Copy from Host to Device
	cublasSetVector( (h_nNodes), sizeof(int), h_free, 1, h_gpuData->d_free, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_sol, 1, h_gpuData->d_sol, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_f, 1, h_gpuData->d_f, 1 );
	cublasSetVector( (h_nElems), sizeof(double), h_pseudoDensity, 1, h_gpuData->d_pseudodensity, 1 );
	cublasSetVector( (h_nNodes), sizeof(bool), h_nodeExists, 1, h_gpuData->d_nodeExists, 1 );
	
	checkCUDAStatus("AFElasticityGPU: GPU Transfer Set 1 ");

	cublasSetVector( 8*(h_nElems), sizeof(int), h_elems, 1, h_gpuData->d_elems, 1 );	
	cublasSetVector( (h_nElems), sizeof(int), h_templateID, 1, h_gpuData->d_templateID, 1 );
	cublasSetVector( h_maxNeighboringElements*(h_nNodes), sizeof(int), h_neighborElems, 1, h_gpuData->d_neighborElems, 1 );
	cublasSetVector( (h_nNodes), sizeof(int), h_nNeighboringElems, 1, h_gpuData->d_nNeighboringElems, 1 );
	cublasSetVector((h_nTemplateElements)*24*24, sizeof(double),h_KETemplates,1,h_gpuData->d_KETemplates, 1 );
	cublasSetVector((h_nTemplateElements)*24*24, sizeof(double),h_METemplates,1,h_gpuData->d_METemplates, 1 );
	checkCUDAStatus("AFElasticityGPU:  GPU Transfer Set 2  ");

	if (h_KeNode != NULL)
		cublasSetVector(256*3*81, sizeof(double),h_KeNode,1,h_gpuData->d_KeNode, 1 );

	if (h_MeNode != NULL)
		cublasSetVector(256*3*81, sizeof(double),h_MeNode,1,h_gpuData->d_MeNode, 1 );

	checkCUDAStatus("AFElasticityGPU:  GPU Transfer Set 3 ");
	if (h_nodeSignature != NULL)
		cublasSetVector( (h_nNodes), sizeof(int), h_nodeSignature, 1, h_gpuData->d_nodeSignature, 1 );
	if (h_neighborNodes != NULL)
		cublasSetVector( 27*(h_nNodes), sizeof(int), h_neighborNodes, 1, h_gpuData->d_neighborNodes, 1 );
	
	checkCUDAStatus("AFElasticityGPU:  GPU Transfer Set 4  ");
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Transferred data from CPU to GPU  \n");

}
/**********************************************************************/
void setGPUForcesBC(int h_nNodes, double *h_sol,int *h_free,double *h_f,GPUData *h_gpuData)
{
	if ((h_gpuData == NULL) || (h_gpuData->h_nNodes == 0)) {
		printf("AFElasticityGPU: GPU memory has not been allocated\n");
	}

	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Copying forces and BC from CPU to GPU \n");
	//	Copy from Host to Device
	cublasSetVector( (h_nNodes), sizeof(int), h_free, 1, h_gpuData->d_free, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_sol, 1, h_gpuData->d_sol, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_f, 1, h_gpuData->d_f, 1 );
	
	checkCUDAStatus("AFElasticityGPU: GPU Data Initialization ");
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Transferred forces and BC data from CPU to GPU  \n");

}
/**********************************************************************/
void updateBCsOnGPU(int h_nNodes,double *h_sol, int *h_free,double *h_f,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector( (h_nNodes), sizeof(int), h_free, 1, h_gpuData->d_free, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_sol, 1, h_gpuData->d_sol, 1 );
	cublasSetVector( (3*h_nNodes), sizeof(double), h_f, 1, h_gpuData->d_f, 1 );
	checkCUDAStatus("updateBCsOnGPU error");
}

/**********************************************************************/
void updateKeOnGPU(int h_nTemplateElements,double *h_KETemplates,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector((h_nTemplateElements)*24*24, sizeof(double),h_KETemplates,1,h_gpuData->d_KETemplates, 1 );
	checkCUDAStatus("updateKeOnGPU error");
}
/**********************************************************************/
void updateMeOnGPU(int h_nTemplateElements,double *h_METemplates,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector((h_nTemplateElements)*24*24, sizeof(double),h_METemplates,1,h_gpuData->d_METemplates, 1 );
	checkCUDAStatus("updateMeOnGPU error");
}
/**********************************************************************/
void setKeNodeOnGPU(double *h_KeNode,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(81*3*256,sizeof(double),h_KeNode,1,h_gpuData->d_KeNode, 1 );
	checkCUDAStatus("setKeNodeOnGPU error");
}
/**********************************************************************/
void setMeNodeOnGPU(double *h_MeNode,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(81*3*256,sizeof(double),h_MeNode,1,h_gpuData->d_MeNode, 1 );
	checkCUDAStatus("setMeNodeOnGPU error");
}
/**********************************************************************/
void updateNodeNeighborsOnGPU(int h_nNodes, int *h_neighborNodes,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(h_nNodes*27,sizeof(int),h_neighborNodes,1,h_gpuData->d_neighborNodes, 1 );
	checkCUDAStatus("updateNodeNeighborsOnGPU error");
}
/**********************************************************************/
void updateNodeSignaturesOnGPU(int h_nNodes, int *h_nodeSignature,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(h_nNodes,sizeof(int),h_nodeSignature,1,h_gpuData->d_nodeSignature, 1 );
	checkCUDAStatus("updateNodeSignaturesOnGPU error");
}
/**********************************************************************/
void setGPUPseudoDensity(int h_nElems,double *h_pseudoDensity,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector( (h_nElems), sizeof(double), h_pseudoDensity, 1, h_gpuData->d_pseudodensity, 1 );
	checkCUDAStatus("setGPUPseudoDensity error");
}
/**********************************************************************/
void setGPUNodeExists(int h_nNodes,bool *h_nodeExists,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector( (h_nNodes), sizeof(bool), h_nodeExists, 1, h_gpuData->d_nodeExists, 1 );
	checkCUDAStatus("setGPUNodeExists error");
}
/**********************************************************************/
void setGPUSolution(int h_nNodes,double *sol,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(3*h_nNodes, sizeof(double), sol, 1, h_gpuData->d_sol, 1 );
	checkCUDAStatus("setGPUSolution error");
}
/**********************************************************************/
void setGPUForce(int h_nNodes,double *f,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(3*h_nNodes, sizeof(double),f, 1, h_gpuData->d_f, 1 );
	checkCUDAStatus("setGPUForce error");
}
/**********************************************************************/
void setGPUFreeDOF(int h_nNodes,int *free,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(h_nNodes, sizeof(int),free, 1, h_gpuData->d_free, 1 );
	checkCUDAStatus("setGPUFreeDOF error");
}
/**********************************************************************/
void updateNumNeighboringElemsOnGPU(int h_nNodes,int *h_nNeighboringElems,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(h_nNodes, sizeof(int),h_nNeighboringElems, 1, h_gpuData->d_nNeighboringElems, 1 );
	checkCUDAStatus("updateNumNeighboringElems error");
}
/**********************************************************************/
void updateNeighboringElemsOnGPU(int h_nNodes,int h_maxNeighboringElements,int *h_neighboringElems,GPUData *h_gpuData)
{
	if (h_gpuData->h_nNodes == 0) return;
	cublasSetVector(h_maxNeighboringElements*h_nNodes, sizeof(int),h_neighboringElems, 1, h_gpuData->d_neighborElems, 1 );
	checkCUDAStatus("updateNeighboringElems error");
}
/**********************************************************************/
__global__ void ZEAXPBYKernel(int h_nNodes,double *z,double a,double *x,double b, double *y)

// z = a*x + b*y
// Each thread is assigned to a node
{	
	register int node;
	node = blockIdx.x*blockDim.x + threadIdx.x;
	if (node < h_nNodes) {
		z[3*node] = a*x[3*node] + b*y[3*node];
		z[3*node+1] = a*x[3*node+1] + b*y[3*node+1];
		z[3*node+2] = a*x[3*node+2] + b*y[3*node+2];
	}
}
/**********************************************************************/
__global__ void DivideByVectorKernel(int h_nNodes,double *d, double *r,double *a)
// a = r./d
// Each thread is assigned to a node
{	
	register int node;

	node = blockIdx.x*blockDim.x + threadIdx.x;

	if (node < h_nNodes) {
		a[3*node] = r[3*node]/d[3*node];
		a[3*node+1] = r[3*node+1]/d[3*node+1];
		a[3*node+2] = r[3*node+2]/d[3*node+2];
	}
}
/**********************************************************************/
__global__ void ComputeDiagKernel(int h_nNodes,int h_nElems, int h_nTemplateElements, int *d_nNeighboringElems,
						double *d_rho, int *d_neighborElems, int *d_templateID, double *d_KETemplates,double *d)
// Compute the diagonal of the K maitrx
// Each thread is assigned to a node
{	
	register int j,node,elem,KEindex,maxElem,nodeIndex,elemID,codedElemNum;
	register double uResult=0.0,vResult=0.0,wResult=0.0;
	node = blockIdx.x*blockDim.x + threadIdx.x ;

	if (node < h_nNodes) {
		uResult=0.0;
		vResult=0.0;
		wResult=0.0;
		maxElem = d_nNeighboringElems[node];// how many elements connected to node
		for (j =0; j < maxElem; j++) {//
			codedElemNum = d_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
			nodeIndex = codedElemNum/MAXELEMS;
			elem =codedElemNum -nodeIndex*MAXELEMS;  
			elemID = d_templateID[elem];

			KEindex = (24*3*nodeIndex + 3*nodeIndex)*(h_nTemplateElements) + elemID;
			uResult += d_KETemplates[KEindex];			
					
			KEindex = (24*3*nodeIndex + 24 + 3*nodeIndex + 1)*(h_nTemplateElements) + elemID;
			vResult += d_KETemplates[KEindex];

			KEindex = (24*3*nodeIndex + 48 + 3*nodeIndex + 2)*(h_nTemplateElements) + elemID;
			wResult += d_KETemplates[KEindex];
		}
		// write back the result
		d[3*node] = uResult;
		d[3*node+1] = vResult;
		d[3*node+2] = wResult;
	}
}
/**********************************************************************/
__global__ void KuKernelElemBasedConstantMemory(int h_nNodes,int h_nElems, int h_nTemplateElements, int *d_nNeighboringElems,
						int *d_elems, double *d_pseudodensity, int *d_neighborElems, int *d_templateID, double *d_KETemplates,
						double *x,int *d_free,double *y)
// Carry out assembly-free K*u on the GPU
// Each thread is assigned to a node
{	
	 register int i,j,node,node2,temp, elem,KEindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree,elemID;
	 register double KE, rho,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
	 
	
	node = blockIdx.x*blockDim.x + threadIdx.x;


	if (node < h_nNodes) {
		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);

		if (ufree || vfree || wfree) {// process only if any of the dof is free
			maxElem = d_nNeighboringElems[node];// how many elements connected to node
			for (j =0; j < maxElem; j++) {//
				codedElemNum = d_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
				elem = codedElemNum - nodeIndex*MAXELEMS;
				rho = d_pseudodensity[elem];
				if (rho == 0)// element exists but is void (example: during topology optimization)
					continue; 
				elemID = d_templateID[elem];
                rho = pow((double)rho,(double)E_RHO_PENALTY);
				for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
					node2 = d_elems[(h_nElems)*i + elem];
					uVal = x[3*node2];
					vVal = x[3*node2+1];
					wVal = x[3*node2+2];
					
					KEindex = (24*3*nodeIndex + 3*i)*(h_nTemplateElements) + elemID;
					KE = KeConstantMem[KEindex];
					uResult += rho*KE*uVal;			

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					uResult +=  rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					uResult += rho*KE*wVal;
					
					//KEindex = (24*3*nodeIndex + 24+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					vResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					vResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					vResult += rho*KE*wVal;

					//KEindex = (24*3*nodeIndex + 48+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					wResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					wResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = KeConstantMem[KEindex];
					wResult += rho*KE*wVal;
				}
			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}
/**********************************************************************/
__global__ void KuKernelElemBased(int h_nNodes,int h_nElems, int h_nTemplateElements,bool *d_nodeExists, int *d_nNeighboringElems,
						int *d_elems, double *d_pseudodensity, int *d_neighborElems, int *d_templateID, double *d_KETemplates,
						double *x,int *d_free,double *y)
// Carry out assembly-free K*u on the GPU
// Each thread is assigned to a node
{	
	 register int i,j,node,node2,temp, elem,KEindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree,elemID;
	 register double KE, rho,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
		
	node = blockIdx.x*blockDim.x + threadIdx.x;
    	
	if (node < h_nNodes) {
		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);

		if ( (d_nodeExists[node]) && (ufree || vfree || wfree)) {// process only if any of the dof is free
			maxElem = d_nNeighboringElems[node];// how many elements connected to node
			for (j =0; j < maxElem; j++) {//
				codedElemNum = d_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
				nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
				elem = codedElemNum - nodeIndex*MAXELEMS;
				rho = d_pseudodensity[elem];
				if (rho == 0)// element exists but is void (example: during topology optimization)
					continue; 
				elemID = d_templateID[elem];
                rho = pow((double)rho,(double)E_RHO_PENALTY);
				for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
					node2 = d_elems[(h_nElems)*i + elem];
					uVal = x[3*node2];
					vVal = x[3*node2+1];
					wVal = x[3*node2+2];
					
					KEindex = (24*3*nodeIndex + 3*i)*(h_nTemplateElements) + elemID;
					KE = d_KETemplates[KEindex];
					uResult += rho*KE*uVal;			

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					uResult +=  rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					uResult += rho*KE*wVal;
					
					//KEindex = (24*3*nodeIndex + 24+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*wVal;

					//KEindex = (24*3*nodeIndex + 48+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*wVal;
				}
			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}
/**********************************************************************/
/*__global__ void KuKernelElemBased(int h_nNodes,int h_nElems, int h_nTemplateElements,bool *d_nodeExists, int *d_nNeighboringElems,
						int *d_elems, double *d_pseudodensity, int *d_neighborElems, int *d_templateID, double *d_KETemplates,
						double *x,int *d_free,double *y)
// Carry out assembly-free K*u on the GPU
// Each thread is assigned to a node
{	
	 register int i,j,node,node2,temp, elem,KEindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree,elemID;
	 register double KE, rho,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
	 
	
	node = blockIdx.x*blockDim.x + threadIdx.x;

	if (node < h_nNodes) {
		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);

		if ( (d_nodeExists[node]) && (ufree || vfree || wfree)) {// process only if any of the dof is free
			maxElem = d_nNeighboringElems[node];// how many elements connected to node
			for (j =0; j < maxElem; j++) {//
				codedElemNum = d_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
				nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
				elem = codedElemNum - nodeIndex*MAXELEMS;
				rho = d_pseudodensity[elem];
				if (rho == 0)// element exists but is void (example: during topology optimization)
					continue; 
				elemID = d_templateID[elem];
                rho = pow((double)rho,(double)E_RHO_PENALTY);
				for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
					node2 = d_elems[(h_nElems)*i + elem];
					uVal = x[3*node2];
					vVal = x[3*node2+1];
					wVal = x[3*node2+2];
					
					KEindex = (24*3*nodeIndex + 3*i)*(h_nTemplateElements) + elemID;
					KE = d_KETemplates[KEindex];
					uResult += rho*KE*uVal;			

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					uResult +=  rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					uResult += rho*KE*wVal;
					
					//KEindex = (24*3*nodeIndex + 24+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					vResult += rho*KE*wVal;

					//KEindex = (24*3*nodeIndex + 48+3*i)*(h_nTemplateElements) + elemID;
					KEindex += 22*h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*uVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*vVal;

					KEindex += h_nTemplateElements;
					KE = d_KETemplates[KEindex];
					wResult += rho*KE*wVal;
				}
			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}*/
/**********************************************************************/
__global__ void KuKernelNodeBased(int h_nNodes,bool *d_nodeExists,int *d_nodeSignature,int *d_neighborNodes, 
								double *d_KeNode,double *x,int *d_free,double *y)
// Carry out assembly-free K*u on the GPU
// Each thread is assigned to a node
{	
	register int j,node,node2,temp,KEIndex,index,ufree,vfree,wfree;
	register double KE,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
	 
	node = blockIdx.x*blockDim.x + threadIdx.x;
	if ( (node < h_nNodes) ) {

		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);
		
		if ((d_nodeExists[node]) && (ufree || vfree || wfree)) {// process only if any of the dof is free
			KEIndex = d_nodeSignature[node];// from 0 to 256

			for (j =0; j < 27; j++) {//
				node2 = d_neighborNodes[j*h_nNodes + node];// process neighbor
				if (node2 == -1) continue;

				uVal = x[3*node2];
				vVal = x[3*node2+1];
				wVal = x[3*node2+2];

				index = 81*3*KEIndex + 3*j;

				KE = d_KeNode[index];

				uResult += KE*uVal;
				KE = d_KeNode[index+1];
				uResult += KE*vVal;
				KE = d_KeNode[index+2];
				uResult += KE*wVal;

				KE = d_KeNode[index+81];
				vResult += KE*uVal;
				KE = d_KeNode[index+82];
				vResult += KE*vVal;
				KE = d_KeNode[index+83];
				vResult += KE*wVal;

				KE = d_KeNode[index+162];
				wResult += KE*uVal;
				KE = d_KeNode[index+163];
				wResult += KE*vVal;
				KE = d_KeNode[index+164];
				wResult += KE*wVal;

			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}
/**********************************************************************/
__global__ void MuKernelElemBased(int h_nNodes,int h_nElems, int h_nTemplateElements, int *d_nNeighboringElems,
						int *d_elems, double *d_pseudodensity, int *d_neighborElems, int *d_templateID, double *d_METemplates,
						double *x,int *d_free,double *y)
// Carry out assembly-free M*u on the GPU
// Each thread is assigned to a node
{	
	register int i,j,node,node2,temp, elem,MEindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree,elemID;
	register double ME, rho,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
	 
	node = blockIdx.x*blockDim.x + threadIdx.x;

	if (node < h_nNodes) {
		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);

		if (ufree || vfree || wfree) {// process only if any of the dof is free
			maxElem = d_nNeighboringElems[node];// how many elements connected to node
			for (j =0; j < maxElem; j++) {//
				codedElemNum = d_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
				nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
				elem = codedElemNum - nodeIndex*MAXELEMS;
				rho = d_pseudodensity[elem];
				if (rho == 0)// element exists but is void (example: during topology optimization)
					continue; 
				elemID = d_templateID[elem];
				rho = pow((double)rho,(double)M_RHO_PENALTY);
				for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
					node2 = d_elems[(h_nElems)*i + elem];
					uVal = x[3*node2];
					vVal = x[3*node2+1];
					wVal = x[3*node2+2];
					
					MEindex = (24*3*nodeIndex + 3*i)*(h_nTemplateElements) + elemID;
					ME = d_METemplates[MEindex];
					uResult += rho*ME*uVal;			

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					uResult +=  rho*ME*vVal;

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					uResult += rho*ME*wVal;
					
					//MEindex = (24*3*nodeIndex + 24+3*i)*(h_nTemplateElements) + elemID;
					MEindex += 22*h_nTemplateElements;
					ME = d_METemplates[MEindex];
					vResult += rho*ME*uVal;

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					vResult += rho*ME*vVal;

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					vResult += rho*ME*wVal;

					//MEindex = (24*3*nodeIndex + 48+3*i)*(h_nTemplateElements) + elemID;
					MEindex += 22*h_nTemplateElements;
					ME = d_METemplates[MEindex];
					wResult += rho*ME*uVal;

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					wResult += rho*ME*vVal;

					MEindex += h_nTemplateElements;
					ME = d_METemplates[MEindex];
					wResult += rho*ME*wVal;
				}
			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}
/**********************************************************************/
__global__ void MuKernelNodeBased(int h_nNodes,bool *d_nodeExists, int *d_nodeSignature,int *d_neighborNodes, 
								double *d_MeNode,double *x,int *d_free,double *y)
// Carry out assembly-free M*u on the GPU
// Each thread is assigned to a node
{	
	register int j,node,node2,temp,MEIndex,index,ufree,vfree,wfree;
	register double ME,uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;
	 
	node = blockIdx.x*blockDim.x + threadIdx.x;
	if (node < h_nNodes) {
		temp = d_free[node];//encoded
		wfree = temp>>2;
		vfree = (temp-wfree*4)>>1;
		ufree = (temp-wfree*4-vfree*2);

		if ((ufree || vfree || wfree) && (d_nodeExists[node])){// process only if any of the dof is free
			MEIndex = d_nodeSignature[node];// from 0 to 256

			for (j =0; j < 27; j++) {//
				node2 = d_neighborNodes[j*h_nNodes + node];// process neighbor
				if (node2 == -1)
					continue;

				uVal = x[3*node2];
				vVal = x[3*node2+1];
				wVal = x[3*node2+2];

				index = 81*3*MEIndex + 3*j;

				ME = d_MeNode[index];

				uResult += ME*uVal;
				ME = d_MeNode[index+1];
				uResult += ME*vVal;
				ME = d_MeNode[index+2];
				uResult += ME*wVal;

				ME = d_MeNode[index+81];
				vResult += ME*uVal;
				ME = d_MeNode[index+82];
				vResult += ME*vVal;
				ME = d_MeNode[index+83];
				vResult += ME*wVal;

				ME = d_MeNode[index+162];
				wResult += ME*uVal;
				ME = d_MeNode[index+163];
				wResult += ME*vVal;
				ME = d_MeNode[index+164];
				wResult += ME*wVal;
			}
		}
		// write back the result
		y[3*node] = ufree*uResult;// if ufree = 0 (fixed) then write 0
		y[3*node+1] = vfree*vResult;
		y[3*node+2] = wfree*wResult;
	}
}
/**********************************************************************/
void computeKDiagGPU(GPUData *h_gpuData, double *diag)
{

	int h_nNodes = h_gpuData->h_nNodes;
	int h_nElems = h_gpuData->h_nElems;
	int h_nTemplateElements = h_gpuData->h_nTemplateElements;

	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 

	ComputeDiagKernel<<< dimGrid, dimBlock,0 >>>(h_nNodes,h_nElems,h_nTemplateElements, 
							h_gpuData->d_nNeighboringElems, h_gpuData->d_pseudodensity,  
							h_gpuData->d_neighborElems, h_gpuData->d_templateID,  h_gpuData->d_KETemplates,diag);
	cudaThreadSynchronize();
}
/**********************************************************************/
void vectorDivideGPU(int h_nNodes, double *z,double *x,double *y)
{
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	DivideByVectorKernel<<< dimGrid, dimBlock,0 >>>(h_nNodes,z,x,y);// z = x./y
	cudaThreadSynchronize();
}
/**********************************************************************/
void zeaxpyGPU(int h_nNodes, double *z,double a,double *x,double b, double *y)
{
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	ZEAXPBYKernel<<< dimGrid, dimBlock,0 >>>(h_nNodes,z,a,x,b,y);
	cudaThreadSynchronize();
}
/**********************************************************************/
void KuGPUWithMemoryTransfer(GPUData *h_gpuData, double *hin, double *hout,bool h_nodeBasedAssembly)
{
	int h_nNodes = h_gpuData->h_nNodes;
	int h_nElems = h_gpuData->h_nElems;
	int h_nTemplateElements = h_gpuData->h_nTemplateElements;
	int nDOF;

	nDOF = 3*h_nNodes;
	double *din, *dout;

	cublasAlloc( nDOF, sizeof(double), (void **) &din);	
	cublasAlloc( nDOF, sizeof(double), (void **) &dout);	
	checkCUDAStatus("GPU Memory allocation error in KuGPUWithMemoryTransfer");

	cublasSetVector(nDOF, sizeof(double), hin, 1, din, 1 );
	checkCUDAStatus("GPU Memory copy error in KuGPUWithMemoryTransfer");

	// Carry out K*u, also enforce Dirichlet boundary conditions
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);

	if (h_nodeBasedAssembly) 
		KuKernelNodeBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_gpuData->d_nodeExists, h_gpuData->d_nodeSignature,
								h_gpuData->d_neighborNodes,h_gpuData->d_KeNode,
								din,h_gpuData->d_free,dout);
	else
		KuKernelElemBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_nElems,h_nTemplateElements,h_gpuData->d_nodeExists, h_gpuData->d_nNeighboringElems,
								h_gpuData->d_elems,h_gpuData->d_pseudodensity,h_gpuData->d_neighborElems, 
								h_gpuData->d_templateID,h_gpuData->d_KETemplates,
								din,h_gpuData->d_free,dout);
	cudaThreadSynchronize();

	cublasGetVector( nDOF, sizeof(double), dout, 1, hout, 1 );

	checkCUDAStatus("GPU Get Memory error in KuGPUWithMemoryTransfer");
	cublasFree(din);
	cublasFree(dout);
	checkCUDAStatus("GPU Memory deallocation error in KuGPUWithMemoryTransfer");
}
/**********************************************************************/
void MuGPUWithMemoryTransfer(GPUData *h_gpuData, double *hin, double *hout,bool h_nodeBasedAssembly)
{
	int h_nNodes = h_gpuData->h_nNodes;
	int h_nElems = h_gpuData->h_nElems;
	int h_nTemplateElements = h_gpuData->h_nTemplateElements;
	int nDOF;

	nDOF = 3*h_nNodes;
	double *din, *dout;

	cublasAlloc( nDOF, sizeof(double), (void **) &din);	
	cublasAlloc( nDOF, sizeof(double), (void **) &dout);	
	checkCUDAStatus("GPU Memory allocation error in MuGPUWithMemoryTransfer");

	cublasSetVector(nDOF, sizeof(double), hin, 1, din, 1 );
	checkCUDAStatus("GPU Memory copy error in MuGPUWithMemoryTransfer");

	// Carry out K*u, also enforce Dirichlet boundary conditions
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);

	if (h_nodeBasedAssembly) 
		MuKernelNodeBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_gpuData->d_nodeExists, h_gpuData->d_nodeSignature,
								h_gpuData->d_neighborNodes,h_gpuData->d_MeNode,
								din,h_gpuData->d_free,dout);
	else
		MuKernelElemBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_nElems,h_nTemplateElements,h_gpuData->d_nNeighboringElems,
								h_gpuData->d_elems,h_gpuData->d_pseudodensity,h_gpuData->d_neighborElems, 
								h_gpuData->d_templateID,h_gpuData->d_METemplates,
								din,h_gpuData->d_free,dout);

	cudaThreadSynchronize();

	cublasGetVector( nDOF, sizeof(double), dout, 1, hout, 1 );

	checkCUDAStatus("GPU Get Memory error in MuGPUWithMemoryTransfer");
	cublasFree(din);
	cublasFree(dout);
	checkCUDAStatus("GPU Memory deallocation error in MuGPUWithMemoryTransfer");
}
/**********************************************************************/
void KuGPU(GPUData *h_gpuData, double *din, double *dout,bool h_nodeBasedAssembly)
{
	int h_nNodes = h_gpuData->h_nNodes;
	int h_nElems = h_gpuData->h_nElems;
	int h_nTemplateElements = h_gpuData->h_nTemplateElements;

	// Carry out K*u, also enforce Dirichlet boundary conditions
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);
	//cudaFuncSetCacheConfig(KuKernelElemBased,cudaFuncCachePreferL1);
	// Carry out dout = K*din

	if (h_nodeBasedAssembly) 
		KuKernelNodeBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_gpuData->d_nodeExists, h_gpuData->d_nodeSignature,
								h_gpuData->d_neighborNodes,h_gpuData->d_KeNode,
								din,h_gpuData->d_free,dout);
	else
		KuKernelElemBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_nElems,h_nTemplateElements,h_gpuData->d_nodeExists, h_gpuData->d_nNeighboringElems,
								h_gpuData->d_elems,h_gpuData->d_pseudodensity,h_gpuData->d_neighborElems, 
								h_gpuData->d_templateID,h_gpuData->d_KETemplates,
								din,h_gpuData->d_free,dout);
	cudaThreadSynchronize();
}
/**********************************************************************/
void MuGPU(GPUData *h_gpuData, double *din, double *dout, bool h_nodeBasedAssembly)
{
	int h_nNodes = h_gpuData->h_nNodes;
	int h_nElems = h_gpuData->h_nElems;
	int h_nTemplateElements = h_gpuData->h_nTemplateElements;

	// Carry out M*u, also enforce Dirichlet boundary conditions
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);   	
	// Carry out d_u = M*d_p
	if (h_nodeBasedAssembly)
		MuKernelNodeBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_gpuData->d_nodeExists,h_gpuData->d_nodeSignature,
								h_gpuData->d_neighborNodes,h_gpuData->d_MeNode,
								din,h_gpuData->d_free,dout);
	else
		MuKernelElemBased<<< dimGrid, dimBlock,0 >>> (h_nNodes, h_nElems,h_nTemplateElements, h_gpuData->d_nNeighboringElems,
								h_gpuData->d_elems,h_gpuData->d_pseudodensity,h_gpuData->d_neighborElems, 
								h_gpuData->d_templateID,h_gpuData->d_METemplates,
								din,h_gpuData->d_free,dout);
	cudaThreadSynchronize();
}
/****************************/
int ConjugateGradientOnGPU(GPUData *h_gpuData,int MaxIter, double CGRelResErr, bool h_nodeBasedAssembly)
{
	// Conjugate Gradient method with Jacobi preconditioning
	double rho,alpha,beta,rho_new,normr0,normr;
	int niter,nDOF;
	double *r,*p,*a,*f,*sol;
	Timer timer;

	int h_nNodes = h_gpuData->h_nNodes;
	nDOF = 3*h_nNodes;
	sol = h_gpuData->d_sol;
	f = h_gpuData->d_f;
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: norm(f) = %e \n",cublasDdot(3*h_nNodes,f,1,f,1));
	checkCUDAStatus("GPU Data extraction error in CG");
	cublasAlloc( nDOF, sizeof(double), (void **) &r);	
	cublasAlloc( nDOF, sizeof(double), (void **) &p);	
	cublasAlloc( nDOF, sizeof(double), (void **) &a);	
	checkCUDAStatus("GPU Memory allocation error in CG");
	timer.start();
	KuGPU(h_gpuData,sol,a,h_nodeBasedAssembly);// a = K*u;
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Time taken for one Kx: %f (msec) \n",timer.getElapsedTimeInMilliSec());
	zeaxpyGPU(h_nNodes,r,-1,a,1,f);// r = -a + f
	normr = cublasDdot(3*h_nNodes,r,1,r,1);
	cudaThreadSynchronize();
	normr = sqrt(normr);
	normr0 = normr;

	cublasDcopy(3*h_nNodes,r,1,a,1);// Set a = r 

	if (h_nodeBasedAssembly) {
		if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Starting node-based CG on GPU\n");
		//cublasDcopy(nDOF,r,1,a,1); // a = r
	}
	else {
		if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Starting elem-based CG on GPU\n");
	}
	cublasDcopy(nDOF,a,1,p,1);// Set p = a 
	cudaThreadSynchronize();
	rho = cublasDdot(nDOF,a,1,r,1);// rho = a'*r;
	cudaThreadSynchronize();
	niter = 0;  
	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: normr0 = %e, rho =%e \n",normr0,rho);
	while  ((niter < 10) ||(normr > CGRelResErr*normr0))   {//% Test break condition
		KuGPU(h_gpuData,p,a,h_nodeBasedAssembly);// a = K*p;
		beta = cublasDdot(nDOF,a,1,p,1);//DOTPROD3D(a,p);
		cudaThreadSynchronize();
		alpha = rho/beta;
		zeaxpyGPU(h_nNodes,sol,alpha,p,1,sol);//u = alpha*p + u;		
		zeaxpyGPU(h_nNodes,r,-alpha,a,1,r);	//r = -alpha*a + r ;
		normr = cublasDdot(nDOF,r,1,r,1);
		cudaThreadSynchronize();
		normr = sqrt(normr);
		cublasDcopy(nDOF,r,1,a,1);// Set a= r 

		/*if (h_nodeBasedAssembly)
			cublasDcopy(nDOF,r,1,a,1); // a = r
		else 
			vectorDivideGPU(h_nNodes,d,r,a);// a = r./D */
		cudaThreadSynchronize();
		rho_new =  cublasDdot(nDOF,a,1,r,1);//rho = a'*r
		cudaThreadSynchronize();
		zeaxpyGPU(h_nNodes,p,1,a,(rho_new/rho),p);//p = a + rho_new/rho * p;
		rho = rho_new;
		if (niter == 1) normr0 = normr;//update for consistent scaling convergence
		niter++;
		 
		if ((h_hexmeshGPU_verbose) && (niter%100 == 0)) printf("\n %d: %e ",niter,normr/normr0);
		//if (niter%1000 ==0) printf("\nniter = %d: normr = %e \n",niter,normr/normr0);
		if (niter == MaxIter) {
			//printf("GPU CG: Did not converge in %d iterations! \n",niter);
			//printf("GPU CG: Relative error in residual = %e \n",normr/normr0);
			niter = -1;
			break;
		}
		if (normr > 1000*normr0) {
			//printf("GPU CG: Diverging! \n");
			niter = -1;
			break;
		}
	}
	if (h_hexmeshGPU_verbose) printf("\n %d: %e \n",niter,normr/normr0);
	cublasFree(r);
	cublasFree(p);
	cublasFree(a);
	checkCUDAStatus("GPU Memory deallocation error in CG");
	//printf("AFElasticity: CG on GPU; nIter = %d, resErr = %e \n",niter,normr);
	return niter;
}
/****************************/
int firstEigenmodeOnGPU(GPUData *h_gpuData,int MaxIter, bool h_nodeBasedAssembly, double eigRelativeAccuracy,double *eigenValue,double **eigenMode)
{
	double MNorm,RayleighQuotient,alpha,lambda,lambdaOld,alphaOld,a,b,c,d,ak,bk,ck,disc,delta;
	int i,niter,success;
	int ufree,vfree,wfree,freeEncoded,*freeDOF,nDOF;
	double *x,*Kx,*Mx,*p,*Kp,*Mp,*g,*xCPU;
	Timer timer;
	int h_nNodes = h_gpuData->h_nNodes;  
	nDOF = 3*h_nNodes;
	// use temporary GPU variables
	cublasAlloc( nDOF, sizeof(double), (void **) &x );
	cublasAlloc( nDOF, sizeof(double), (void **) &Kx );
	cublasAlloc( nDOF, sizeof(double), (void **) &Mx );
	cublasAlloc( nDOF, sizeof(double), (void **) &p );
	cublasAlloc( nDOF, sizeof(double), (void **) &Kp );
	cublasAlloc( nDOF, sizeof(double), (void **) &Mp );
	cublasAlloc( nDOF, sizeof(double), (void **) &g );

	checkCUDAStatus("GPU Memory allocation error in firstEigenmode");

	// Create random vector on CPU
	xCPU = (double *)malloc(nDOF*sizeof(double));
	freeDOF = (int *)malloc(h_nNodes*sizeof(int));
	//Get freeDOF from GPU to CPU since we don't have access to this otherwise
	cublasGetVector(h_nNodes, sizeof(int), h_gpuData->d_free, 1, freeDOF, 1 );

	for (i =0; i < h_nNodes; i++) {
		freeEncoded = freeDOF[i];
		wfree = freeEncoded/4;
		vfree = (freeEncoded - wfree*4)/2;
		ufree = (freeEncoded - wfree*4 -vfree*2);

		xCPU[3*i] = 1.0*ufree*rand()/RAND_MAX;// 0 if Dirichlet else random
		xCPU[3*i+1] = 1.0*vfree*rand()/RAND_MAX;
		xCPU[3*i+2] = 1.0*wfree*rand()/RAND_MAX;
	}

	// copy to GPU
	cublasSetVector(nDOF, sizeof(double), xCPU, 1, x, 1 );
	MuGPU(h_gpuData,x,Mx,h_nodeBasedAssembly);// Mx = M*x;
	MNorm = sqrt(cublasDdot(3*h_nNodes,x,1,Mx,1));// MNorm = sqrt(x'*Mx);
	cublasDscal(nDOF,1.0/MNorm,x,1);//x = x/MNorm
	cublasDscal(nDOF,1.0/MNorm,Mx,1);//Mx = Mx/MNorm;
	KuGPU(h_gpuData,x,Kx,h_nodeBasedAssembly);// Kx = K*x;
	RayleighQuotient = cublasDdot(nDOF,x,1,Kx,1);//RayleighQuotient = x'*Kx;
	zeaxpyGPU(h_nNodes,g,2.0,Kx,-2*RayleighQuotient,Mx);//g = 2*(Kx- RayleighQuotient*Mx);
	alpha = cublasDdot(nDOF,g,1,g,1);//alpha = g'*g;
	lambdaOld = 1e20*rand()/RAND_MAX;
	alphaOld = 1e20*rand()/RAND_MAX;
	niter = 1;
	while (1) {
		if ((niter%50 == 0) && (fabs(lambdaOld-lambda)/lambda) < eigRelativeAccuracy)
			break;
		zeaxpyGPU(h_nNodes,p,-1,g,alpha/alphaOld,p);//p = -g + alpha/alphaOld*p; fletcher-reeves algorithm for new direction
		MuGPU(h_gpuData,p,Mp,h_nodeBasedAssembly);// Mp = M*p;
		a = cublasDdot(nDOF,x,1,Mp,1);// a = x'*Mp
		b = cublasDdot(nDOF,Kx,1,p,1);// b = Kx'*p;
		KuGPU(h_gpuData,p,Kp,h_nodeBasedAssembly);// Kp = K*p;
		c = cublasDdot(nDOF,p,1,Kp,1);// c = p'*Kp;
		d = cublasDdot(nDOF,p,1,Mp,1);// d = p'*Mp;
		ak = a*c - b*d;
		bk = c - RayleighQuotient*d;
		ck = b - RayleighQuotient*a;
		disc = sqrt(bk*bk - 4*ak*ck);
		if (bk > 0) delta = -2*ck/(bk + disc);
		else if (ak !=0) delta = (-bk + disc)/(2*ak);
		else delta = -ck/bk;
		zeaxpyGPU(h_nNodes,x,1,x,delta,p);// x = x + delta*p;
		zeaxpyGPU(h_nNodes,Mx,1,Mx,delta,Mp);//Mx = Mx + delta*Mp;
		zeaxpyGPU(h_nNodes,Kx,1,Kx,delta,Kp);//Kx = Kx + delta*Kp
		MNorm = sqrt(cublasDdot(3*h_nNodes,x,1,Mx,1));// MNorm = sqrt(x'*Mx);
		cublasDscal(nDOF,1.0/MNorm,x,1);//x = x/MNorm;
		cublasDscal(nDOF,1.0/MNorm,Mx,1);//Mx = Mx/MNorm;
		cublasDscal(nDOF,1.0/MNorm,Kx,1);//Kx = Kx/MNorm;
		if (niter%50 == 1) lambdaOld = lambda;// update every 50 iterations
		RayleighQuotient = cublasDdot(nDOF,x,1,Kx,1);//RayleighQuotient = x'*Kx;
		lambda = sqrt(RayleighQuotient)/(2*3.1415);
		zeaxpyGPU(h_nNodes,g,2.0,Kx,-2*RayleighQuotient,Mx);//g = 2*(Kx- RayleighQuotient*Mx);
		alphaOld = alpha;
		alpha = cublasDdot(nDOF,g,1,g,1);// alpha = g'*g;
		niter++;
		if (niter%500 == 0) 
			printf("nIter = %d, lambda = %0.3e (Hz) \n",niter,sqrt(RayleighQuotient)/(2*3.1415));
		//printf("nIter = %d, lambda = %0.3e (Hz) \n",niter,sqrt(RayleighQuotient)/(2*3.1415));
		if (niter == MaxIter)  {  
			printf("AFElasticity: RayleighCG in GPU did not converge in %d iter \n",niter);
			printf("AFElasticity: Estimated PercentErr in eigen frequency: %0.3f \n",100*fabs(lambdaOld-lambda)/lambda);
			break;
		}
	}
	success = niter;
	//printf("nIter = %d, f = %0.3e  (Hz) \n",niter,lambda);
	eigenValue[0] = lambda;
	cublasGetVector(nDOF, sizeof(double), x, 1, xCPU, 1 );
	for (i = 0; i < h_nNodes; i++) {
		eigenMode[0][3*i  ] = xCPU[3*i];
		eigenMode[0][3*i+1] = xCPU[3*i+1];
		eigenMode[0][3*i+2] = xCPU[3*i+2];
	}
	cublasFree(x);
	cublasFree(Kx);
	cublasFree(Mx);
	cublasFree(p);
	cublasFree(Kp);
	cublasFree(Mp);
	cublasFree(g);

	//free CPU memory
	free(xCPU);
	free(freeDOF);

	return success;
}

/****************************/
int RayleighCGOnGPU(GPUData *h_gpuData,int nEigenModes, double *eigenValues,double **eigenModes,int useInitialGuess,
					double relResidual,int maxIterPerMode, bool h_nodeBasedAssembly)
{
	double MNorm,omega,alpha,alphaOld,a,b,c,d,ak,bk,ck,disc,delta;
	int i,niter,nDOF,totalIter;
	int ufree,vfree,wfree,freeEncoded,*freeDOF,currentMode;
	double *x,*Kx,*Mx,*p,*Kp,*Mp,*g,*xCPU,*(temp[MAX_EIGEN_MODES]),*(eigModesGPU[MAX_EIGEN_MODES]),s,normg0;
	Timer timer;

	
	int nNodes = h_gpuData->h_nNodes;
	nDOF = 3*nNodes;
	totalIter = 0;
	// use temporary GPU variables
	cublasAlloc( nDOF, sizeof(double), (void **) &x );
	cublasAlloc( nDOF, sizeof(double), (void **) &Kx );
	cublasAlloc( nDOF, sizeof(double), (void **) &Mx );
	cublasAlloc( nDOF, sizeof(double), (void **) &p );
	cublasAlloc( nDOF, sizeof(double), (void **) &Kp );
	cublasAlloc( nDOF, sizeof(double), (void **) &Mp );
	cublasAlloc( nDOF, sizeof(double), (void **) &g );
	
	for (i =0; i < nEigenModes; i++) {
		cublasAlloc( nDOF, sizeof(double), (void **) &(temp[i]) );
		cublasAlloc( nDOF, sizeof(double), (void **) &(eigModesGPU[i]) );
	}
	checkCUDAStatus("GPU Memory allocation error in RayleighCGOnGPU");
	xCPU = (double *)malloc(nDOF*sizeof(double));
	freeDOF = (int *)malloc(nNodes*sizeof(int));
	for (currentMode = 0; currentMode < nEigenModes; currentMode++) {
		if (useInitialGuess) {
			cublasSetVector(nDOF, sizeof(double), eigenModes[currentMode], 1, x, 1 );
		}
		else {	
			//Get freeDOF from GPU to CPU since we don't have access to this otherwise
			cublasGetVector(nNodes, sizeof(int), h_gpuData->d_free, 1, freeDOF, 1 );
			// Create random vector on CPU
			for (i =0; i < nNodes; i++) {
				freeEncoded = freeDOF[i];
				wfree = freeEncoded/4;
				vfree = (freeEncoded - wfree*4)/2;
				ufree = (freeEncoded - wfree*4 -vfree*2);

				xCPU[3*i] = 1.0*ufree*rand()/RAND_MAX;// 0 if Dirichlet else random
				xCPU[3*i+1] = 1.0*vfree*rand()/RAND_MAX;
				xCPU[3*i+2] = 1.0*wfree*rand()/RAND_MAX;
			}
			// copy random vector to GPU
			cublasSetVector(nDOF, sizeof(double), xCPU, 1, x, 1 );
		}
		// M-orthogonalize with respect all previous eigen vectors
		for (int n = 0; n < currentMode; n++) {
			cublasSetVector(nDOF, sizeof(double), eigenModes[n], 1, eigModesGPU[n], 1 );
			MuGPU(h_gpuData,eigModesGPU[n],temp[n],h_nodeBasedAssembly);// temp = M*eigMode
			s = cublasDdot(nDOF,x,1,temp[n],1);// s = x'*temp
			zeaxpyGPU(nNodes,x,1,x,-s,eigModesGPU[n]);// x = x - s*eigmode;
		}
		// now x is orthogonal to all previous eigenmodes
		MuGPU(h_gpuData,x,Mx,h_nodeBasedAssembly);// Mx = M*x;
		MNorm = sqrt(cublasDdot(nDOF,x,1,Mx,1));// MNorm = sqrt(x'*Mx);
		cublasDscal(nDOF,1.0/MNorm,x,1);//x = x/MNorm
		cublasDscal(nDOF,1.0/MNorm,Mx,1);//Mx = Mx/MNorm;
		KuGPU(h_gpuData,x,Kx,h_nodeBasedAssembly);// Kx = K*x;
		omega = cublasDdot(nDOF,x,1,Kx,1);//omega = x'*Kx;
		zeaxpyGPU(nNodes,g,2.0,Kx,-2*omega,Mx);//g = 2*(Kx- omega*Mx);
		alpha = cublasDdot(nDOF,g,1,g,1);//alpha = g'*g;
		normg0 = alpha;
		alphaOld = 1e20*rand()/RAND_MAX;
		niter = 1;

		for (i =0; i <nDOF; i++) xCPU[i] = 0.0;
		cublasSetVector(nDOF, sizeof(double), xCPU, 1, p, 1 );
		while (1) {
			zeaxpyGPU(nNodes,p,-1,g,alpha/alphaOld,p);//p = -g + alpha/alphaOld*p; fletcher-reeves algorithm for new direction	
			// M-orthogonalize with respect all previous eigen vectors
			for (int n = 0; n < currentMode; n++) {
				s = cublasDdot(nDOF,p,1,temp[n],1);// s = p'*temp
				zeaxpyGPU(nNodes,p,1,p,-s,eigModesGPU[n]);// p = p - s*eigmode;
			}
			// now p is orthogonal to all previous eigenmodes
			MuGPU(h_gpuData,p,Mp,h_nodeBasedAssembly);// Mp = M*p;
			a = cublasDdot(nDOF,x,1,Mp,1);// a = x'*Mp
			b = cublasDdot(nDOF,Kx,1,p,1);// b = Kx'*p;
			KuGPU(h_gpuData,p,Kp,h_nodeBasedAssembly);// Kp = K*p;
			c = cublasDdot(nDOF,p,1,Kp,1);// c = p'*Kp;
			d = cublasDdot(nDOF,p,1,Mp,1);// d = p'*Mp;
			ak = a*c - b*d;
			bk = c - omega*d;
			ck = b - omega*a;

			disc = sqrt(bk*bk - 4*ak*ck);
			if (bk > 0) delta = -2*ck/(bk + disc);
			else if (ak !=0) delta = (-bk + disc)/(2*ak);
			else delta = -ck/bk;

			zeaxpyGPU(nNodes,x,1,x,delta,p);// x = x + delta*p;
			zeaxpyGPU(nNodes,Mx,1,Mx,delta,Mp);//Mx = Mx + delta*Mp;
			zeaxpyGPU(nNodes,Kx,1,Kx,delta,Kp);//Kx = Kx + delta*Kp
			MNorm = sqrt(cublasDdot(nDOF,x,1,Mx,1));// MNorm = sqrt(x'*Mx);
			cublasDscal(nDOF,1.0/MNorm,x,1);//x = x/MNorm;
			cublasDscal(nDOF,1.0/MNorm,Mx,1);//Mx = Mx/MNorm;
			cublasDscal(nDOF,1.0/MNorm,Kx,1);//Kx = Kx/MNorm;

			omega = cublasDdot(nDOF,x,1,Kx,1);//RayleighQuotient = x'*Kx;
			zeaxpyGPU(nNodes,g,2.0,Kx,-2*omega,Mx);//g = 2*(Kx- RayleighQuotient*Mx);
			alphaOld = alpha;
			alpha = cublasDdot(nDOF,g,1,g,1);// alpha = g'*g;
			niter++;
			totalIter++;
			//if (totalIter%2000 == 0)  printf("nIter = %d, lambda = %0.3e (Hz) \n",totalIter,sqrt(omega)/(2*3.1415));
			if ((niter > maxIterPerMode) || ((alpha/normg0) < relResidual))
				break;
		}
		eigenValues[currentMode] = omega;
		cublasGetVector(nDOF, sizeof(double), x, 1, eigenModes[currentMode], 1 );
		if (relResidual < 1e-5) printf("Mode %d: %f (Hz); niter = %d \n",currentMode+1,sqrt(omega)/(2*3.1415),niter);

	}
	cublasFree(x);
	cublasFree(Kx);
	cublasFree(Mx);
	cublasFree(p);
	cublasFree(Kp);
	cublasFree(Mp);
	cublasFree(g);
	for (i =0; i < nEigenModes; i++)  {
		cublasFree(temp[i]);
		cublasFree(eigModesGPU[i]);
	}
	checkCUDAStatus("GPU Memory deallocation error in RayleighCGOnGPU");

	//free CPU memory
	free(xCPU);
	free(freeDOF);

	return totalIter;
}
/**********************************************************************/
void transferSolutionFromGPU(GPUData *h_gpuData, double *h_sol)
{
	cublasGetVector( 3*h_gpuData->h_nNodes, sizeof(double), h_gpuData->d_sol, 1, h_sol, 1 );
}
/**********************************************************************/
__device__ int GPUfindNumCommonElements(int n, int *v1, int *v2)
{
    int nCommon = 0,i,k;
    for (i = 0; i < n; i++) {
        for (k =0; k < n; k++) {
            if (v1[i] == v2[k])
                  nCommon++;
		}
    }
    return (nCommon);
}
/**********************************************************************/
__global__ void GPUfindTriNeighbors(int * nTri,int * devtriNodes,int * devtriNeighbors)
{
	/* 1.Did away with global nNeighbors[] array as it was the bottleneck.
	   2.Instead I declared nNeighbors as local variable as they are stored in registers, which is fastest out of all types of memory.
	   3.Due to fact(2), the pruning needs to be changed slightly. 
	   4.Shifted data here and there and tried all possible combinations of memory storage and this was the most optimised combo. shared memory is cached.
	   5.Can't use constant memory as MAX_TRIANGLES is 25000. sizeof(unsigned short int)* 25000*3 >>64KB . So idea = flop.
	   6.It turns out that the code runs quicker if I remove conditionals like "if(tid<n&&tid!=i)" and the like, so I did.[Im guessing threads are waiting at __syncthreads(); performing NOP] 
	   */

	int tid=blockIdx.x*blockDim.x+threadIdx.x;
	int n=(*nTri);
	int i,j,nCommonNodes,nNeighbors=0;
	__shared__ int striNeighbors[3*THREADS];
	__shared__ int striNodes[3*THREADS];
	int pts[3];
		
	for(i=0;i<3;i++){
		striNeighbors[threadIdx.x*3+i]=-1;
		striNodes[threadIdx.x*3+i]=devtriNodes[tid*3+i];
	}
					
	for(i=0;i<n;i++){
		if(nNeighbors==3)//  Note the "break" instead of continue
			break;
		for(j=0;j<3;j++)
			pts[j]=devtriNodes[i*3+j];
			nCommonNodes = GPUfindNumCommonElements(3,&(pts[0]),&(striNodes[threadIdx.x*3]));
			if(nCommonNodes==2){
				striNeighbors[threadIdx.x*3+nNeighbors]=i;
				nNeighbors++;
			}			
		__syncthreads();
	}
	__syncthreads();
	for(i=0;i<3;i++)
		devtriNeighbors[tid*3+i]=striNeighbors[threadIdx.x*3+i];
}
/**********************************************************************/
void findTriNeighborsGPU(int nTriangles,int *triNodes,int *triNeighbors)
/* compute the neighbors of triangles
Input: triNodes: (n1,n2,n3,...) 3 nodes for each triangle, followed by next triangle
Output: triNeighbors: (t1,t2,t3,...)3 neighbors for each triangle, followed by next triangle
*/
{
	int *devtriNodes;
	int *devtriNeighbors;
	int *nTri;
	int gridSize;
	gridSize= (int) ceil(1.0*nTriangles/THREADS);
	
	cudaMalloc((void**)&devtriNodes,sizeof(int)*3* nTriangles);
	cudaMalloc((void**)&devtriNeighbors,sizeof(int)*3* nTriangles);
	cudaMalloc((void**)&nTri,sizeof(int));
	

	cudaMemcpy(devtriNodes,triNodes,sizeof(int)*3* nTriangles,cudaMemcpyHostToDevice);
    cudaMemcpy(nTri,&nTriangles,sizeof(int),cudaMemcpyHostToDevice);
	
	GPUfindTriNeighbors<<<gridSize,THREADS>>>(nTri,devtriNodes,devtriNeighbors);
	
	cudaMemcpy(triNeighbors,devtriNeighbors,sizeof(int)*3* nTriangles,cudaMemcpyDeviceToHost);		 
	cudaFree(devtriNeighbors);
	cudaFree(devtriNodes);
}
/**********************************************************************/
__device__ int rayIntersectTriGPU(double orig[3], double dir[3],
			double vert0[3], double vert1[3], double vert2[3],
			double *t, double *u, double *v)
{
	// Ray-triangle intersection

   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUBTRACT3D(edge1, vert1, vert0);
   SUBTRACT3D(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS3D(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOTPROD3D(edge1, pvec);

   /* calculate distance from vert0 to ray origin */
   SUBTRACT3D(tvec, orig, vert0);
   inv_det = 1.0 / det;

   if (det > TOL)
   {
	   /* calculate U parameter and test bounds */
	   *u = DOTPROD3D(tvec, pvec);
	   if (*u < 0.0 || *u > det)
		   return 0;
	   /* prepare to test V parameter */
	   CROSS3D(qvec, tvec, edge1);

	   /* calculate V parameter and test bounds */
	   *v = DOTPROD3D(dir, qvec);
	   if (*v < 0.0 || *u + *v > det)
		   return 0;
   }
   else if(det < -TOL)
   {
	   /* calculate U parameter and test bounds */
	   *u = DOTPROD3D(tvec, pvec);
	   if (*u > 0.0 || *u < det)
		   return 0;
	   /* prepare to test V parameter */
	   CROSS3D(qvec, tvec, edge1);

	   /* calculate V parameter and test bounds */
	   *v = DOTPROD3D(dir, qvec) ;
	   if (*v > 0.0 || *u + *v < det)
		   return 0;
   }
   else {
	   return 0;  /* ray is parallell to the plane of the triangle */
   }
   /* calculate t, ray intersects triangle */
   *t = DOTPROD3D(edge2, qvec) * inv_det;
   (*u) *= inv_det;
   (*v) *= inv_det;

   return 1;
}
/**********************************************************************/
void releaseHexmeshGPUMemory(GPUData *h_gpuData)
{
	//cublasShutdown();
	// GPU memory deallocation 
	if ((h_gpuData == NULL) || (h_gpuData->h_nNodes == 0))
		return;

	if (h_hexmeshGPU_verbose) printf("AFElasticityGPU: Releasing GPU memory\n");
	cublasFree(h_gpuData->d_elems);
	cublasFree(h_gpuData->d_pseudodensity);
	cublasFree(h_gpuData->d_neighborElems);
	cublasFree(h_gpuData->d_sol);;
	cublasFree(h_gpuData->d_free);
	cublasFree(h_gpuData->d_nodeSignature);
	cublasFree(h_gpuData->d_neighborNodes);
	cublasFree(h_gpuData->d_f);
	cublasFree(h_gpuData->d_KETemplates);
	cublasFree(h_gpuData->d_METemplates);
	cublasFree(h_gpuData->d_KeNode);
	h_gpuData->h_nNodes =0;
}
#include "WorkSpaceMethodGPU.cu"

#endif


