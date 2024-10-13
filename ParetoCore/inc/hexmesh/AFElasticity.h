#ifndef HEXMESHGLOBALS_H
#define HEXMESHGLOBALS_H
#ifdef _WIN32
#include <windows.h>
#endif
#ifndef __APPLE__
#include <omp.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <stdarg.h>
#include <cmatrix.h>
#include <ctime>
#include <list>
#include <IntegerSet.h>
#include <Timer.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <AFElasticityGPUDefines.h>
#include <WorkspaceGPUdefine.h>


#define MAX_PARTS 100

#define ELASTICITY_DIRICHLET 0
#define ELASTICITY_NEUMANN 1
#define ELASTICITY_RETAIN 2
#define ELASTICITY_SLIDING 3
#define ELASTICITY_OUTPUT 4
#define ELASTICITY_TORQUE 5
#define ELASTICITY_PRESSURE_CYL 6
#define ELASTICITY_PRESSURE 7
#define ELASTICITY_BEARING_LOAD 8
#define ELASTICITY_REMOTE_LOAD 9

#define THERMAL_DIRICHLET 7
#define THERMAL_NEUMANN 8
#define THERMAL_CONVECTION 9
#define THERMAL_RADIATION 10

// The following is same as STL_MAX_GROUPS
#define	HEXMESH_MAX_FACE_LABELS		1000
#define MAX_LINEAR_SOLVES 2000

#define MIN_WORKSPACE_NODES 20
#define MIN_WORKSPACE_GROUPS 64
#define MAX_WORKSPACE_GROUPS 2000

using namespace std;
using namespace Eigen;

typedef std::array<double,3> Point3D;

bool dummyCallBack(double fractionCompleted);

extern void writeMessageToGUIWindow(const char format[],...);
extern void writeMessageToParetoLog(const char format[],...);
class AFElasticity {
private:

		double	*h_sol;// displacement solution at each node (u,v,w)
		double	*h_pseudoDensity; // for each element, a pseudo-density; used to represent voids (rho = 0)
		
		bool	h_simpMode;//Turn to true if SIMP mode, to turn off 0.5 cut-off

        double	h_deformationMax; // max deformation in static result,
		bool	isStaticResultValid;// true or false

		double	*h_solErr; // only valid when an exact solution is available

		double	*h_residual;// 3*h_nNodes; can be used for error predicting methods
		Timer	h_hexmeshTimer;// can be used for timing various routines

		double	h_N[8][8]; // shape functions values at 8 Gauss points of the 8 shape functions
        double	h_gradNxi[8][8],h_gradNeta[8][8],h_gradNmu[8][8];//gradients at 8 Gauss points (first index) of 8 shape functions
		double	h_gradNxi0[8],h_gradNeta0[8],h_gradNmu0[8]; //Gradients at the center, precomputed
		
		// Part and Material data
		int		h_nParts; // i.e., independent bodies with possibly distinct materials
		int		h_partToMaterialMap[MAX_PARTS]; // all parts could map to the same material, etc
		bool	h_partOptimize[MAX_PARTS]; // should this part be optimized?


		char    h_materialName[MAX_MATERIALS][256];// used for user convenience and project readability
        double  h_E[MAX_MATERIALS]; // Young's modulus
		double  h_nu[MAX_MATERIALS]; // Poisson's ratio
		double  h_Y[MAX_MATERIALS]; // Yield strength
		double  h_rho[MAX_MATERIALS]; // Density
		double	h_conductivity[MAX_MATERIALS]; // Thermal conductivity
		double	h_alpha[MAX_MATERIALS]; // thermal expansion
		double  h_specificHeat[MAX_MATERIALS]; //specific heat J/kgK
		double  h_materialPrice[MAX_MATERIALS]; //price

        double  h_bulkModulus[MAX_MATERIALS]; //derived from E and nu
        double  h_shearModulus[MAX_MATERIALS]; //derived from E and nu

		int		h_nTemplateElements;// how many distinct elements, geometry + material
								// 1 for a uniform hex grid with same material
								// affects h_KETemplates and h_METemplates

		double	h_DMatrices[MAX_MATERIALS*36]; // The structural D matrices (6,6) for each material, either computed from E and nu or assigned by user

		int		*h_templateID; // for each element, the corresponding template id for looking up Ke stiffness matrix

		double	*h_KETemplates;//  24*24*h_nTemplateElements;
		double	*h_METemplates;//  24*24*h_nTemplateElements;

		double	*h_KeNode;// 256*3*81, nodal stiffness, 256 possible combinations for presence/absence of 8 elements
						 // 27 neighboring nodes, i.e., 81 dof
						// only applicable for Voxel meshes
		double	*h_MeNode;// 256*3*81, nodal mass, 256 possible combinations for presence/absence of 8 elements
						 // 27 neighboring nodes, i.e., 81 dof
						//only applicable for Voxel meshes

		int		h_nMeshComponents;

        bool    h_usingPreconditioner; //default to false, helps only in SIMP mode

// for direct solver
        VectorXd                            h_solutionVector;// u,v,w for each node
        VectorXd                            h_forceVector;
        MatrixXd                            h_denseKMatrix;
        Eigen::SparseMatrix<double>         h_sparseKMatrix;
        SimplicialLDLT<SparseMatrix<double>> h_sparseKMatrixSolver;


//	for debugging
		bool		h_hexmesh_verbose;// if set to true, print progress for debugging stuff
		bool		h_deflation_verbose;// if set to true, print progress for debugging stuff

#ifdef COMPILE_GPUCODE
		//GPU control is private to this class
		
		void	 transferBCsToGPU(void);
		
		void	 transferKeToGPU(void);
		int		 performCGOnGPU(void);
		int		 findFirstEigenmodeOnGPU(double eigRelativeAccuracy,double *eigenValues,double **eigenModes);
		int		 findEigenRayleighCGOnGPU(int nEigmodes,double *eigenValues,double **eigenModes,int useInitialGuess,double relResidual,int maxIterPerMode);
		void	 getResultsFromGPU(void);
		void	 releaseGPUMemory(void);
#endif


protected:// inherited classes can access these variables

		double   h_Eexponent; //E = E0*(rho)^n; this is the n value
		double   h_Mexponent; //M = E0*(rho)^m; this is the m value
        double   h_pseudoDensityCutoff;// below this number, assume element does not exist

