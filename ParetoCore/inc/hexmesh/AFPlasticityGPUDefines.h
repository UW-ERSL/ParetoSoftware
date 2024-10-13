#ifndef AFPLASTICITYGPUDEFINES_H
#define AFPLASTICITYGPUDEFINES_H

#include "CommonDefines.h"

// MAXELEMS is the maximum number of elements allowed
// It is used for encoding neighboring element for a given node (minimizes data access)
// The constraint is MAXELEMS*index must not exceed 2^30 (for int); index ranges from to 7
// We therefore set
// MAXELEMS to 2^26 
#define MAXELEMS 134017728


// If any element is 'out', what is the relative rho value (also see below). Default use 0
#define VOID_EPS 0

// The following is used as E = E0*(rho)^Penalty, and M = M0*(rho)^penalty where 
// rho is the pseudo-density of an element, i.e., partial density of voxel
// Setting the E penalty to > M penalty leads to a lower bound
// estimate for eigen values

#define MAXBLOCKS (65535)
#define THREADS (512) 
#define TCLMatrix dMatrix
#define TCLVector dVector


struct p_GPUData {
	// GPU Variables; see AFElasticity.h for definitions
    int p_nNodes;// total number of nodes
    int p_nElems; // total number of elements
    int p_numPlasticElems;
    int p_maxNeighboringElements;// for each node, 8 for a uniform hex grid



    int		*p_elems; // 8*p_nElems, store the first nodes of all elements, then second nodes, etc
    int     *p_plasticElem;
    int		*p_neighborElems; // p_maxNeighboringElements*p_nNodes, the element neighbors of a node; the 0th neighbor of all nodes are stored first, then the 1st neighbor and so on
    int		*p_nNeighboringElems; // for each node, how many valid neighboring elements for a given node,  8 for a uniform hex mesh
    int		*p_free; // Are (u,v,w) fixed or free; each component is encoded into a single int
					// 1 means u is free, 2 means v is free, 4 means w is free
					//3 means u and v are free, etc

    int		*p_nodeSignature; // for node-based voxel grid, for node, provides its topology (how many elements connected and where)
    int		*p_neighborNodes; // for node-based voxel grid, for each node, who are the 27 neighbors

    double	*p_f; //  force on each node
    double	*p_sol;// dU displacement solution at each node
    double	*p_KETemplates;//  24*24*p_nTemplateElements;

    bool  *p_elemPlasticState; //plastic state flag
    double  *KPlastic; //dev stresses at each GP
} ;

struct p_WorkSpaceData {
    // GPU Variables;
    int		d_nGroups;// total number of groups
    int		d_workSpaceType;// Type of group 0: rigid body or 1: plate

    int		*d_nodeGroupNumber; // h_nNodes, store the group number for each node
    int		*d_groupNodes; // h_nNodes, store the group number for each node?
    int		*d_groupKey; // h_nNodes, store the group number for each node?

    double	*d_x; //  x coordinates of h_nNodes
    double	*d_y; //  y coordinates of h_nNodes
    double	*d_z; //  z coordinates of h_nNodes
    double	*d_CGPatch;//  [nGroups][3];
    double	*d_KLower;//  [6*nGroups][6*nGroups];// lower triangle matrix stored in full
    double	*d_KPermuteVector;//permuation vector
} ;

// GPU functions
//Basic functions
void	dumpGPUInfo_plasticity();
bool	checkCUDAStatus_plasticity(const char *msg);
bool	createAFPlasticityGPUMemory(int p_nNodes,int p_nElems, int p_numPlasticElems, int p_maxNeighboringElements, p_GPUData *p_gpuData);
void	initializeGPUHexmeshData_plasticity(int p_nNodes,int p_nElems, int p_numPlasticElems, int p_maxNeighboringElements,
                                            int *p_elems, int *p_free,double *p_f,int *p_neighborElems,
                                            int *p_nNeighboringElems, double *p_KETemplates, int *p_nodeSignature,
                                            int *p_neighborNodes, int *p_plasticElem,
                                            bool *p_elemPlasticState, double *KPlastic, p_GPUData *p_gpuData);
//CG functions
//void	KuGPUWithMemoryTransfer(p_GPUData *p_gpuData, double *hin, double *hout,bool nodeBasedAssembly);
void	KtuGPU_plasticity(p_GPUData *p_gpuData, double *din, double *dout);
int		ConjugateGradientOnGPU_plasticity(p_GPUData *p_gpuData,int MaxIter,double CGResidualError);
//Post process
void	transferSolutionFromGPU_plasticity(p_GPUData *p_gpuData, double *p_sol);
void	releaseHexmeshGPUMemory_plasticity(p_GPUData *p_gpuData);

//Deflation

void	InitializeDeflationForPlasticityOnGPU(int h_nNodes, int nGroups, int ws_deflationType, int *ws_nodeGroupNumber, int *ws_groupSortedNodes, int *ws_groupEndNodeIndex,
                                 double *h_x, double *h_y, double *h_z, double *h_CGPatch, double *KWKWLower,double *KPermute,
                                 p_WorkSpaceData *gpuWS);
void    enforceDirichletGPU_plasticity(p_GPUData *p_gpuData,double *t);
void    permuteVectorGPU_plasticity(p_WorkSpaceData *p_gpuWS,int N,double *mu,double *muP);
void    permuteTransposeVectorGPU_plasticity(p_WorkSpaceData *p_gpuWS,int N,double *mu,double *muP);
void    getWorkSpaceVectorGPU_plasticity(p_WorkSpaceData *p_gpuWS, double *mu, double *t);
void    getSolutionSpaceVectorGPU_plasticity(p_WorkSpaceData *p_gpuWS, double *t, double *result, int h_nNodes);
int     WorkSpaceCGOnGPU_plasticity(p_GPUData *p_gpuData, p_WorkSpaceData *p_gpuWS, int MaxIter, double CGRelResErr);
void    freeGPUWorkSpaceData_plasticity(p_WorkSpaceData *p_gpuWS);
#endif
