#include <stdio.h>
#include <math.h>
#include <cuda.h>
#include "../inc/hexmesh/Timer.h"
#include "../inc/hexmesh/AFPlasticityGPUDefines.h"
#include "cublas.h"
#include <string.h>
#include <stdlib.h>
/**********************************************************************/
#ifdef COMPILE_GPUCODE
/**********************************************************************/
void dumpGPUInfo_plasticity()
{
        size_t free, total;
        int gpuCount, i;
        CUresult res;
        CUdevice dev;
        CUcontext ctx;
        cudaDeviceProp devProp;


        cuInit(0);

        cuDeviceGetCount(&gpuCount);
        printf("AFPlasticityGPU: Detected %d GPU\n",gpuCount);

        for (i=0; i<gpuCount; i++) {
                cuDeviceGet(&dev,i);
                cuCtxCreate(&ctx, 0, dev);
                res = cuMemGetInfo(&free, &total);
                if(res != CUDA_SUCCESS)
                        printf("AFPlasticityGPU: cuMemGetInfo failed! (%x)", res);
                printf("AFPlasticityGPU: Device: %d\n",i);
        cudaGetDeviceProperties(&devProp, i);
                printf("AFPlasticityGPU: Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
                printf("AFPlasticityGPU: Major revision number:         %d\n", devProp.major);
                printf("AFPlasticityGPU: Minor revision number:         %d\n", devProp.minor);
                printf("AFPlasticityGPU: Name:                          %s\n", devProp.name);
                printf("AFPlasticityGPU: Total global memory:           %u\n", devProp.totalGlobalMem);
                printf("AFPlasticityGPU: Total shared memory per block: %u\n", devProp.sharedMemPerBlock);
                printf("AFPlasticityGPU: Total registers per block:     %d\n", devProp.regsPerBlock);
                printf("AFPlasticityGPU: Warp size:                     %d\n", devProp.warpSize);
                printf("AFPlasticityGPU: Maximum memory pitch:          %u\n", devProp.memPitch);
                printf("AFPlasticityGPU: Maximum threads per block:     %d\n", devProp.maxThreadsPerBlock);
                for (int i = 0; i < 3; ++i)
                        printf("AFPlasticityGPU: Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
                for (int i = 0; i < 3; ++i)
                        printf("AFPlasticityGPU: Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
                printf("AFPlasticityGPU: Clock rate:                    %d\n", devProp.clockRate);
                printf("AFPlasticityGPU: Total constant memory:         %u\n", devProp.totalConstMem);
                printf("AFPlasticityGPU: Texture alignment:             %u\n", devProp.textureAlignment);
                printf("AFPlasticityGPU: Concurrent copy and execution: %s\n", (devProp.deviceOverlap ? "Yes" : "No"));
                printf("AFPlasticityGPU: Number of multiprocessors:     %d\n", devProp.multiProcessorCount);
                printf("AFPlasticityGPU: Kernel execution timeout:      %s\n", (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
                printf("AFPlasticityGPU: Free :%lu MB\n", (free/1024/1024));
                cuCtxDetach(ctx);
   }
}
/**********************************************************************/
__global__ void ZEAXPBYKernel_plasticity(int p_nNodes,double *z,double a,double *x,double b, double *y)

// z = a*x + b*y
// Each thread is assigned to a node
{
    register int node;
    node = blockIdx.x*blockDim.x + threadIdx.x;
    if (node < p_nNodes) {
        z[3*node] = a*x[3*node] + b*y[3*node];
        z[3*node+1] = a*x[3*node+1] + b*y[3*node+1];
        z[3*node+2] = a*x[3*node+2] + b*y[3*node+2];
    }
}
/**********************************************************************/
__global__ void DivideByVectorKernel_plasticity(int p_nNodes,double *d, double *r,double *a)
// a = r./d
// Each thread is assigned to a node
{
    register int node;

    node = blockIdx.x*blockDim.x + threadIdx.x;

    if (node < p_nNodes) {
        a[3*node] = r[3*node]/d[3*node];
        a[3*node+1] = r[3*node+1]/d[3*node+1];
        a[3*node+2] = r[3*node+2]/d[3*node+2];
    }
}
/**********************************************************************/
__global__ void KEuKernelElemBased_plasticity(int p_nNodes,int p_nElems, int *d_nNeighboringElems,
                                  int *d_elems, int *d_neighborElems, double *d_KETemplates,
                                  double *x,int *d_free,double *y)
// Carry out assembly-free K*u on the GPU
// Each thread is assigned to a node
{
    register int i,j,node,node2,temp, elem,KEindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree;
    register double KE, uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;

    node = blockIdx.x*blockDim.x + threadIdx.x;

    if (node < p_nNodes) {
        temp = d_free[node];//encoded
        wfree = temp>>2;
        vfree = (temp-wfree*4)>>1;
        ufree = (temp-wfree*4-vfree*2);

        if ( (ufree || vfree || wfree)) {// process only if any of the dof is free
            maxElem = d_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElemNum = d_neighborElems[(p_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElemNum - nodeIndex*MAXELEMS;

                for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
                    node2 = d_elems[(p_nElems)*i + elem];
                    uVal = x[3*node2];
                    vVal = x[3*node2+1];
                    wVal = x[3*node2+2];

                    KEindex = (24*3*nodeIndex + 3*i) ;
                    KE = d_KETemplates[KEindex];
                    uResult += KE*uVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    uResult +=  KE*vVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    uResult += KE*wVal;

                    //KEindex = (24*3*nodeIndex + 24+3*i)*(p_nTemplateElements) + elemID;
                    KEindex += 22;
                    KE = d_KETemplates[KEindex];
                    vResult += KE*uVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    vResult += KE*vVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    vResult += KE*wVal;

                    //KEindex = (24*3*nodeIndex + 48+3*i)*(p_nTemplateElements) + elemID;
                    KEindex += 22;
                    KE = d_KETemplates[KEindex];
                    wResult += KE*uVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    wResult += KE*vVal;

                    KEindex += 1;
                    KE = d_KETemplates[KEindex];
                    wResult += KE*wVal;
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
__global__ void KPuKernelElemBased(int p_nNodes,int p_nElems,bool *p_elemPlasticState,
                                   int *d_nNeighboringElems, int *d_elems, int *d_neighborElems, double *KPlastic,
                                  int * p_plasticElem, double *x, int *p_free, double *y)
{

    int i,j,node,node2,temp, elem,KPindex,maxElem,nodeIndex,codedElemNum,ufree,vfree,wfree;
    double KP, uResult=0.0,vResult=0.0,wResult=0.0,uVal,vVal,wVal;    
    double p_KPlastic[576];
    node = blockIdx.x*blockDim.x + threadIdx.x;

    if (node < p_nNodes) {        
        temp = p_free[node];//encoded
        wfree = temp>>2;
        vfree = (temp-wfree*4)>>1;
        ufree = (temp-wfree*4-vfree*2);

        if ( (ufree || vfree || wfree)) {// process only if any of the dof is free
            maxElem = d_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElemNum = d_neighborElems[(p_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElemNum/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElemNum - nodeIndex*MAXELEMS;

                if (!p_elemPlasticState[elem]) continue;
                int k= p_plasticElem[elem];
                for (int ii=0; ii<576; ii++)
                p_KPlastic[ii] = KPlastic[ii + k*576];

                for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
                    node2 = d_elems[(p_nElems)*i + elem];
                    uVal = x[3*node2];
                    vVal = x[3*node2+1];
                    wVal = x[3*node2+2];

                    KPindex = (24*3*nodeIndex + 3*i) ;
                    KP = p_KPlastic[KPindex];
                    uResult += KP*uVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    uResult +=  KP*vVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    uResult += KP*wVal;

                    KPindex += 22;
                    KP = p_KPlastic[KPindex];
                    vResult += KP*uVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    vResult += KP*vVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    vResult += KP*wVal;

                    KPindex += 22;
                    KP = p_KPlastic[KPindex];
                    wResult += KP*uVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    wResult += KP*vVal;

                    KPindex += 1;
                    KP = p_KPlastic[KPindex];
                    wResult += KP*wVal;
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
/*************************************************************************/
/********************** Deflation   **************************************/
__global__ void enforceDirichletKernel_plasticity(int h_nNodes, double *t, int *d_free)
{
    register int node,temp,ufree,vfree,wfree;

    node = blockIdx.x*blockDim.x + threadIdx.x;

    if (node < h_nNodes) {
        temp = d_free[node];//encoded
        wfree = temp>>2;
        vfree = (temp-wfree*4)>>1;
        ufree = (temp-wfree*4-vfree*2);
        t[3*node] *= ufree;// if ufree = 0 (fixed) then write 0
        t[3*node+1] *= vfree;
        t[3*node+2] *= wfree;
    }
}
/*************************************************************************/
__global__ void permuteVectorKernel_plasticity(int N, double *PVector,double *x, double *y)
{
    register int row,row_x;

    row = blockIdx.x*blockDim.x + threadIdx.x;

    if (row < N) {
        row_x = PVector[row];
        y[row] = x[row_x];
    }
}
/*************************************************************************/
__global__ void permuteTransposeVectorKernel_plasticity(int N, double *PVector,double *x, double *y)
{
    register int row,row_x;

    row = blockIdx.x*blockDim.x + threadIdx.x;

    if (row < N) {
        row_x = PVector[row];
        y[row_x] = x[row];
    }
}
/*************************************************************************/
__global__ void getWorkSpaceVectorKernel0_plasticity(int d_nGroups, int *d_groupKey,double *outVector, double *inVector,
                                          double *d_x, double *d_y, double *d_z, double *d_CGPatch,
                                          int *d_groupNodes)
                                          // mu = W'*r
                                          // Each thread is assigned to a node
{
    register int node, group, start = 0, stop, index;
    register double gX, gY, gZ, xnode, ynode, znode,
        u = 0, v = 0, w = 0, uY = 0, uZ = 0,
        vX = 0, vZ = 0, wX = 0, wY = 0;

    group = blockIdx.x*blockDim.x + threadIdx.x;
    if (group < d_nGroups) {
        gX = d_CGPatch[group*3];
        gY = d_CGPatch[group*3 + 1];
        gZ = d_CGPatch[group*3 + 2];

        if (group != 0) start = d_groupKey[group - 1];
        stop = d_groupKey[group];

        for (index = start; index < stop; index ++) {
            node = d_groupNodes[index];

                xnode = (d_x[node] - gX);
                ynode = (d_y[node] - gY);
                znode = (d_z[node] - gZ);

                u += *(inVector + 3*node);
                uY += *(inVector + 3*node) * znode;
                uZ -= *(inVector + 3*node) * ynode;

                v += *(inVector + 3*node + 1) ;
                vX -= *(inVector + 3*node + 1) * znode;
                vZ += *(inVector + 3*node + 1) * xnode;

                w += *(inVector + 3*node + 2) ;
                wX += *(inVector + 3*node + 2) * ynode;
                wY -= *(inVector + 3*node + 2) * xnode;

        }
        __syncthreads();
        outVector[6*group] += u;
        outVector[6*group + 1] += v;
        outVector[6*group + 2] += w;
        outVector[6*group + 3] += vX + wX;
        outVector[6*group + 4] += uY + wY;
        outVector[6*group + 5] += uZ + vZ;
    }
}

/***********************************************************************/
__global__ void getWorkSpaceVectorKernel1_plasticity(int d_nGroups, int *d_groupKey,double *outVector, double *inVector,
                                        double *d_x, double *d_y, double *d_z, double *d_CGPatch,
                                        int *d_groupNodes)
// mu = W'*r
// Each thread is assigned to a node
{
    register int node, group, start = 0, stop, index;
    register double gX, gY, gZ, xnode, ynode, znode, u, v, w,
                    u0 = 0, u1 = 0, u2 = 0, u3 = 0,
                    u4 = 0, u5 = 0,	u6 = 0, u7 = 0, u8 = 0;

    group = blockIdx.x*blockDim.x + threadIdx.x;

    if (group < d_nGroups){
        gX = d_CGPatch[group*3];
        gY = d_CGPatch[group*3 + 1];
        gZ = d_CGPatch[group*3 + 2];

        if (group != 0) start = d_groupKey[group - 1];
        stop = d_groupKey[group];

        for (index = start; index < stop; index ++){
            node = d_groupNodes[index];

                xnode = (d_x[node] - gX);
                ynode = (d_y[node] - gY);
                znode = (d_z[node] - gZ);
                u = *(inVector + 3*node);
                v = *(inVector + 3*node + 1);
                w = *(inVector + 3*node + 2);

                u0	+= u;
                u1	+= v;
                u2	+= w;
                u3	+= w * ynode - v * znode;
                u4	+= u * znode - w * xnode;
                u5	+= v * xnode - u * ynode;
                u6	+= (w * xnode/2 - u * znode) * xnode;
                u7	+= (w * ynode/2 - v * znode) * ynode;
                u8	+= w * xnode * ynode - v * znode * xnode
                            - u * znode * ynode;

        }
        __syncthreads();
        outVector[9*group]		+= u0;
        outVector[9*group + 1]	+= u1;
        outVector[9*group + 2]	+= u2;
        outVector[9*group + 3]	+= u3;
        outVector[9*group + 4]	+= u4;
        outVector[9*group + 5]	+= u5;
        outVector[9*group + 6]	+= u6;
        outVector[9*group + 7]	+= u7;
        outVector[9*group + 8]	+= u8;
    }
}
/*************************************************************************/
__global__ void getSolutionSpaceVectorKernel0_plasticity(int h_nNodes,double *outVector, double *inVector,
                                            double *d_x, double *d_y, double *d_z, double *d_CGPatch,
                                            int *d_nodeGroupNumber)
// r = W*mu
// Each thread is assigned to a node
{
    register int node, group;
    register double xnode,ynode,znode;

    node = blockIdx.x*blockDim.x + threadIdx.x;
    if ((node < h_nNodes)){
        group = d_nodeGroupNumber[node];

        xnode = (d_x[node] - d_CGPatch[group*3]);
        ynode = (d_y[node] - d_CGPatch[group*3 + 1]);
        znode = (d_z[node] - d_CGPatch[group*3 + 2]);

        outVector[3*node] = inVector[6*group] + inVector[6*group + 4] * znode
                                    - inVector[6*group + 5] * ynode;// check indexing for coordinates

        outVector[3*node + 1] = inVector[6*group + 1] - inVector[6*group + 3] * znode
                                    + inVector[6*group + 5] * xnode;// check indexing for coordinates

        outVector[3*node + 2] = inVector[6*group + 2] + inVector[6*group + 3] * ynode
                                    - inVector[6*group + 4] * xnode;// check indexing for coordinates
    }
}


/*************************************************************************/
__global__ void getSolutionSpaceVectorKernel1_plasticity(int h_nNodes,double *outVector, double *inVector,
                                            double *d_x, double *d_y, double *d_z, double *d_CGPatch,
                                            int *d_nodeGroupNumber)
// r = W*mu
// Each thread is assigned to a node
{
    register int node, group;
    register double xnode,ynode,znode, u0, u1, u2, u3, u4, u5, u6, u7, u8;

    node = blockIdx.x*blockDim.x + threadIdx.x;
    if ((node < h_nNodes)){
        group = d_nodeGroupNumber[node];

        xnode = (d_x[node] - d_CGPatch[group*3]);
        ynode = (d_y[node] - d_CGPatch[group*3 + 1]);
        znode = (d_z[node] - d_CGPatch[group*3 + 2]);
        u0 = inVector[9*group]; u1 = inVector[9*group + 1]; u2 = inVector[9*group + 2];
        u3 = inVector[9*group + 3]; u4 = inVector[9*group + 4]; u5 = inVector[9*group + 5];
        u6 = inVector[9*group + 6]; u7 = inVector[9*group + 7]; u8 = inVector[9*group + 8];

        outVector[3*node] = u0 + u4 * znode - u5 * ynode
                            - u6 * znode * xnode - u8 * znode * ynode;

        outVector[3*node + 1] = u1 - u3 * znode + u5 * xnode
                                - u7 * znode * ynode - u8 * znode * xnode;

        outVector[3*node + 2] = u2 + u3 * ynode - u4 * xnode
                                + u6 * xnode * xnode/2 + u7 * ynode * ynode/2
                                + u8 * xnode * ynode;
    }
}
/*************************************************************************/
/*************************************************************************/

// Simple utility function to check for CUDA runtime errors
bool checkCUDAStatus_plasticity(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) {
        fprintf(stderr, "%s: %s.\n", msg,cudaGetErrorString( err) );
        return false;
    }
    return true;
}
/**********************************************************************/
int isThereValidCudaDevice_plasticity(void)
{
    int deviceCount,dev = 0;
        cudaDeviceProp deviceProp;
        cuInit(0);
        dumpGPUInfo_plasticity();
    cuDeviceGetCount(&deviceCount);
        if (deviceCount == 0)
                return -1;

    cudaGetDeviceProperties(&deviceProp, dev);

        if (deviceProp.major == 9999 && deviceProp.minor == 9999)   return -1;
        if (deviceProp.major == 1 && deviceProp.minor < 3) 	return -1;
        cudaFree(0);
        cublasStatus status = cublasInit();// for Cublas Vector operations

        if (status != CUBLAS_STATUS_SUCCESS) {
                printf("AFPlasticityGPU: Cublas Init failure\n");
                return -1;
        }
        cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
        return 1;
}

/**********************************************************************/
bool createAFPlasticityGPUMemory(int p_nNodes,int p_nElems, int p_numPlasticElems, int p_maxNeighboringElements, p_GPUData *p_gpuData)
{
    size_t freeInitial, freeFinal, total;

    if (p_nNodes == 0) return false;
    cuMemGetInfo(&freeInitial, &total);
    p_gpuData->p_nNodes = p_nNodes;
    p_gpuData->p_nElems = p_nElems;
    p_gpuData->p_numPlasticElems=p_numPlasticElems;
    p_gpuData->p_maxNeighboringElements = p_maxNeighboringElements;   

    // GPU Memory allocation
    cublasAlloc( 8*(p_nElems), sizeof(int), (void **) &(p_gpuData->p_elems) );
    cublasAlloc( p_nElems, sizeof(int), (void **) &(p_gpuData->p_plasticElem) );

    cublasAlloc( p_maxNeighboringElements*(p_nNodes), sizeof(int), (void **) &(p_gpuData->p_neighborElems) );
    cublasAlloc( (p_nNodes), sizeof(int), (void **) &(p_gpuData->p_nNeighboringElems) );
    cublasAlloc( (p_nNodes), sizeof(int), (void **) &(p_gpuData->p_free) );

    cublasAlloc( (p_nNodes), sizeof(int), (void **) &(p_gpuData->p_nodeSignature) );
    cublasAlloc( (27*p_nNodes), sizeof(int), (void **) &(p_gpuData->p_neighborNodes) );

    cublasAlloc( (3*p_nNodes), sizeof(double), (void **) &(p_gpuData->p_f));			// force
    cublasAlloc( (3*p_nNodes), sizeof(double), (void **) &(p_gpuData->p_sol) );			//  displacement solution
    cublasAlloc( 576, sizeof(double), (void **) &(p_gpuData->p_KETemplates) );	// Ke templates

    cublasAlloc( p_nElems, sizeof(bool), (void **) &(p_gpuData->p_elemPlasticState) );

    cublasAlloc( 576*p_numPlasticElems, sizeof(double), (void **) &(p_gpuData->KPlastic) );

    if (!checkCUDAStatus_plasticity("AFPlasticityGPU: GPU Memory allocation "))
        return false;
    printf("AFPlasticityGPU: Memory allocation successful \n");
    cuMemGetInfo(&freeFinal, &total);
    printf("AFPlasticityGPU: Allocated %f MBytes out of %f MB \n",1.0*(freeInitial-freeFinal)/1024.0/1024.0, 1.0*freeInitial/1024.0/1024.0);

    return true;
}
/**********************************************************************/
void initializeGPUHexmeshData_plasticity(int p_nNodes,int p_nElems, int p_numPlasticElems, int p_maxNeighboringElements,
                              int *p_elems, int *p_free,double *p_f,int *p_neighborElems,
                              int *p_nNeighboringElems, double *p_KETemplates, int *p_nodeSignature,
                              int *p_neighborNodes, int *p_plasticElem,
                              bool *p_elemPlasticState, double *KPlastic, p_GPUData *p_gpuData)
{
    if ((p_gpuData == NULL) || (p_gpuData->p_nNodes == 0)) {
        printf("AFPlasticityGPU: GPU memory has not been allocated\n");
    }

    //	Copy from Host to Device
    cublasSetVector( 8*(p_nElems), sizeof(int), p_elems, 1, p_gpuData->p_elems, 1 );
    cublasSetVector( p_nElems, sizeof(int), p_plasticElem, 1, p_gpuData->p_plasticElem, 1 );
    cublasSetVector( p_maxNeighboringElements*(p_nNodes), sizeof(int), p_neighborElems, 1, p_gpuData->p_neighborElems, 1 );
    cublasSetVector( (p_nNodes), sizeof(int), p_nNeighboringElems, 1, p_gpuData->p_nNeighboringElems, 1 );

    checkCUDAStatus_plasticity("AFPlasticityGPU: GPU Transfer Set 1 ");


    if (p_nodeSignature != NULL)
            cublasSetVector( (p_nNodes), sizeof(int), p_nodeSignature, 1, p_gpuData->p_nodeSignature, 1 );
    if (p_neighborNodes != NULL)
            cublasSetVector( 27*(p_nNodes), sizeof(int), p_neighborNodes, 1, p_gpuData->p_neighborNodes, 1 );
    checkCUDAStatus_plasticity("AFPlasticityGPU:  GPU Transfer Set 2  ");

    cublasSetVector( (p_nNodes), sizeof(int), p_free, 1, p_gpuData->p_free, 1 );
    //cublasSetVector( (3*p_nNodes), sizeof(double), p_sol, 1, p_gpuData->p_sol, 1 );//not needed
    cudaMemset(p_gpuData->p_sol, 0, (3*p_nNodes)*sizeof(double));
    cublasSetVector( (3*p_nNodes), sizeof(double), p_f, 1, p_gpuData->p_f, 1 );
    cublasSetVector(24*24, sizeof(double),p_KETemplates,1,p_gpuData->p_KETemplates, 1 );
    checkCUDAStatus_plasticity("AFPlasticityGPU:  GPU Transfer Set 3 ");


    cublasSetVector( p_nElems, sizeof(bool), p_elemPlasticState, 1, p_gpuData->p_elemPlasticState, 1 );
    if (KPlastic != NULL)
    cublasSetVector( 576*(p_numPlasticElems), sizeof(double), KPlastic,1, p_gpuData->KPlastic,1 );
    checkCUDAStatus_plasticity("AFPlasticityGPU:  GPU Transfer Set 4 ");
}
/**********************************************************************/
void vectorDivideGPU_plasticity(int p_nNodes, double *z,double *x,double *y)
{
    dim3 dimBlock(THREADS);
    int nBlocks = p_nNodes/(THREADS) + ((p_nNodes%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    DivideByVectorKernel_plasticity<<< dimGrid, dimBlock,0 >>>(p_nNodes,z,x,y);// z = x./y
    cudaThreadSynchronize();
}
/**********************************************************************/
void zeaxpyGPU_plasticity(int p_nNodes, double *z,double a,double *x,double b, double *y)
{
    dim3 dimBlock(THREADS);
    int nBlocks = p_nNodes/(THREADS) + ((p_nNodes%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    ZEAXPBYKernel_plasticity<<< dimGrid, dimBlock,0 >>>(p_nNodes,z,a,x,b,y);
    cudaThreadSynchronize();
}
/**********************************************************************/

void KtuGPU_plasticity(p_GPUData *p_gpuData, double *din, double *dout)
{
    int p_nNodes = p_gpuData->p_nNodes;
    int p_nElems = p_gpuData->p_nElems;

    int nDOF = 3*p_nNodes;
    double *doutElastic, *doutPlastic;
    cublasAlloc( nDOF, sizeof(double), (void **) &doutElastic);
    cublasAlloc( nDOF, sizeof(double), (void **) &doutPlastic);
    //set all the elements of doutElastic and doutPlastic to zero.
    cudaMemset(doutElastic, 0, nDOF*sizeof(double));
    cudaMemset(doutPlastic, 0, nDOF*sizeof(double));

    // Carry out K*u, also enforce Dirichlet boundary conditions
    dim3 dimBlock(THREADS);
    int nBlocks = p_nNodes/(THREADS) + ((p_nNodes%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    cudaFuncSetCacheConfig(KEuKernelElemBased_plasticity,cudaFuncCachePreferL1);
    // Carry out dout = K*din

    KEuKernelElemBased_plasticity<<<dimGrid, dimBlock,0 >>>(p_nNodes, p_nElems,p_gpuData->p_nNeighboringElems,
                                                  p_gpuData->p_elems,p_gpuData->p_neighborElems,p_gpuData->p_KETemplates,
                                                  din,p_gpuData->p_free,doutElastic);
    cudaThreadSynchronize();
    checkCUDAStatus_plasticity("AFPlasticityGPU: Kernel for Elastic K matrix launched");

    KPuKernelElemBased<<<dimGrid, dimBlock,0>>> (p_nNodes, p_nElems,p_gpuData->p_elemPlasticState,p_gpuData->p_nNeighboringElems,
                                         p_gpuData->p_elems,p_gpuData->p_neighborElems, p_gpuData->KPlastic, p_gpuData->p_plasticElem,
                                         din,p_gpuData->p_free,doutPlastic);

    cudaThreadSynchronize();
    checkCUDAStatus_plasticity("AFPlasticityGPU: Kernel for PLastic K matrix launched");

    zeaxpyGPU_plasticity(p_nNodes,dout,1,doutElastic,-1, doutPlastic);
    cublasFree(doutElastic);
    cublasFree(doutPlastic);
    cudaThreadSynchronize();

}
/**********************************************************************/
int ConjugateGradientOnGPU_plasticity(p_GPUData *p_gpuData,int MaxIter, double CGRelResErr)
{
    // Conjugate Gradient method with Jacobi preconditioning
    double rho,alpha,beta,rho_new,normr0,normr;
    int niter,nDOF;
    double *r,*p,*a,*f,*sol;
    Timer timer;

    int p_nNodes = p_gpuData->p_nNodes;
    nDOF = 3*p_nNodes;
    sol = p_gpuData->p_sol;
    f = p_gpuData->p_f;
   printf("AFPlasticityGPU: norm(f) = %e \n",cublasDdot(3*p_nNodes,f,1,f,1));
    checkCUDAStatus_plasticity("GPU Data extraction error in CG");
    cublasAlloc( nDOF, sizeof(double), (void **) &r);
    cublasAlloc( nDOF, sizeof(double), (void **) &p);
    cublasAlloc( nDOF, sizeof(double), (void **) &a);
    checkCUDAStatus_plasticity("GPU Memory allocation error in CG");
    timer.start();


   // zeaxpyGPU(p_nNodes,r,-1,a,1,f);// r = -a + f
    cublasDcopy(3*p_nNodes,f,1,r,1);// Set r = f  as a=0
    normr = cublasDdot(3*p_nNodes,r,1,r,1);
    cudaThreadSynchronize();
    normr = sqrt(normr);
    normr0 = normr;

    cublasDcopy(3*p_nNodes,r,1,a,1);// Set a = r

    cublasDcopy(nDOF,a,1,p,1);// Set p = a
    cudaThreadSynchronize();
    rho = cublasDdot(nDOF,a,1,r,1);// rho = a'*r;
    cudaThreadSynchronize();
    niter = 0;

    while  ((niter < 10) ||(normr > CGRelResErr*normr0))   {//% Test break condition
        //check this
        KtuGPU_plasticity(p_gpuData,p,a);// a = K*p;
        checkCUDAStatus_plasticity("GPU KtuGPU_plasticity error in CG");
        beta = cublasDdot(nDOF,a,1,p,1);//DOTPROD3D(a,p);
        cudaThreadSynchronize();
        alpha = rho/beta;
        zeaxpyGPU_plasticity(p_nNodes,sol,alpha,p,1,sol);//u = alpha*p + u;
        zeaxpyGPU_plasticity(p_nNodes,r,-alpha,a,1,r);	//r = -alpha*a + r ;
        normr = cublasDdot(nDOF,r,1,r,1);
        cudaThreadSynchronize();
        normr = sqrt(normr);
        cublasDcopy(nDOF,r,1,a,1);// Set a= r

        cudaThreadSynchronize();
        rho_new =  cublasDdot(nDOF,a,1,r,1);//rho = a'*r
        cudaThreadSynchronize();
        zeaxpyGPU_plasticity(p_nNodes,p,1,a,(rho_new/rho),p);//p = a + rho_new/rho * p;
        rho = rho_new;
        if (niter == 1) normr0 = normr;//update for consistent scaling convergence
        niter++;

        if ((niter%500 == 0)) printf("\n %d: %e ",niter,normr/normr0);
        if (niter%1000 ==0) printf("\nniter = %d: normr = %e \n",niter,normr/normr0);
        if (niter == MaxIter) {
            printf("GPU CG: Did not converge in %d iterations! \n",niter);
            printf("GPU CG: Relative error in residual = %e \n",normr/normr0);
            niter = -1;
            break;
        }
        if (normr > 1000*normr0) {
            //printf("GPU CG: Diverging! \n");
            niter = -1;
            break;
        }
    }
    cudaThreadSynchronize();
    //printf("\n %d: %e \n",niter,normr/normr0);
    cublasFree(r);
    cublasFree(p);
    cublasFree(a);
    checkCUDAStatus_plasticity("GPU Memory deallocation error in CG");
    //printf("AFPlasticity: CG on GPU; nIter = %d, resErr = %e \n",niter,normr);
    return niter;
}
/**********************************************************************/
void transferSolutionFromGPU_plasticity(p_GPUData *p_gpuData, double *p_sol)
{
    cublasGetVector( 3*p_gpuData->p_nNodes, sizeof(double), p_gpuData->p_sol, 1, p_sol, 1 );
}
/**********************************************************************/
void releaseHexmeshGPUMemory_plasticity(p_GPUData *p_gpuData)
{
    //cublasShutdown();
    // GPU memory deallocation
    if ((p_gpuData == NULL) || (p_gpuData->p_nNodes == 0))
        return;

    //printf("AFPlasticityGPU: Releasing GPU memory\n");
    cublasFree(p_gpuData->p_elems);
    cublasFree(p_gpuData->p_plasticElem);
    cublasFree(p_gpuData->p_neighborElems);
    cublasFree(p_gpuData->p_nNeighboringElems);
    cublasFree(p_gpuData->p_free);

    cublasFree(p_gpuData->p_nodeSignature);
    cublasFree(p_gpuData->p_neighborNodes);

    cublasFree(p_gpuData->p_f);   
    cublasFree(p_gpuData->p_sol);
    cublasFree(p_gpuData->p_KETemplates);

    cublasFree(p_gpuData->p_elemPlasticState);
    cublasFree(p_gpuData->KPlastic);

    p_gpuData->p_nNodes =0;
}
/************************************************************************/
/************************************************************************/
/************************************************************************/

void InitializeDeflationForPlasticityOnGPU(int h_nNodes, int nGroups, int gType, int *nodeGroupNumber,
                            int *groupNodes, int *groupKey,
                            double *h_x, double *h_y, double *h_z, double *CGPatch,
                            double *KLower, double *KPermuteVector, p_WorkSpaceData *p_gpuWS)
{
    size_t freeInitial, freeFinal, total;
    int nWorkSpace;

    cuMemGetInfo(&freeInitial, &total);
    p_gpuWS->d_nGroups = nGroups;

    p_gpuWS->d_workSpaceType = gType;
    nWorkSpace = 6*nGroups;

    // GPU Memory allocation
    cublasAlloc( (h_nNodes), sizeof(int), (void **) &(p_gpuWS->d_nodeGroupNumber) );
    cublasAlloc( (h_nNodes), sizeof(int), (void **) &(p_gpuWS->d_groupNodes) );
    cublasAlloc( (nGroups), sizeof(int), (void **) &(p_gpuWS->d_groupKey) );
    cublasAlloc( (h_nNodes), sizeof(double), (void **) &(p_gpuWS->d_x) );
    cublasAlloc( (h_nNodes), sizeof(double), (void **) &(p_gpuWS->d_y) );
    cublasAlloc( (h_nNodes), sizeof(double), (void **) &(p_gpuWS->d_z) );
    cublasAlloc( (3*nGroups), sizeof(double), (void **) &(p_gpuWS->d_CGPatch) );
    cublasAlloc( (nWorkSpace*nWorkSpace), sizeof(double), (void **) &(p_gpuWS->d_KLower) );
    cublasAlloc( (nWorkSpace), sizeof(double), (void **) &(p_gpuWS->d_KPermuteVector) );
    checkCUDAStatus_plasticity("GPU Memory allocation ");

    cuMemGetInfo(&freeFinal, &total);

        printf("GPU: Allocated %f MBytes/1000000.0 \n",1.0*(freeInitial-freeFinal));

    //	Copy from Host to Device
    cublasSetVector( (h_nNodes), sizeof(int), nodeGroupNumber, 1, p_gpuWS->d_nodeGroupNumber, 1 );
    cublasSetVector( (h_nNodes), sizeof(int), groupNodes, 1, p_gpuWS->d_groupNodes, 1 );
    cublasSetVector( (nGroups), sizeof(int), groupKey, 1, p_gpuWS->d_groupKey, 1 );
    cublasSetVector( (h_nNodes), sizeof(double), h_x, 1, p_gpuWS->d_x, 1 );
    cublasSetVector( (h_nNodes), sizeof(double), h_y, 1, p_gpuWS->d_y, 1 );
    cublasSetVector( (h_nNodes), sizeof(double), h_z, 1, p_gpuWS->d_z, 1 );
    cublasSetVector( (3*nGroups), sizeof(double), CGPatch, 1, p_gpuWS->d_CGPatch, 1 );
    cublasSetVector( (nWorkSpace*nWorkSpace), sizeof(double), KLower, 1, p_gpuWS->d_KLower, 1 );
    cublasSetVector( (nWorkSpace), sizeof(double), KPermuteVector, 1, p_gpuWS->d_KPermuteVector, 1 );
    checkCUDAStatus_plasticity("GPU Data Initialization ");

        printf("GPU: Transferred Workspace data from CPU to GPU  \n");

}
/*************************************************************************/
void enforceDirichletGPU_plasticity(p_GPUData *gpuData,double *t)
{
    dim3 dimBlock(THREADS);
    int h_nNodes = gpuData->p_nNodes;
    int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    enforceDirichletKernel_plasticity<<< dimGrid, dimBlock,0 >>>(h_nNodes, t, gpuData->p_free);
    cudaThreadSynchronize();
}
/*************************************************************************/
void permuteVectorGPU_plasticity(p_WorkSpaceData *gpuWS,int N,double *mu,double *muP)
{
    dim3 dimBlock(THREADS);
    int nBlocks = N/(THREADS) + ((N%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    permuteVectorKernel_plasticity<<< dimGrid, dimBlock,0 >>>(N, gpuWS->d_KPermuteVector,mu,muP);
    cudaThreadSynchronize();
}
/*************************************************************************/
void permuteTransposeVectorGPU_plasticity(p_WorkSpaceData *gpuWS,int N,double *mu,double *muP)
{
    dim3 dimBlock(THREADS);
    int nBlocks = N/(THREADS) + ((N%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    permuteTransposeVectorKernel_plasticity<<< dimGrid, dimBlock,0 >>>(N, gpuWS->d_KPermuteVector,mu,muP);
    cudaThreadSynchronize();
}
/*************************************************************************/
void getWorkSpaceVectorGPU_plasticity(p_WorkSpaceData *gpuWS, double *mu, double *t)
{
    int THREADS_S = 8; //8 The magic number!
    dim3 dimBlock(THREADS_S);
    int nBlocks = (gpuWS->d_nGroups/THREADS_S) + ((gpuWS->d_nGroups%THREADS_S)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    int gType = gpuWS->d_workSpaceType;
    // Carry out mu = W'*t
    if (gType == DEFLATION_RIGID) {
        cudaFuncSetCacheConfig(getWorkSpaceVectorKernel0_plasticity,cudaFuncCachePreferL1);
        getWorkSpaceVectorKernel0_plasticity<<< dimGrid, dimBlock,0 >>> (gpuWS->d_nGroups, gpuWS->d_groupKey, mu, t, gpuWS->d_x,
            gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
            gpuWS->d_groupNodes);
    }
    else {
        cudaFuncSetCacheConfig(getWorkSpaceVectorKernel1_plasticity,cudaFuncCachePreferL1);
        getWorkSpaceVectorKernel1_plasticity<<< dimGrid, dimBlock,0 >>> (gpuWS->d_nGroups, gpuWS->d_groupKey, mu, t, gpuWS->d_x,
        gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
        gpuWS->d_groupNodes);
    }
    cudaThreadSynchronize();
}
/************************************************************************/
void getSolutionSpaceVectorGPU_plasticity(p_WorkSpaceData *gpuWS, double *t, double *result, int h_nNodes)
{
    dim3 dimBlock(THREADS);
    int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
    nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
    dim3 dimGrid(nBlocks);
    int gType = gpuWS->d_workSpaceType;
    // Carry out mu = W'*t
    if (gType == DEFLATION_RIGID)
        getSolutionSpaceVectorKernel0_plasticity<<< dimGrid, dimBlock,0 >>> (h_nNodes, t, result, gpuWS->d_x,
                            gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
                            gpuWS->d_nodeGroupNumber);
    else
        getSolutionSpaceVectorKernel1_plasticity<<< dimGrid, dimBlock,0 >>> (h_nNodes, t, result, gpuWS->d_x,
                            gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
                            gpuWS->d_nodeGroupNumber);
    cudaThreadSynchronize();
}
/**********************************************************************/
int WorkSpaceCGOnGPU_plasticity(p_GPUData *p_gpuData, p_WorkSpaceData *p_gpuWS, int MaxIter, double CGRelResErr)
{
    // Conjugate Gradient method with workspace

    int h_nNodes = p_gpuData->p_nNodes;
    int nGroups = p_gpuWS->d_nGroups;
    int gType = p_gpuWS->d_workSpaceType;

    double rho, alpha, beta, rho_new, normr0, normr;
    int niter, nDOF, nWorkSpace;
    double *r,*p,*a,*d, *t, *mu, *result, *f,*sol,*muP;
    Timer totalTimer;

    totalTimer.start();
    nDOF = 3*h_nNodes;
    if (gType == DEFLATION_RIGID)
        nWorkSpace = 6*nGroups;//Workspace method
    else
        nWorkSpace = 9*nGroups;

    sol = p_gpuData->p_sol;
    f = p_gpuData->p_f;
    enforceDirichletGPU_plasticity(p_gpuData,f);
    cublasAlloc( nDOF, sizeof(double), (void **) &r);
    cublasAlloc( nDOF, sizeof(double), (void **) &p);
    cublasAlloc( nDOF, sizeof(double), (void **) &a);
    cublasAlloc( nDOF, sizeof(double), (void **) &d);
    cublasAlloc( nDOF, sizeof(double), (void **) &t);	// for workspace correction
    cublasAlloc( nWorkSpace, sizeof(double), (void **) &mu);
    cublasAlloc( nWorkSpace, sizeof(double), (void **) &muP);
    cublasAlloc( nWorkSpace, sizeof(double), (void **) &result);

    checkCUDAStatus_plasticity("GPU Memory allocation error in CGW");

    // Performing conjugate gradient here with residual to start
    cublasDcopy(3*h_nNodes,f,1,r,1);// Set r = f  as a=0
    cudaMemset(mu, 0, nWorkSpace*sizeof(double));
    getWorkSpaceVectorGPU_plasticity(p_gpuWS,mu,r);// mu = W'*r


    permuteVectorGPU_plasticity(p_gpuWS,nWorkSpace,mu,muP);
    cublasDtrsv('L','N','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
    cublasDtrsv('L','T','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
    permuteTransposeVectorGPU_plasticity(p_gpuWS,nWorkSpace,muP,mu);

    getSolutionSpaceVectorGPU_plasticity(p_gpuWS,t,mu,h_nNodes);//sol = W*result

    //cudaMemset(result, 0, nWorkSpace*sizeof(double));
    //cublasDgemv ( 'N', nWorkSpace, nWorkSpace, 1,p_gpuWS->d_KLower, nWorkSpace, mu,1, 1, result, 1);//Solve result = Ktilde\mu;
    //getSolutionSpaceVectorGPU_plasticity(p_gpuData,p_gpuWS,t,result,h_nNodes);//sol = W*result
    enforceDirichletGPU_plasticity(p_gpuData,t);
    zeaxpyGPU_plasticity(h_nNodes,sol,1,t,1,sol);//u = t + u;

    KtuGPU_plasticity(p_gpuData,sol,a);// a = K*sol;
    zeaxpyGPU_plasticity(h_nNodes,r,-1,a,1,f);// r = -a + f
    normr0 = cublasDdot(nDOF,r,1,r,1);
    normr = 10*normr0;
    //********************************************************
    KtuGPU_plasticity(p_gpuData,r,t);// t = K*r;
    cudaMemset(mu, 0, nWorkSpace*sizeof(double));
    getWorkSpaceVectorGPU_plasticity(p_gpuWS,mu,t);// mu = W'*t

    permuteVectorGPU_plasticity(p_gpuWS,nWorkSpace,mu,muP);
    cublasDtrsv('L','N','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
    cublasDtrsv('L','T','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
    permuteTransposeVectorGPU_plasticity(p_gpuWS,nWorkSpace,muP,mu);

    getSolutionSpaceVectorGPU_plasticity(p_gpuWS,t,mu,h_nNodes);//sol = W*result

    enforceDirichletGPU_plasticity(p_gpuData,t);
    zeaxpyGPU_plasticity(h_nNodes,p,-1,t,1,r);// p = r - t
    //********************************************************
    rho =  cublasDdot(nDOF,r,1,r,1);
    niter = 1;
    while (normr > CGRelResErr*normr0)   {
        KtuGPU_plasticity(p_gpuData,p,a);// a = K*p;
        beta = cublasDdot(nDOF,a,1,p,1);//DOTPROD(a,p);
        alpha = rho/beta;
        zeaxpyGPU_plasticity(h_nNodes,sol,alpha,p,1,sol);//u = alpha*p + u;
        if (niter%500 == 0)	{ // restart CG with recomputing residual
            KtuGPU_plasticity(p_gpuData,sol,a);// a = K*sol;
            zeaxpyGPU_plasticity(h_nNodes,r,-1,a,1,f);// r = -a + f
        }
        else
            zeaxpyGPU_plasticity(h_nNodes,r,-alpha,a,1,r);	//r = -alpha*a + r

        rho_new =  cublasDdot(nDOF,r,1,r,1);//rho = r'*r
        normr = (rho_new);
        KtuGPU_plasticity(p_gpuData,r,t);// t = K*r;
        cudaMemset(mu, 0, nWorkSpace*sizeof(double));
        getWorkSpaceVectorGPU_plasticity(p_gpuWS,mu,t);// mu = W'*f

        permuteVectorGPU_plasticity(p_gpuWS,nWorkSpace,mu,muP);
        cublasDtrsv('L','N','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
        cublasDtrsv('L','T','N',nWorkSpace, p_gpuWS->d_KLower, nWorkSpace,muP, 1);
        permuteTransposeVectorGPU_plasticity(p_gpuWS,nWorkSpace,muP,mu);

        getSolutionSpaceVectorGPU_plasticity(p_gpuWS,t,mu,h_nNodes);//sol = W*result

        enforceDirichletGPU_plasticity(p_gpuData,t);
        zeaxpyGPU_plasticity(h_nNodes,p,1,r,(rho_new/rho),p);//p = r + rho_new/rho * p;
        zeaxpyGPU_plasticity(h_nNodes,p,-1,t,1,p);// p = -t + p
        rho = rho_new;
        niter++;

        if ((niter%50 ==0)) printf(" %d	%e \n",niter,normr/normr0);
        if (niter == MaxIter) {
            printf("GPU DCG: Did not converge in %d iterations! \n",niter);
            printf("GPU DCG: Relative percent error in residual = %e \n",100*normr/normr0);
            niter = -1;
            break;
        }
        if (normr > 1e10*normr0) {
            printf("GPU DCG: Diverging! \n");
            niter = -1;
            break;
        }
    }

    printf("Deflation: CG on GPU; nIter = %d; time = %0.3f (s)\n",niter,totalTimer.getElapsedTimeInSec());

    //printf("\nGPU DCG: %d iterations; residual = %e \n",niter,normr/normr0);
    cublasFree(r);
    cublasFree(p);
    cublasFree(a);
    cublasFree(d);
    cublasFree(t);
    cublasFree(mu);
    cublasFree(muP);
    cublasFree(result);
    return niter;
}
/*************************************************************************/
void freeGPUWorkSpaceData_plasticity(p_WorkSpaceData *p_gpuWS)
{
    if ((p_gpuWS == NULL)||(p_gpuWS->d_nGroups==0))
        return;
    cublasFree(p_gpuWS->d_nodeGroupNumber);
    cublasFree(p_gpuWS->d_x);
    cublasFree(p_gpuWS->d_y);
    cublasFree(p_gpuWS->d_z);
    cublasFree(p_gpuWS->d_CGPatch);
    cublasFree(p_gpuWS->d_KLower);
    cublasFree(p_gpuWS->d_KPermuteVector);
    p_gpuWS->d_nGroups = 0;
}
/****************************/
//#include "WorkSpaceMethodGPUPlasticity.cu"
#endif