		int		h_nNodes;// total number of nodes
		int		h_nElems; // total number of elements
		int		h_maxNeighboringElements;// for each node, 8 for a uniform hex grid

		bool	h_nodeBasedAssembly; //set to true for node-based K*u, only valid for voxel meshes

		bool	*h_nodeExists; // for each node, is it outside or inside the current topology
		int		*h_neighborNodes; // for each node, all the 27 neighboring nodes, -1 if a node is absent, 14 position is self
								//  the 0th neighbor of all nodes are stored first, then the 1st neighbor and so on
								// Computed only for for uniform voxel data structure
		int		*h_nodeSignature; // for each node, and for each of the 27 neighbors, how many elems do they share?

		// for checking identical elements
		double h_elemSignatureTolerance;

        bool    h_keepAllMaterials; // when a new material is added, should we keep old material (Default: false)
		int		h_nMaterials;
		int		*h_partIdOfElem;// for each element, the material ID
		int		*h_materialIdOfElem; // for each element, the material ID, it helps to keep them seperate


        // for quadratic mesh ... special applications such as Navier Stokes
        int             h_nQuadraticNodes;
        int             **h_elem27Nodes;
		// For uniform voxel grid
		bool	h_voxelgrid;// set to true if all elements are voxels
		int		h_nelx,h_nely,h_nelz;// number of elements in background grid
		double	h_xMin, h_yMin, h_zMin;// Starting (min) location of the voxel mesh
		double	h_dx,h_dy,h_dz; /// used only for uniform grid meshes
		int		*h_gridNodeToRealNode; // grid node numbers are based on a uniform voxel, many don't
										// exist in the domain, the real node numbers are the actual
										//node numbers used in hexmesh
										// If a real node does not exist at a grid node, set to -1


        double	*h_x,*h_y,*h_z; // h_nNodes, (x,y,z) location of all corner nodes. Only needed for plotting if Ke is read directly
		double	h_modelScale; //used for plotting


		int		*h_nodeLabels; // each node is assigned a label, i.e., the number of the face group that the node lies on
		int		h_NumNodesWithLabel[HEXMESH_MAX_FACE_LABELS];// for each label type how many nodes
		int		h_maxLabel; // maximum value of node label (max number of boundary condition types);
		int		*h_elems; // 8*h_nElems, store the first nodes of all elements, then second nodes, etc
		int		*h_neighborElems; // h_maxNeighboringElements*h_nNodes, the element neighbors of a node; the 0th neighbor of all nodes are stored first, then the 1st neighbor and so on
		int		*h_nNeighboringElems; // for each node, how many valid neighboring elements for a given node,  8 for a uniform hex mesh

		
		int		*h_free;// Are (u,v,w) fixed or free; each component is encoded into a single int
						// 0 means all nodes are fixed
						// 1 means u is free, v & w are fixed
						// 2 means v is free, u & w are fixed
						// 3 means u and v are free, w is fixed
					    // 4 means w is free, u & v are fixed
						// 5 means u & w are free, v is fixed
						// 6 means v & w are free, u is fixed
						// 7 means all dof are free
	
		double	*h_output;//for compliance problems
		bool	*h_retain; // whether the node is to be retained or not
						  // useful for topology optimization

		double	*h_f; //  total nodal force (fx, fy and fz)  due to external traction and body force
		bool	h_isThereAcceleration;
		double  *h_bodyForce;// elem force (fx, fy and fz) per unit volume ... kept separately only for topological sensitivity

		bool	h_useEigenProjection; // use eigen modes if populated to solve static problem
		int		h_nEigenModes;// M number of eigenmodes currently populated
		double	*h_eigenValues; // M values
		double	**h_eigenModes;// M * 3*nNodes


		
		double	*(h_elemstrains[6]);// the 6 strains in the middle of each element
		double	*(h_elemstresses[6]);// the 6 stresses in the middle of each element
		double	*(h_nodestrains[6]);// the 6 strains at each node
		double	*(h_nodestresses[6]);// the 6 stresses at each node
		double	*h_nodevonmises;

		double	*h_dirichlet;// Dirichlet value (typically zero)


		bool	h_workSpaceInitialized;
		int			ts_currentIsoCounter;// for every succesful topology, this is incremented, starting with -1

		//below is used in conjugate gradient
		int		h_CGMaxIter;
		double	h_CGRelativeResErr;

		bool	*h_keepElem; // h_nElems, true means keep element during topoloy optimization
		
		//  Morphological Operations
        int		*h_StructuringElems; // for each element, neighboring elements
		int		h_StructuringSize;
		
		// results

		double	h_uvwMin[3],h_uvwMax[3]; 
		double	h_vonmisesMax,h_vonmisesMin;
		double	h_elemWithMaxStress;// which elem has maximum stress, also see below
		int		h_nodeWithMaxDeformation;// which node has the maximum deformation
		int		h_nodeWithMaxStress;// which node has the maximum stress

		// store max stress for each part, necessary for checking against yield of that part
		double	h_vonmisesMaxInPart[MAX_PARTS];

		// below is used by GPU (if applicable)
		bool	usingGPU;// if set to true, then CG and eigenvalue computations are carried out on GPU
		GPUData	h_gpuData;

		// for contour extraction
		int		*h_elemComponent;// h_nElems, once the optimization is completed, the mesh gets split into distuinct component

//Tesselation Data
		// for fast plotting purposes, stores fine tesselation of stl file
		int			h_numTesselationPoints;
		int			h_numTesselationTriangles;
		double		*h_tessellationNormals;//3*nTriangles
		double		*h_tessellationXYZ;//3*npoints
		int			*h_tessellationTriangles;//3*nTriangles

		// also store the displacement, stress  results
		double		*h_tessellationUVW;//3*h_numTesselationPoints
		double		*h_tessellationVonMises;//h_numTesselationPoints
		double		*h_tessellationTemperature;//h_numTesselationPoints
        double		*h_tessellationModalUVW;//3*h_numTesselationPoints

//CG Data
		int		h_nCGItersActual; //last CG iteration
		double	*h_cgNorm; // cg norm vs iterations
		int		h_nCGIterations[MAX_LINEAR_SOLVES]; // keep track of CG iterations
		int		h_nLinearSolves;//keep track of the number of linear solves



