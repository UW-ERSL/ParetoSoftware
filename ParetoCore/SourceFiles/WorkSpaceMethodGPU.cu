
#include "../inc/hexmesh/WorkspaceGPUdefine.h"

#ifdef COMPILE_GPUCODE
/**********************************************************************/
void InitializeDeflationForElasticityOnGPU(int h_nNodes, int nGroups, int gType, int *nodeGroupNumber,
							int *groupNodes, int *groupKey,
							double *h_x, double *h_y, double *h_z, double *CGPatch,
							double *KLower, double *KPermuteVector, WorkSpaceData *gpuWS)
{
	size_t freeInitial, freeFinal, total;
	int nWorkSpace;

	cuMemGetInfo(&freeInitial, &total);
	gpuWS->d_nGroups = nGroups;
	
	gpuWS->d_workSpaceType = gType;
	if (gType == DEFLATION_RIGID) nWorkSpace = 6*nGroups;
	else nWorkSpace = 9*nGroups;
	
	// GPU Memory allocation 
	cublasAlloc( (h_nNodes), sizeof(int), (void **) &(gpuWS->d_nodeGroupNumber) );
	cublasAlloc( (h_nNodes), sizeof(int), (void **) &(gpuWS->d_groupNodes) );
	cublasAlloc( (nGroups), sizeof(int), (void **) &(gpuWS->d_groupKey) );
	cublasAlloc( (h_nNodes), sizeof(double), (void **) &(gpuWS->d_x) );
	cublasAlloc( (h_nNodes), sizeof(double), (void **) &(gpuWS->d_y) );
	cublasAlloc( (h_nNodes), sizeof(double), (void **) &(gpuWS->d_z) );
	cublasAlloc( (3*nGroups), sizeof(double), (void **) &(gpuWS->d_CGPatch) );
	cublasAlloc( (nWorkSpace*nWorkSpace), sizeof(double), (void **) &(gpuWS->d_KLower) );
	cublasAlloc( (nWorkSpace), sizeof(double), (void **) &(gpuWS->d_KPermuteVector) );
	checkCUDAStatus("GPU Memory allocation ");

	cuMemGetInfo(&freeFinal, &total);
	if (h_hexmeshGPU_verbose)
		printf("GPU: Allocated %f MBytes/1000000.0 \n",1.0*(freeInitial-freeFinal));

	//	Copy from Host to Device
	cublasSetVector( (h_nNodes), sizeof(int), nodeGroupNumber, 1, gpuWS->d_nodeGroupNumber, 1 );
	cublasSetVector( (h_nNodes), sizeof(int), groupNodes, 1, gpuWS->d_groupNodes, 1 );
	cublasSetVector( (nGroups), sizeof(int), groupKey, 1, gpuWS->d_groupKey, 1 );
	cublasSetVector( (h_nNodes), sizeof(double), h_x, 1, gpuWS->d_x, 1 );
	cublasSetVector( (h_nNodes), sizeof(double), h_y, 1, gpuWS->d_y, 1 );
	cublasSetVector( (h_nNodes), sizeof(double), h_z, 1, gpuWS->d_z, 1 ); 
	cublasSetVector( (3*nGroups), sizeof(double), CGPatch, 1, gpuWS->d_CGPatch, 1 ); 
	cublasSetVector( (nWorkSpace*nWorkSpace), sizeof(double), KLower, 1, gpuWS->d_KLower, 1 );
	cublasSetVector( (nWorkSpace), sizeof(double), KPermuteVector, 1, gpuWS->d_KPermuteVector, 1 );
	checkCUDAStatus("GPU Data Initialization ");
	if (h_hexmeshGPU_verbose)
		printf("GPU: Transferred Workspace data from CPU to GPU  \n");
	
}
/*************************************************************************/
__global__ void enforceDirichletKernel(int h_nNodes, double *t, int *d_free)
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
__global__ void permuteVectorKernel(int N, double *PVector,double *x, double *y)
{
	register int row,row_x;
	
	row = blockIdx.x*blockDim.x + threadIdx.x;

	if (row < N) {
		row_x = PVector[row];
		y[row] = x[row_x];
	}
}
/*************************************************************************/
__global__ void permuteTransposeVectorKernel(int N, double *PVector,double *x, double *y)
{
	register int row,row_x;
	
	row = blockIdx.x*blockDim.x + threadIdx.x;

	if (row < N) {
		row_x = PVector[row];
		y[row_x] = x[row];
	}
}
/*************************************************************************/
__global__ void getWorkSpaceVectorKernel0(int d_nGroups, int *d_groupKey,double *outVector, double *inVector,
										  double *d_x, double *d_y, double *d_z, double *d_CGPatch,
										  int *d_groupNodes, bool *d_nodeExists)
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
			if(d_nodeExists[node]){
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
__global__ void getWorkSpaceVectorKernel1(int d_nGroups, int *d_groupKey,double *outVector, double *inVector,
										double *d_x, double *d_y, double *d_z, double *d_CGPatch, 
										int *d_groupNodes, bool *d_nodeExists)
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
			if(d_nodeExists[node]){
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
__global__ void getSolutionSpaceVectorKernel0(int h_nNodes,double *outVector, double *inVector,
											double *d_x, double *d_y, double *d_z, double *d_CGPatch, 
											int *d_nodeGroupNumber, bool *d_nodeExists)
// r = W*mu
// Each thread is assigned to a node
{	
	register int node, group;
	register double xnode,ynode,znode;

	node = blockIdx.x*blockDim.x + threadIdx.x;
	if ((d_nodeExists[node]) && (node < h_nNodes)){
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
__global__ void getSolutionSpaceVectorKernel1(int h_nNodes,double *outVector, double *inVector,
											double *d_x, double *d_y, double *d_z, double *d_CGPatch, 
											int *d_nodeGroupNumber, bool *d_nodeExists)
// r = W*mu
// Each thread is assigned to a node
{	
	register int node, group;
	register double xnode,ynode,znode, u0, u1, u2, u3, u4, u5, u6, u7, u8;

	node = blockIdx.x*blockDim.x + threadIdx.x;
	if ((d_nodeExists[node]) && (node < h_nNodes)){
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
void enforceDirichletGPU(GPUData *gpuData,double *t)
{
	dim3 dimBlock(THREADS);
	int h_nNodes = gpuData->h_nNodes;
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	enforceDirichletKernel<<< dimGrid, dimBlock,0 >>>(h_nNodes, t, gpuData->d_free);
	cudaThreadSynchronize();
}
/*************************************************************************/
void permuteVectorGPU(WorkSpaceData *gpuWS,int N,double *mu,double *muP)
{
	dim3 dimBlock(THREADS);
	int nBlocks = N/(THREADS) + ((N%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	permuteVectorKernel<<< dimGrid, dimBlock,0 >>>(N, gpuWS->d_KPermuteVector,mu,muP);
	cudaThreadSynchronize();
}
/*************************************************************************/
void permuteTransposeVectorGPU(WorkSpaceData *gpuWS,int N,double *mu,double *muP)
{
	dim3 dimBlock(THREADS);
	int nBlocks = N/(THREADS) + ((N%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks); 
	permuteTransposeVectorKernel<<< dimGrid, dimBlock,0 >>>(N, gpuWS->d_KPermuteVector,mu,muP);
	cudaThreadSynchronize();
}
/*************************************************************************/
void getWorkSpaceVectorGPU(GPUData *gpuData, WorkSpaceData *gpuWS, double *mu, double *t, int h_nNodes)
{
	int THREADS_S = 8; //8 The magic number!
	dim3 dimBlock(THREADS_S);
	int nBlocks = (gpuWS->d_nGroups/THREADS_S) + ((gpuWS->d_nGroups%THREADS_S)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);
	int gType = gpuWS->d_workSpaceType;
	// Carry out mu = W'*t
	if (gType == DEFLATION_RIGID) {
		cudaFuncSetCacheConfig(getWorkSpaceVectorKernel0,cudaFuncCachePreferL1);
		getWorkSpaceVectorKernel0<<< dimGrid, dimBlock,0 >>> (gpuWS->d_nGroups, gpuWS->d_groupKey, mu, t, gpuWS->d_x,
			gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
			gpuWS->d_groupNodes,gpuData->d_nodeExists);
	}
	else {
		cudaFuncSetCacheConfig(getWorkSpaceVectorKernel1,cudaFuncCachePreferL1);
		getWorkSpaceVectorKernel1<<< dimGrid, dimBlock,0 >>> (gpuWS->d_nGroups, gpuWS->d_groupKey, mu, t, gpuWS->d_x,
		gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
		gpuWS->d_groupNodes,gpuData->d_nodeExists);
	}
	cudaThreadSynchronize();
}
/************************************************************************/
void getSolutionSpaceVectorGPU(GPUData *gpuData, WorkSpaceData *gpuWS, double *t, double *result, int h_nNodes)
{
	dim3 dimBlock(THREADS);
	int nBlocks = h_nNodes/(THREADS) + ((h_nNodes%THREADS)?1:0);
	nBlocks = (nBlocks > MAXBLOCKS)? MAXBLOCKS: nBlocks;
	dim3 dimGrid(nBlocks);   	
	int gType = gpuWS->d_workSpaceType;
	// Carry out mu = W'*t
	if (gType == DEFLATION_RIGID)
		getSolutionSpaceVectorKernel0<<< dimGrid, dimBlock,0 >>> (h_nNodes, t, result, gpuWS->d_x,
							gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
							gpuWS->d_nodeGroupNumber,gpuData->d_nodeExists);
	else
		getSolutionSpaceVectorKernel1<<< dimGrid, dimBlock,0 >>> (h_nNodes, t, result, gpuWS->d_x,
							gpuWS->d_y,gpuWS->d_z,gpuWS->d_CGPatch,
							gpuWS->d_nodeGroupNumber,gpuData->d_nodeExists);						
	cudaThreadSynchronize();
}
/*************************************************************************/
int WorkSpaceCGOnGPU(GPUData *gpuData, WorkSpaceData *gpuWS, int MaxIter, double CGRelResErr, bool h_nodeBasedAssembly)
{
	// Conjugate Gradient method with workspace
	
	int h_nNodes = gpuData->h_nNodes;
	int nGroups = gpuWS->d_nGroups;
	int gType = gpuWS->d_workSpaceType;
							
	double rho, alpha, beta, rho_new, normr0, normr;
	int niter, nDOF, nWorkSpace;
	double *r,*p,*a,*d, *t, *mu, *result, *f,*sol,*muP;
	Timer timer,totalTimer;

	totalTimer.start();
	nDOF = 3*h_nNodes;
	if (gType == DEFLATION_RIGID)
		nWorkSpace = 6*nGroups;//Workspace method
	else
		nWorkSpace = 9*nGroups;
		
	sol = gpuData->d_sol;
	f = gpuData->d_f;
	cublasAlloc( nDOF, sizeof(double), (void **) &r);	
	cublasAlloc( nDOF, sizeof(double), (void **) &p);	
	cublasAlloc( nDOF, sizeof(double), (void **) &a);	
	cublasAlloc( nDOF, sizeof(double), (void **) &d);

	
	cublasAlloc( nDOF, sizeof(double), (void **) &t);	// for workspace correction
	cublasAlloc( nWorkSpace, sizeof(double), (void **) &mu);
	cublasAlloc( nWorkSpace, sizeof(double), (void **) &muP);
	cublasAlloc( nWorkSpace, sizeof(double), (void **) &result);
	
	checkCUDAStatus("GPU Memory allocation error in CGW");
	
	// Performing conjugate gradient here with residual to start
	KuGPU(gpuData,sol,a,h_nodeBasedAssembly);// a = K*sol;
	zeaxpyGPU(h_nNodes,r,-1,a,1,f);// r = -a + f	
	cudaMemset(mu, 0, nWorkSpace*sizeof(double));
	getWorkSpaceVectorGPU(gpuData,gpuWS,mu,r,h_nNodes);// mu = W'*f


	permuteVectorGPU(gpuWS,nWorkSpace,mu,muP);
	cublasDtrsv('L','N','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
	cublasDtrsv('L','T','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
	permuteTransposeVectorGPU(gpuWS,nWorkSpace,muP,mu);

	getSolutionSpaceVectorGPU(gpuData,gpuWS,t,mu,h_nNodes);//sol = W*result

	//cudaMemset(result, 0, nWorkSpace*sizeof(double));
	//cublasDgemv ( 'N', nWorkSpace, nWorkSpace, 1,gpuWS->d_KLower, nWorkSpace, mu,1, 1, result, 1);//Solve result = Ktilde\mu;
	//getSolutionSpaceVectorGPU(gpuData,gpuWS,t,result,h_nNodes);//sol = W*result
	enforceDirichletGPU(gpuData,t);
	zeaxpyGPU(h_nNodes,sol,1,t,1,sol);//u = t + u;

	KuGPU(gpuData,sol,a,h_nodeBasedAssembly);// a = K*sol;
	zeaxpyGPU(h_nNodes,r,-1,a,1,f);// r = -a + f
	normr0 = cublasDdot(nDOF,r,1,r,1);
	normr = 10*normr0;
	//********************************************************	
	KuGPU(gpuData,r,t,h_nodeBasedAssembly);// t = K*r;
	cudaMemset(mu, 0, nWorkSpace*sizeof(double));
	getWorkSpaceVectorGPU(gpuData,gpuWS,mu,t,h_nNodes);// mu = W'*t

	permuteVectorGPU(gpuWS,nWorkSpace,mu,muP);
	cublasDtrsv('L','N','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
	cublasDtrsv('L','T','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
	permuteTransposeVectorGPU(gpuWS,nWorkSpace,muP,mu);

	getSolutionSpaceVectorGPU(gpuData,gpuWS,t,mu,h_nNodes);//sol = W*result

	enforceDirichletGPU(gpuData,t);
	zeaxpyGPU(h_nNodes,p,-1,t,1,r);// p = r - t
	//********************************************************	
	rho =  cublasDdot(nDOF,r,1,r,1);
	niter = 1;  	
	while (normr > CGRelResErr*normr0)   {
		KuGPU(gpuData,p,a,h_nodeBasedAssembly);// a = K*p;
		beta = cublasDdot(nDOF,a,1,p,1);//DOTPROD(a,p);		
		alpha = rho/beta;
		zeaxpyGPU(h_nNodes,sol,alpha,p,1,sol);//u = alpha*p + u;
		if (niter%500 == 0)	{ // restart CG with recomputing residual
			KuGPU(gpuData,sol,a,h_nodeBasedAssembly);// a = K*sol;
			zeaxpyGPU(h_nNodes,r,-1,a,1,f);// r = -a + f
		}
		else
			zeaxpyGPU(h_nNodes,r,-alpha,a,1,r);	//r = -alpha*a + r

		rho_new =  cublasDdot(nDOF,r,1,r,1);//rho = r'*r
		normr = (rho_new);
		KuGPU(gpuData,r,t,h_nodeBasedAssembly);// t = K*r;
		cudaMemset(mu, 0, nWorkSpace*sizeof(double));
		getWorkSpaceVectorGPU(gpuData,gpuWS,mu,t,h_nNodes);// mu = W'*f

		permuteVectorGPU(gpuWS,nWorkSpace,mu,muP);
		cublasDtrsv('L','N','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
		cublasDtrsv('L','T','N',nWorkSpace, gpuWS->d_KLower, nWorkSpace,muP, 1);
		permuteTransposeVectorGPU(gpuWS,nWorkSpace,muP,mu);

		getSolutionSpaceVectorGPU(gpuData,gpuWS,t,mu,h_nNodes);//sol = W*result

		enforceDirichletGPU(gpuData,t);
		zeaxpyGPU(h_nNodes,p,1,r,(rho_new/rho),p);//p = r + rho_new/rho * p;
		zeaxpyGPU(h_nNodes,p,-1,t,1,p);// p = -t + p
		rho = rho_new;
		niter++;
		
		if ((h_hexmeshGPU_verbose) && (niter%50 ==0)) printf(" %d	%e \n",niter,normr/normr0);
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

	if (h_hexmeshGPU_verbose) printf("Deflation: CG on GPU; nIter = %d; time = %0.3f (s)\n",niter,totalTimer.getElapsedTimeInSec());

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
void freeGPUWorkSpaceData(WorkSpaceData *gpuWS)
{
	cublasFree(gpuWS->d_nodeGroupNumber);
	cublasFree(gpuWS->d_x);
	cublasFree(gpuWS->d_y);
	cublasFree(gpuWS->d_z);
	cublasFree(gpuWS->d_CGPatch);
	cublasFree(gpuWS->d_KLower);
	cublasFree(gpuWS->d_KPermuteVector);
	gpuWS->d_nGroups = 0;
}
/****************************/
#endif
