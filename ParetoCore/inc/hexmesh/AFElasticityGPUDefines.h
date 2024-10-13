#ifndef HEXMESHGPUDEFINES_H
#define HEXMESHGPUDEFINES_H

#include "CommonDefines.h"

// MAXELEMS is the maximum number of elements allowed
// It is used for encoding neighboring element for a given node (minimizes data access)
// The constraint is MAXELEMS*index must not exceed 2^30 (for int); index ranges from to 7
// We therefore set
// MAXELEMS to 2^26 
#define MAXELEMS 134017728


// If any element is 'out', what is the relative rho value (also see below). Default use 0
#define VOID_EPS 0


#define MAXBLOCKS (65535)
#define THREADS (512) 
#define TCLMatrix dMatrix
#define TCLVector dVector


struct GPUData {
	// GPU Variables; see AFElasticity.h for definitions
	int h_nNodes;// total number of nodes
	int h_nElems; // total number of elements
	int h_nTemplateElements;// how many distinct elements, 1 for a uniform hex grid
	int h_maxNeighboringElements;// for each node, 8 for a uniform hex grid



	int		*d_elems; // 8*h_nElems, store the first nodes of all elements, then second nodes, etc
	double	*d_pseudodensity; // h_nElems, pseudo density (used, for example, in topology optimization)
	bool	*d_nodeExists; // h_nNodes, nodes may disappear during topology optimization
	int		*d_neighborElems; // h_maxNeighboringElements*h_nNodes, the element neighbors of a node; the 0th neighbor of all nodes are stored first, then the 1st neighbor and so on
	int		*d_nNeighboringElems; // for each node, how many valid neighboring elements for a given node,  8 for a uniform hex mesh
	int		*d_templateID; // for each element, the corresponding template id for looking up Ke stiffness matrix
	int		*d_free; // Are (u,v,w) fixed or free; each component is encoded into a single int
					// 1 means u is free, 2 means v is free, 4 means w is free
					//3 means u and v are free, etc

	int		*d_nodeSignature; // for node-based voxel grid, for node, provides its topology (how many elements connected and where)
	int		*d_neighborNodes; // for node-based voxel grid, for each node, who are the 27 neighbors 

	double	*d_f; //  force on each node
	double	*d_KETemplates;//  24*24*h_nTemplateElements;
	double	*d_METemplates;//  24*24*h_nTemplateElements;
	double	*d_KeNode; // 256*3*81; for node-based voxel grid
	double	*d_MeNode; // 256*3*81; for node-based voxel grid
	double	*d_sol;// final displacement solution at each node
} ;


// GPU functions
void	dumpGPUInfo();
bool	checkCUDAStatus(const char *msg);
bool	createAFElasticityGPUMemory(int h_nNodes,int h_nElems, int h_maxNeighboringElements, int h_nTemplateElements, GPUData *h_gpuData);
void	initializeGPUHexmeshData(int h_nNodes,int h_nElems, int h_maxNeighboringElements, int h_nTemplateElements, 
							int *h_elems,double *h_pseudoDensity,bool *h_nodeExists,int *h_templateID,
							double *h_sol,int *h_free,double *h_f,int *h_neighborElems, int *h_nNeighboringElems,double *h_KETemplates,
							double *h_METemplates,int *h_nodeSignature,int *h_neighborNodes, double *h_KeNode, double *h_MeNode,GPUData *h_gpuData);
void	updateBCsOnGPU(int h_nNodes,double *h_sol,int *h_free,double *h_f,GPUData *h_gpuData);
void	updateKeOnGPU(int h_nTemplateElements,double *h_KETemplates,GPUData *h_gpuData);
void	updateMeOnGPU(int h_nTemplateElements,double *h_METemplates,GPUData *h_gpuData);
void	updateNodeNeighborsOnGPU(int h_nNodes, int *h_neighborNodes,GPUData *h_gpuData);
void	updateNodeSignaturesOnGPU(int h_nNodes, int *h_nodeSignature,GPUData *h_gpuData);
void	updateNumNeighboringElemsOnGPU(int h_nNodes,int *h_nNeighboringElems,GPUData *h_gpuData);
void	updateNeighboringElemsOnGPU(int h_nNodes,int h_maxNeighboringElements,int *h_neighboringElems,GPUData *h_gpuData);
void	setKeNodeOnGPU(double *h_KeNode,GPUData *h_gpuData);
void	setMeNodeOnGPU(double *h_KeNode,GPUData *h_gpuData);
void	setGPUPseudoDensity(int h_nElems,double *h_pseudoDensity,GPUData *h_gpuData);
void	setGPUNodeExists(int h_nNodes,bool *h_nodeExists,GPUData *h_gpuData);
void	setGPUSolution(int h_nNodes,double *sol,GPUData *h_gpuData);
void	setGPUKDiag(int h_nNodes,double *Kdiag,GPUData *h_gpuData);
void	setGPUForce(int h_nNodes,double *f,GPUData *h_gpuData);
void	setGPUFreeDOF(int h_nNodes,int *free,GPUData *h_gpuData);
void	setGPUForcesBC(int h_nNodes, double *h_sol,int *h_free,double *h_f,GPUData *h_gpuData);
void	setGPUVerbose(bool val);

void	KuGPUWithMemoryTransfer(GPUData *h_gpuData, double *hin, double *hout,bool nodeBasedAssembly);
void	MuGPUWithMemoryTransfer(GPUData *h_gpuData, double *hin, double *hout,bool nodeBasedAssembly);
int		ConjugateGradientOnGPU(GPUData *h_gpuData,int MaxIter,double CGResidualError,bool nodeBasedAssembly);
int		firstEigenmodeOnGPU(GPUData *h_gpuData,int MaxIter,bool h_nodeBasedAssembly, double eigRelativeError,double *eigenValues,double **eigenModes);
int		RayleighCGOnGPU(GPUData *h_gpuData,int nEigenModes, double *eigenValues,double **eigenModes, int useInitialGuess,double relResidual,int maxIterPerMode,bool nodeBasedAssembly);
void	transferSolutionFromGPU(GPUData *h_gpuData, double *h_sol);
void	releaseHexmeshGPUMemory(GPUData *h_gpuData);
int		isThereValidCudaDevice();
void	KuGPU(GPUData *h_gpuData, double *din, double *dout,bool nodeBasedAssembly);
void	MuGPU(GPUData *h_gpuData, double *din, double *dout,bool nodeBasedAssembly);
#endif