		void	 allocateAllMemory(void);
		void	 releaseCPUMemory(void);
		bool	 allocateNodeMemory(void);
		bool	 allocateElemMemory(void);
		void	 voxelWilsonH8(double dx,double dy, double dz, double E,double nu,double Ke[24][24]);
		void	 voxelDisplacementKE(double dx,double dy,double dz,double E,double nu,double KE[24][24]);
		void	 isotropicElementMass(double dx,double dy,double dz,double rho,double ME[24][24]);
		void	 isotropicBubbleElementMass(double dx,double dy, double dz, double rho,double Me[24][24]);
		void	 voxelDisplacementKEUnitCube(double E,double nu,double KE[24][24]);
        void     displacementKESingleGaussPoint(double E,double nu,double Ke[24][24]);
		void	 voxelHVCC8(double dx,double dy, double dz, double E,double nu,double Ke[24][24]);
        int		 find_word(FILE *filein,const char *word);
		void	 copy(double *x, double *y);
        void	 copy(double *x, double *y, int N);
		void	 axpy(double alpha,double *x, double *y);
        void	 axpy(double alpha,double *x, double *y,int N);
		void	 zeaxpy(double alpha,double beta,double *z,double *x, double *y);
        void	 zeaxpy(double alpha,double beta,double *z,double *x, double *y,int N);
		void	 scale(double alpha,double *x);
        void	 scale(double alpha,double *x, double *y);
		void	 ScaleByVector(double *z,double *x, double *y);
		void	 convertTo8Bits(int a,int bits[8]);
		
		double	 normalize(double *x, double *y);
		void	 performSingleKUonGPU(GPUData *h_gpuData);
		void	 ComputeDiagOfK(double *diag);
		void	 KuCPUGPU(double *x, double *y);
		void	 MuCPUGPU(double *x, double *y);
		
        void	 computeKeAndMeNodal(bool useBubbleFunctions);
		void	 displacementKE(double x[8],double y[8],double z[8],double *dmatrix,double Ke[24][24]);
		void	 HVCC8(double x[8],double y[8], double z[8],double E,double nu,double elemVol, double Ke[24][24]);
		void	 wilsonKE(double x[8],double y[8],double z[8],double *dmatrix,double Ke[24][24]);
		void	 voxelWilsonH8SoftShear(double dx,double dy, double dz, double E,double nu,double Ke[24][24]);
		bool	 taylorKE(double x[8],double y[8],double z[8],double *dmatrix,double Ke[24][24]);
		void	 displacementME(double x[8],double y[8],double z[8],double Me[24][24]);
		void	 hybridKE(double x[8],double y[8],double z[8],double *dmatrix,double Ke[24][24]);
		void	 postprocessAtElems(void);
		void	 postprocessAtNodes(void);
		void	 LanczosBasis(double *x,double mu,int m,double **Z);	
		void	 computeJacobiPreconditioner(double *preconditionerJacobi);
		
		void	 initializeRandomVector(double *x);
		void	 enforceDirichlet(double *x);
		void	 enforceDirichlet0(double *x);
		void	 initializeToZeros(double *x);
		void	 ReactionForceCPUNodeBased(double *r);
		void	 ReactionForceCPUElemBased(double *r);


// Buckling
		void	computeKgUBuckling(double *u, double *result);
		double  RayleighQuotientBuckling(double *x);

        int		 findElementsAlongDirection(int nx,int ny,int nz,int dir,int *elements);
		int		 findNodesAlongDirection(int nx,int ny,int nz,int dir,int *nodes);
		void	 improveCongruenceSnapToTemplate(double allowedNodeMovement,int nPasses);
		void	 improveCongruenceAveraging(double allowedNodeMovement,int nPasses);
		void	 improveCongruenceSweep(double allowedNodeMovement,int nPasses);
		void	 bubbleVariables(double u[8],double v[8],double w[8], double Lx[3],double Ly[3],double Lz[3]);
		int		 solveLinearSystemViaEigenProjection(void);
		double	 RayleighQuotient(double *x);
		int		 findDuplicateIsotropicMaterial(double E, double nu,double rho,double K, double alpha,double specificHeat);
		void	 getKeNode(double *KeNode);
		void	 getMeNode(double *MeNode);
		

		void	 setKeTemplates(double *KeTemplates);
		void	 setMeTemplates(double *MeTemplates);
		void	 allocVector(int N,double **x);
		void	 allocMatrix(int N,int M,double ***A);
		void	 freeMatrix(int N,double **A);
		double	 Norm(double *x);// x is assumed to of length h_nNodes
		double	 Norm(double *x, int length);
		void	 setNodeSignatures(int *nodeSignatures);// used
		void	 releaseKeAndMeNodal(void);
		double	 computeMaxVonMisesAllGP(int elem);
		void	 dihedralAngles(int elem, double *angles);
		void	 computeFieldGradientsInElemWithBubble(int elem, int GP, double uGrad[3], double vGrad[3],double wGrad[3]);
		void	 computeFieldGradientsAtNodeWithBubble(int node,double uGrad[3], double vGrad[3],double wGrad[3]);
		void	 computeFieldHessianInElem(int elem,  double uHessian[3][3], double vHessian[3][3],double wHessian[3][3]);
		void	 getBMatrixVoxelAtCenter(double B[6][24]);
		void	 computeBucklingKgVoxel(int elem,double Kg[24][24]);
		void	 accurateStressEstimator(int elem);

		void	 computeGrad_Bubble(double xi,double eta, double mu, double gradNxi[3],double gradNeta[3],double gradNmu[3]);
		void	 computeCrossDerivatives_H8(double xi,double eta, double mu, double Nxieta[8],double Nximu[8],double Netamu[8]);
		void	 computeSecondDerivatives_Bubble(double xi,double eta, double mu,  double Nxixi[3],double Netaeta[3],double Nmumu[3]);
		void	 computePointInElem(int elem,int gp,double gaussPtXYZ[3]);

        //private algorithms
        bool	 performCG(int *niter);
        bool	 performPCG(int *niter);
        void	 enforceSliding(double *x);
        void	 KProjection(int m,double **Z,double **Kz);
        void	 MProjection(int m,double **Z,double **MZ);
        void	 RRSubspace(int subspaceSize,double **S,double **X,double *L,double **Y);
        void	 MatrixMatrixMultiply(int L, int M, int N, double **A,double **B, double **C);

