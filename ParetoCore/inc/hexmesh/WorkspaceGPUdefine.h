#ifndef WORKSPACEGPUDEFINES_H
#define WORKSPACEGPUDEFINES_H



struct WorkSpaceData {
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
void	InitializeDeflationForElasticityOnGPU(int h_nNodes, int nGroups, int ws_deflationType, int *ws_nodeGroupNumber, int *ws_groupSortedNodes, int *ws_groupEndNodeIndex,
								 double *h_x, double *h_y, double *h_z, double *h_CGPatch, double *KWKWLower,double *KPermute,
								 WorkSpaceData *gpuWS);
void	setVectorGPU(int size, double *vector, double value);
void	getWorkSpaceVectorGPU(GPUData *h_gpuData, double *outVector, double *inVector);
void	getSolutionSpaceVectorGPU(GPUData *h_gpuData, double *outVector, double *inVector);
int		WorkSpaceCGOnGPU(GPUData *h_gpuData,WorkSpaceData *gpuWS,int MaxIter,
						 double CGResidualError,bool nodeBasedAssembly);
void	freeGPUWorkSpaceData(WorkSpaceData *gpuWS);
#endif