        void	 KuCPUElemBased(double *x,double *y);
        void	 MuCPUElemBased(double *x,double *y);
        void	 KuCPUNodeBased(double *x,double *y);
        void	 MuCPUNodeBased(double *x,double *y);
        void	 swap(double *v, int *index, int i, int j);
        void	 swap(double *v, int i, int j);
        void	 transferAFElasticityToGPU(void);
        void	 improveCongruence(double allowedNodeMovement,int nPasses);
        void	 removeForcesOnHangingNodes(void);
        double	 getPseudoDensityGridElem(int nx,int ny,int nz);
        void	 computeIsotropicDMatrix(int matlIndex,double E,double nu);

public:
		//Constructor
				 AFElasticity();
		void	InitializeAllAFElasticityVariables(void);
	 	void	removeAllMaterials(void);	

		// Hexmesh construction, input  and output
		bool	 refineAndDumpVoxelFileBinary(const char *fileName);
        bool	 dumpMeshBinary(const char *fileName);
        bool	 dumpMeshBinaryNonVoidElements(const char *fileName);
        bool	 dumpMeshBinaryWithMapping(const char *fileName,int *nodeMapping);
		bool	 dumpIsoSurfaceFromMesh(const char *fileName);
        bool     dumpSIMPDensityBinary(const char *fileName);
		bool	 readTessellationFile(const char *fileName);
		int		 readVoxelFile(const char *fileName);
        bool	 readVoxelFileBinary(const char *fileName, bool disregardPseudoDensity = false);
        bool     readSIMPDensityBinary(const char *fileName);
		int		 readAbaqusInpFile(const char *fileName);
		int		 readAbaqusRPTFile(const char *fileName);
		int		 readParetoMeshFile(const char *fileName);
		void	 readQuadMeshAndSweep(const char *fileName,double sweepLength);
		void	 readGen4uQuadMeshAndSweepAlongZDirection(const char *fileName,double sweepLength);
		bool	 laplaceSmoothVoxelMesh(double eta);

		// Hexmesh specific examples 
		void	 createEdgeLoadedProblem(int nelx,int nely,int nelz);
		int		 generateCuboidMesh(int nElems,double boundingBox[3][2]);
        bool     generateCuboidMeshWithMapping(int nElemsDesired, double boundingBox[3][2]);
		void	 fixX0Plane(void);
		void	 applyUnitTipLoad(void);
		void	 applyUnitEdgeLoad(void);
		
		// Enabling and disabling Node based SpMV
		void	 enableNodalSpMV(void);
		void	 disableNodalSpMV(void);

		// Enabling and disabling GPU
		void	 enableGPU(void);
		void	 disableGPU(void);

		// Enabling and disabling Dwflation
		void	 enableDeflation(int ws_deflationType = 1);
		void	 enableDeflation(int ws_deflationType,int nGroups);
		void	 disableDeflation(void);

        bool	isPreconditionerEnabled(void);
		void	enablePreconditioner(void);
		void	disablePreconditioner(void);

		// Enabling and disabling verbose printing
		void	 enableHexmeshVerbose(void);
		void	 enableHexmeshGPUVerbose(void);
		void	 disableHexmeshVerbose(void);
		void	 disableHexmeshGPUVerbose(void);

		void	enableSimpMode(void);
		void	disableSimpMode(void);

        // for microstructure support
        void     assemblePeriodicForce(int fVector);
        void     displacementFE(double x[8],double y[8],double z[8],double *dmatrix,double Fe[24][6]);

		/// Get Utilities
		double	 getElemPseudoDensity(int elem);// pseudoDensity is used in topology optimization to 'kill' elements
		void	 getPseudoDensity(double *pseudoDensity);
		int		 getElemRemovalOrder(int elem);// decided by topology optimization removal
		int		 getNumNodes(void);// num of nodes in mesh
		int		 getNumElems(void);// num of elements in mesh
		void	 getElemNodes(int elem,int *nodes);// for a given element, the 8 nodes
		void	 getSolutionVector(double *sol);// solution vector of 3*numNodes
		void	 getForceVector(double *f);// force vector of 3*numNodes
		void	 getNodeDeformation(int node,double *u,double *v,double *w,double *vonmises);// at a node the (u, v, w) stress
		bool	 getElasticitySolutionAt(double pt[3],double uvw[3],double stresses[6],double *vonMises);// at a point find the displacement and stress
        bool	 getModalSolutionAt(double pt[3],int eigenMode,double modaluvw[3]);// at a point find the displacement and stress
		bool	 getElasticitySolutionInCenterOfElem(int elem,double uvw[3],double stresses[6],double *vonMises);// at a point find the displacement and stress
		void	 getElemNodeLocationWithDeformation(int elem,double x[8],double y[8],double z[8],double scale);// add the deformation and return the 8 node locations
		void	 getElemCenter(int elem,double center[3]);// the center of element
		void	 getElemNodeDeformation(int elem,double u[8],double v[8],double w[8]);// deformation at the 8 nodes
		void	 getElemStrains(int elem,double strains[6]);// get strains in the middle of element
		void	 getElemStresses(int elem,double stresses[6]); //get stresses in the middle of element
		void	 getElemNodesStrains(int elem,int component,double strains[8]);// component = 0 is exx, etc. at all 8 nodes
		void	 getElemNodesStresses(int elem,int component,double stresses[8]);// see above
		double	 getElemVonMisesStress(int elem);// at the center
		int		 getElemMaterialID(int elem);
		void	 getAllElemVonMisesStresses(double *vonMises);// vector of num of elements
		void	 getElemNodeDisplacementErrors(int elem,double uerr[8],double verr[8],double werr[8]);
		void	 getElemNodesVonMisesStresses(int elem,double stresses[8]);
		double	 getModelScale(void);// the largest geometric dimension (among x,y,z) of mesh
		int		 getElemTemplateID(int elem);// elements with identical geometry have the same template ID
		int		 getNumTemplateIDs(void);// how many geometrically distinct elements are there
		double	 getDeformationMax(int *nodeWithMaxDeformation = NULL);// max deformation across all nodes
		void	 getNeumannForce(int node,double *force);// the three components
		void	 getNodeXYZ(int node,double p[3]);// The (x,y,z) location of node
		void	 getNodesStrains(int node,double strains[6]);
		double	 getForceNorm(void);
		double	 getMinVonMisesStress(void);
		double	 getMaxVonMisesStress(int *whichNode=NULL);
		void	 getElemNodesForceNorm(int elem,double forces[8]);
		void	 getElemNodeLocation(int elem,double x[8],double y[8],double z[8]);
		void	 getNodeLocationWithDeformation(int node,double pt[3],double scale);
		double	 getElemVolume(int elem);
		void	 getCompliantOutput(double *output);
		double	 getMeshElementSize(void);
		double	 getMeshVolume(void);
        double   getPriceOfMaterial(int matl);
        double   getMaterialPriceOfPart(int part);
		void	 getNodeExists(bool *nodeExists);
		void	 getNeighborNodes(int *neighborNodes);
        int		 getAllNeighborNodesOfNode(int node,int *neighborNodes);
		int		 getNeighborNodesOfNode(int node,int *neighborNodes);
		void	 getNodeSignatures(int *nodeSignatures);
		int		 getLinearSolvesCounter(void);
		void	 getCGIterations(double *cgIterations);
		int		 getLastCGIteration(void);
		void	 getElemDensityAtElemNodes(int elem,double pseudoDensity[8]);
		void	 getReactionForces(double *reactionForces);
		int		 getCGNorm(double *nrm);
		bool	 getElasticMaterialProperties(int matID,double *E,double *nu);
		int		 getNumberOfParts(void);
		int		 getMaterialIDofPart(int part);
		void	 getMaterialNameOfPart(int part,char materialName[]);
		void	 getMaterialPropertiesOfPart(int part,double materialProperties[7],char materialName[]);
		void	 getMaterialOfPart(int part,double *E, double *nu,double *rho, double *K, double *alpha, double *specificHeat,char materialName[]);
		void	 computeElemForces(int elem, double elemForce[24]);
		void	 getElemSolution(int elem,double sol[24]);
		double	 getElapsedTimeInSec(void);
		double	 getElapsedTimeInMilliSec(void);
        void	 getCenterOfGeometry(double center[3]);
		void	 getBoundingBox(double boundingBox[3][2]);
        int      getNeighboringElemsOfNode(int node,int *elems);
        int      getNearbyNodes(int node,int maxNumNodes,int *nodes);
        double   distanceOfPointFromLine3D(const double Pt[3],const double Pt1[3],const double Pt2[3],double *PtClosest);
		bool	 isPointInsideCylinder( const double startPt[3], const double endPt[3], double r, const double p[3]);
        bool	 isPointInsideCappedCylinder( const double startPt[3], const double endPt[3], double r, const double p[3]);
		bool	 isPointInsideBoundingBox(double boundingBox[3][2],double pt[3]);
        int      getNodesInsideCylinder(const double startPt[3], const double endPt[3], double r,int maxNumNodes,int *nodes);
        int      getOneElemInsideBoundingBox(double boundingBox[3][2]);
        int      getElemsInsideBoundingBox(double boundingBox[3][2],int maxNumElems,int *elems);
        int      getElemsWithinSphereCenteredAtElem(int elem, const double dist,const int maxNumElems,int *elemsIn);
        int      getElemsWithinCylinderCenteredAtElem(int elem, const double xyRadius,double zHeight,const int maxNumElems,int *elemsIn);
        int		 getElemsInsideSphere(const double center[3], const double r,const int maxNumElems,int *elems);
        int		 getElemsIntersectingLine(const double startPt[3], const double endPt[3],const int maxNumElems,int *elems);
        int      getElemsWithinCubeCenteredAtElem(int baseElem, const double cubeHalfWidth,const int maxNumElems,int *elemsIn);
        int		 getElemsInsideCylinder(const double startPt[3], const double endPt[3],  const double r,const int maxNumElems,int *elems);
		int		 getElemsInsideCylinderBelowZPlane(const double startPt[3], const double endPt[3], const double zPlane, const double r,const int maxNumElems,int *elems);
        int      getElemsInsideLongBox(const double startPt[3], const double endPt[3], double boxHalfWidth,int maxNumElems,int *elemsIn);
		void	 getUVWMinMax(double uvwMin[3],double uvwMax[3]);
		int		 getNumMaterials(void);
		void	 getKeTemplates(double *KeTemplates);
		void	 getMeTemplates(double *MeTemplates);
		int		 getPartIdOfElem(int elem);
		int		 getMaterialIdOfElem(int elem);
		double	 getThermalConductivityOfElem(int elem);
		double	 getSpecificHeatOfElem(int elem);
		double	 getDensityOfElem(int elem);
		void	 getDMatrix(int matlIndex,double D[36]);
		int		 getNodeLabel(int node);
		void     getGridIndicesOfElem(int elem,int gridIndices[3]);
		void	 getElemSize(double size[3]);
		int		 getNumberOfElementsInOptimizedParts(void);
		double   getCurrentVolumeFractionOfOptimizedParts(void);
		double   getCurrentVolumeFraction(void);
		bool	 isStaticFEAValid();
		int		 getNeighboringElementsOfElem(int elem,int *neighboringElemsOfElem);
		bool	 doesElementHaveManifoldBoundary(int elem);
		int		 getNumOfNonManifoldElements(void);
		int		 removeNonManifoldElements(void);
		int		 getAllNeighboringElementsOfElem(int elem,int *neighboringElemsOfElem);
        int		 getNeighboringElementsOfElemRecursive(int elem,int *neighboringElemsOfElem,int maxElems,int nRecursions);
		int		 getElemsWithinDistance(double center[3],double radius,int *elemList);
		double   findMaxOverElements(double *x);
		double   findMinOverElements(double *x);
		double   findMaxOverNodes(double *x);
		double   findMinOverNodes(double *x);
		int		 findNodesInBoundingBox(double boundingBox[3][2], int *nodes,bool checkForActiveNodes = false);
		int		 findElemsInBoundingBox(double boundingBox[3][2], int *elems);
		bool	 isDesignPastYield(int *partId,double safetyFactor = 1);
		bool	 getYieldStressOfMaterial(int matlID,double *yield);
		double	 getMaxStressInPart(int part);
		int		 getRealNodeFromGridNode(int gridnode );
        bool	 setKeOfTemplateElem(int templateElem, double Ke[24][24]);
        bool	 getKeOfTemplateElem(int templateElem, double Ke[24][24]);
// Tesselation get
		int		 getNumTessellationPoints(void);
		int		 getNumTessellationTriangles(void);
		bool	 getTessellationPoint(int node,double pt[3]);
		bool	 getTessellationTriangleNodes(int tri,int nodes[3]);
		bool	 getTessellationTriangleNormal(int tri,double normal[3]);
		bool	 getTessellationNodeDisplacement(int node,double uvw[3]);
        bool	 getTessellationNodeModalDisplacement(int node,double uvw[3]);
		bool	 getTessellationNodeVonMises(int node,double vonMises[1]);
		bool	 getTessellationNodeTemperature(int node,double temperature[1]);

		bool	 getTessellationVerticesOfTriangle(int tri,double x[3],double y[3],double z[3],double triNormal[3]);
		bool	 getTessellationElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3],double vonMises[3]);
        bool	 getTessellationModalSolutionOfTriangle(int tri,double u[3],double v[3],double w[3]);

		int		 getNumberOfMeshComponents(void);
		int		 getMeshComponentOfElem(int elem);
		bool	 doesEveryPartHaveAnElement(void);
		bool	 findConnectedComponentsOfMesh_Material(bool reset=true);
		bool	 findConnectedSingleLargestComponentOfMesh(bool reset,double cutoff);
		bool	 findConnectedComponentsOfMesh(bool reset = true);
        int      countNumElemsIn(void);

		// Set Utilities
        void     removeInherentStrainFromElements(int nElems,int *elems);
        void     addInherentStrainToElements(double strain[3], int nElems,int *elems);
        void     setLogFilePtr(FILE *logFilePtr);
		void	 setCGRelativeResError(double cgRelResError);// tolerance error for CG, default of 1e-9
		void	 setCGMaxIter(int MaxIter);// max iterations for CG, default of 20000
		void	 initializePseudoDensity(void);// for each element
		void	 setPseudoDensity(double *pseudoDensity);// for each element
		void	 setPseudoDensityOfElem(int elem,double pseudoDensity);// for one element
		void	 setNodeExists(bool *nodeExists);// when elements get deleted during topopt, force node is part of current mesh
		void	 setNeighborNodes(int *neighborNodes);
        bool	 setNodeXYZ(int node,double pt[3]);
		void	 setSolutionAtAllNodes(double *sol);//overrides computed displacements
		void	 setVonMisesAtAllNodes(double *stress);//overrides computed stresses
		void	 setForceVector(double *f);
		void	 setElasticityAcceleration(double acceleration[4]);// 3 linear and 1 radial (radians per sec)
		void	 addUserForceAtNodes(double *force);
		void	 setBCOnNodesWithLabel(int nodeLabel,int BCType[3],double BCValue[3]);
        void	 fixNodesOnZ0Surface(void);
		void	 retainNodesWithLabel(int nodeLabel);
		void	 setBCOnSingleNode(int node,int BCType[3],double BCValue[]);
		void	 setElementToVoid(bool restart,double *topSensElem, double TSCutoff,int nElemsToRemove);
		void	 setPartToBeOptimized(int partNum);// should not be topology optimized
        void	 setMaterialIndexOfPart(int partNum,int materialIndex);// change material when multiple materials are present
		void	 setPartToNonDesign(int partNum);// should not be topology optimized
		bool	 shouldPartBeOptimized(int partNum);// should the part be topology optimized?
		bool	 setIsotropicMaterialOfPart(int partNum = 0, double E=2.1e11, double nu=0.28,double rho=7700,double K=16,double alpha=6.e-6,double specificHeat = 452, const char *materialName = "AlloySteel");
        bool	 setIsotropicMaterialPropertiesOfPart(int partNum, double materialProperties[7],const char *materialName,double materialPrice = 1);

        /* E = materialProperties[0];
		nu = materialProperties[1];
		Y = materialProperties[2];// Yield strength
		rho = materialProperties[3];
		K = materialProperties[4];
		alpha = materialProperties[5];
		Cp = materialProperties[6];*/

        void     keepUnusedMaterial(bool state);
		void	 resetLinearSolvesCounter(void);
		void	 setPartIdOfElem(int partId, int elem);
		void	 setMaterialIdOfElem(int matIndex,int elem);
		void	 setTemplateIdOfElem(int templateId, int elem);
		void	 offsetMeshBy(double offset[3]);
		
		bool	 setDMatrix(int matlIndex,double D[36]);
		void	 setKeNode(double *KeNode);
		void	 setMeNode(double *KeNode);
		void	 setMaxVonMisesStress(double vonMises);
		void	 createElementMatricesTemplates(bool useBubbleFunctions= true);
		void	 setSolutionError(double *solErr);
		void	 setSolutionOnNode(int node, double sol[3]);
		void	 setVonMisesAtNode(int node,double vonMisesNode);
		void	 setMeshNodeLocation(int node,double pt[3]);
		void	 setEexponent(double vlaue);
		void	 deleteStaticFEAResults();
		void	 removeDeadMaterial(void);

		void	 setStructuringSize(int n);
		void	 setPseudoDensityLimits(double minValue,double maxValue);
		void	 startTimer(void);
		void	 stopTimer(void);
        void     computeEquivalentD(double x[8],double y[8],double z[8],double *dmatrix,double *disp,double *equivDmatrix,double *gradDelem);

        // for quadratic mesh... specific applications
        bool     createQuadraticMesh(void);
        bool     getElemQuadraticNodes(int elem,int elemNodes[27]);

		void	 clearNodeExists(void);	
		void	 removeHangingElems(void);
        bool     removeDisconnectedElementsUsingSeedElement(int seedElem);
		void	 safeFree(void *ptr);
		double	 dotprod(double *x, double *y);
        double	 dotprod(double *x, double *y, int N);
        bool	 isEvolvingMeshConnected(void);
		bool	 isMeshConnected(void);
		void	 assignNodeLabel(int node, int nodeLabel);
		void	 printNodeVector(double *x);
		int		 isNodeOnCorner(int node);
		int		 isNodeOnEdge(int node);
		bool	 isNodeOnBoundary(int node);
        bool     isElemInInteriorOfPart(int elem, int part);
		bool	 isElemInInterior(int elem);
		bool	 isNodeInInterior(int node);
		bool	 isNodeOnEvolvingBoundary(int node);
		bool	 isNodeWithinEvolvingMesh(int node);
		bool	 isElemOnEvolvingBoundary(int element);
		void	 printDMatrix(int matl);
		void	 computeKeNodalFromKeTemplate(void);
		void	 KuCPU(double *x,double *y);
		void	 MuCPU(double *x,double *y);
		double	 computeElemInnerProduct(int elem,double u1[24],double u2[24]);
		void	 kernelSmoothener(double *elemField,double relativeBoxSize = 0.01);
		bool	 pushInitialStaticSolutionToTessellation(void);
        bool	 pushInitialModalSolutionToTessellation(double *eigenMode);

		//Morphological operations
		void	findAllNeighboringElems();
		void	findNeighboringElems(int elem);
		void	findNeighboringElemsNew(int elem);
		void	findNeighboringElemsOld(int elem);
		int		findClosestIndexInArray(double xc,double yc,double zc,double centers[27][3],int nIndices,double tolerance);
		void	erodeMesh();
		void	dilateMesh();
		void	openingOnMesh();
		void	closingOnMesh();

		bool	 getNormalAtNode(int node,double normal[3]);
		void	 computeGrad_H8(double xi,double eta, double mu,  double gradNxi[8],double gradNeta[8],double gradNmu[8]);
		bool	 isMeshVoxelized(void);
		bool	 isNodeFree(int node);
		bool	 doesElemHaveConcaveEdge(int elem);
		bool	 isNodeOnConcaveEdge(int node);
		bool	 doesElemHaveConcaveNode(int elem);	
		bool	 isNodeRetained(int node);
		bool	 isThereForceOnNode(int node);
		bool	 isNodeOutput(int node);
		void	 bubbleSortIncreasing(double *numbers, int *sortIndex,int array_size);
		void	 bubbleSortIncreasingDualIndex(double *numbers, int *sortIndex1,int *sortIndex2,int array_size);
		void	 computeElemStress(double strains[6], double stresses[6]);
		void	 extrapolateFieldAtPoint(double pt[3],int closestElem, double uvw[3]);
		void	 generalizedEigen(int N,double **AMatrix,double **BMatrix,double **XMatrix,double *LVector);
		void	 computeDeformationMax(void);
		void	 initializeSolution(void);
		bool	 isSpMVMethodNodeBased(void);
        int      findCommonElements(int n1,int *v1, int n2,int *v2, int *vCommon);
		void	 computeDerivativesInElem(int elem);
		double	 computeMaxVonMisesAtCenter(int elem);
		void	 computeStrainWithBubble(int elem, double xi, double eta, double mu, double strains[6]);
		double	 computeCompliance(void);
		void	 computeN(double xi,double eta, double mu, double N[8]);
		void	 computeFieldGradientsAtNode(int node,double uGrad[3], double vGrad[3],double wGrad[3]);
		void	 computeFieldGradientsInElem(int elem,int gaussPoint, double uGrad[3], double vGrad[3],double wGrad[3]);
		void	 smoothenElemField(double *elemfield,int nIter);
		void	 resetBC(void);
		int		 getClosestNode(double x,double y,double z);
		int		 findElemWithCenter(double x,double y,double z,double relativeTol = 0.25);
        int		 findElemWithCenterAndNeighborOf(double x,double y,double z,int elem, double relativeTol = 0.25);
		int		 findNonVoidElemClosestTo(double x,double y,double z,double relativeTol = 0.5);
		int		 findNonVoidNodelosestTo(double x, double y, double z, double relativeTol = 0.5);
		bool	 removeDisconnectedElements(double cutoff = 0.5);

        void     computeDistanceOfNodesToEvolvingBoundary(double *distanceToBoundary);

		int		 getMaxNodeLabel(void);
		bool	 doesNodeExist(int node);
		void	 quickSort(double *v, int lo0, int hi0);
		void	 quickSortNonrecursive(double *v, int nEntries);
		void	 quickSort(double *v, int *sortIndex, int lo0, int hi0);
		void	 shellSort(double v[], int nEntries);
		void	 populateEigen(int nEigenValues, double *eigenValues,double **eigenModes);
		void	 releaseEigen(void);
		double	 getFirstEigenValue(void);
		void	 setUseEigenProjection(bool useEigenProjection);

		// Congruence algorithms
		void	 discardTemplates(void );
		int		 getTemplateId(int elem);
		int		 getNumTemplateElements(void);
		void	 snapElementToTemplate(int elem,double templateXYZ[8][3],double allowedNodeMovement);
		void	 analyzeHexElement(int element ,double *area,double *vol, double *inertia,double *principalInertia);
		void	 setElementSignatureTol(double tol);
        void     computeKeOfTemplatesSinglePointGauss(void);
		void	 computeKeOfTemplates(bool useBubbleFunctions = true);
		void	 computeMeOfTemplates(int useBubbleFunctions);
        void	 forceMaxTemplates(void);
		void	 findTemplates(int minTemplates=1);
		void	 inertiaHexElement(int elem, double *inertia, double *principalInertia);
		void	 performSingleKUonCPU(double *y = NULL);

		//Auxilary Computation
		void	 findNeighboringElemsOfNodes(void);
		void	 findNeighboringNodesOfNodes(void);
		void	 postprocess(void);
        double   distanceOfPointFromLine3D(double *Pt,double *Pt1,double *Pt2,double *PtClosest);

		void	 computeStrain(int elem, double xi, double eta, double mu, double strains[6]);
		void	 KuCPUSingleElem(int elem,double *xe,double *ye);
		void	 smoothenVonMisesStress(int nSmoothingIterations);//smoothen the stress field in topopt
		void	 smoothenElasticityNodalField(double *nodalField,int nSmoothingIterations);// smoothen the displacements fields 
		void	 smoothenScalarNodalField(double *nodalField,int nSmoothingIterations);// smoothen scale fields 
		
		// Main algorithms (CPU or GPU depending on whether GPU is enabled or not) 
		bool	 solveElasticityStatic(void);    
		bool	 solveElasticityModal(int nEigenModes,double *eigenValues=NULL,double **eigenModes=NULL,double eigRelativeAccuracy = 0.01,double timeOut = 3600);
        int		 findEigenRayleighCG(int nEigenModes,double *eigenValues,double **eigenModes,bool useInitialGuess,double relResidual,int maxIterPerMode);
		bool	 solveBucklingModal(int nEigenModes,double *eigenValues,double **eigenModes,double eigRelativeAccuracy=0.05,double timeOut = 3600.0);
		bool	 solveElasticityModalOld(int nEigenModes,double *eigenValues,double **eigenModes,double eigRelativeAccuracy);

        // direct solver utilities
        bool    assembleElasticityKMatrix(void);
        bool    assembleElasticityKMatrixGeneric(void);
        bool	 solveElasticityStaticDirect(void);


        //  Large deformation Neo-Hookean
        bool    solveLargeDeformationDisplacementTracking(int nNonlinearSteps, int maxIter,
                                                          double tol,
                                                bool(*callBackFunction)(double fractionComplete,double *reactionForces));
        bool    solveLargeDeformationForceTracking(int nNonlinearSteps, int maxIter, double tol,
                    bool(*callBackFunction)(double fractionComplete,double *reactionForces));
        void    assembleNeoHookeanStiffness(double *tempForce);
        void    KENeoHookean(int elem,double Ke[24][24],double fElem[24],TCLMatrix eyeMatrix);
        void    IsottropicCMatrix(double K,double mu,double C[3][3][3][3], TCLMatrix eyeMatrix);
        void    NeoHookeanCMatrix(double K,double mu,TCLMatrix B, double JF,double C[3][3][3][3], TCLMatrix eyeMatrix);
        void    KirchoffStress(double K,double mu,TCLMatrix B, double JF,double stress[3][3], TCLMatrix eyeMatrix);


		//Printing
        void     dumpKElem(double KElem[24][24]);
		void	 dumpUVW(char *fileName);
		void	 printMaxMinUVW(void);
		void	 printMeshSummary();
		void	 printSolverOptions();
		void	 printAnalysisSummary();
		void	 dumpDeltaSigma(char *fileName,double offset[3]);
		void	 dumpDeltaSigmaMax(char *fileName);
		void	 printVector(int N, double *x);
		void	 printMatrix(int N,int M, double **A);
		int		 dumpVoxelFile(char *fileName);
		int		 dumpMeshAsAbaqusInpFile(char *fileName);
        void	 dumpParetoMesh(const char *fileName);
		void	 printPseudoDensityRange(void);
		void	 printWorkSpaceGroupData(void);

		 ~AFElasticity();
		


#ifdef COMPILE_GPUCODE
protected:
		int		 performCGWOnGPU(void);
		void	 transferWorkSpaceToGPU(void);
		void	 transferForcesBCToGPU(void);
#endif

protected:

		int				*ws_nodeGroupNumber; //  groupNumber of node
		int				*ws_groupSortedNodes; // size: h_nNodes, all nodes belonging to group 0 are first, then group 1
		int				*ws_groupEndNodeIndex; // end location for each group in the above sorted list
		int				*ws_groupCount;//how many nodes in each group
		int				*ws_anyNodeInGroup;//any valid node in each group, to start various searches
		int				ws_nGroupsDesired;// desired
		int				ws_deflationType; // Type of deflation, see above for various type 
		int				ws_nodeAssignmentMethod; // Type of node assignment
		int				ws_nGroups;//includes empty groups, if any
		int				ws_nEmptyGroups;// no nodes 
		int				ws_maxNodesInAnyGroup;//useful for checking full groups
		int				WorkSpaceTransferredToGPU;
		int				ws_DCGMaxIter;//max iterations for DCG
		WorkSpaceData	gpuWS;
		double			*h_CGPatch;

		MatrixXd		denseWKW;

		Eigen::SparseMatrix<double>			sparseWKW;
		MatrixXd							sparseWKW_Lower;
		VectorXd							sparsePermuteVector;
		SimplicialLLT<SparseMatrix<double>> sparseSolver;


	
		//Genesis file variables
		int h_num_dim; // number of dimensions
		int h_num_elem_blk; //number of element-blocks
		int h_num_node_sets; //number of node-sets
		int h_num_side_sets; //number of side-sets


		void	 gridMethod(void);		
		void	 assignNodesToGroupsBoxMethod(void);	
		bool	 findGroupToSplit(int *groupToSplit);
		bool	 splitAGroup(int groupToSplit,int direction=-1);
		int		 createAnEmptyGroup(void);
		int		 findAnEmptyGroup(void);
		bool	 removeSmallGroups(void);
		void	 sortGroupNodes(void);
		void	 findCGOfPatches(void);
		bool	 isGroupConnected(int group);
		void	 splitGroupIfDisconnected(int groupToSplit);
		void	 fillEmptyGroups(void);

		bool	 computeSparseWKW();
		void	 KreductionRBM();
		void	 KreductionPlaneStrain();
		void	 KreductionTranslate();
		void	 KreductionBeam();
		void	 KreductionThinPlate();
		void	 KreductionEP1();
		void	 KreductionEP2();
		void     elementWKW(int elem) ;
		void	 getWorkSpaceVector(double *inVector,VectorXd &outVector);
		void	 getSolutionSpaceVector(VectorXd &inVector, double *outVector);
		bool	 performCGW(int *niter);
		bool	 performPCGW(int *niter );// deflation with preconditioner option
		int		 solveWKW(double *mu, double *result );
		void	 printKTilde(void);
		void     removeKTildeSingularities();
		void	 ReleaseWorkSpace();




public:
		void	 setNodeAssignmentMethod(int method);
		bool	 CreateDeflationGroups(void);
		bool	 InitializeDeflationForElasticity(void);
		int		 getWorkspaceGroup(int node);
		int		 getNumWorkspaceGroups(void);
		int		 getNumEmptyGroups(void);
		int		 getNumNodesInGroup(int group);
		void	 enableDeflationVerbose(void);
		void	 disableDeflationVerbose(void);

public:// Finding singularities in mesh
		int  getNumSingularPoints();
		bool isSingular( int nodeID);
		int  getNumSingularPoints(int elemID);
		int  getChangeInSingularityAfterAddition(int elemID);
		int  getChangeInSingularityAfterDeletion(int elemID);

};
typedef struct  StructStrainComponent {
	static const int exx=0;
	static const int eyy=1;			
	static const int ezz=2;
	static const int exy=3;
	static const int exz=4;
	static const int eyz=5;
} StrainComponent;

typedef struct  StructStressComponent {
	static const int sxx=0;
	static const int syy=1;			
	static const int szz=2;
	static const int sxy=3;
	static const int sxz=4;
	static const int syz=5;
	static const int vonmises=6;
} StressComponent;

#define MAX(a,b) (((a) > (b))? (a):(b))
#define MIN(a,b) (((a) < (b))? (a):(b))

#define SIGN(a) (a<0?-1:1)


void GraphicsTemplate(float modelScale,void (*userFunction)(void),char *str);

#endif
