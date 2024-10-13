#ifndef THERMO_ELASTICITY_H
#define THERMO_ELASTICITY_H
#include "AFPlasticity.h"

#define BOLTZMAN (5.670373e-8)

class ThermoElasticity: virtual public AFPlasticity {

	private:
		
		bool	 performDeflatedCGThermal(double *f,double *u,int *nIter,double transientFactor = 1.0);// with preconditioner

        bool	 performPreconditionedDeflatedCGThermal(double *f,double *u,int *nIter,double transientFactor = 1.0);// with preconditioner

	protected:// inherited classes can access these variables
        double   h_Kexponent; //K = K0*(rho)^n; this is the n value
        bool	 h_thermo_elasticity_verbose;
		double	 h_thermoElasticReferenceTemperature;// when there is no stress or strain for thermo-elastic problems
		double	 *h_sol_thermal; //thermal solution at each node (temperature:T)
		double	 *h_sol_thermalElem; //thermal solution at each elem (temperature:T)

		double	 *h_KeNodalThermal;// 256*1*27, nodal stiffness, 256 possible combinations for presence/absence of 8 elements
						 // 27 neighboring nodes, i.e., 27 dof
		double	 *h_KETemplates_thermal;//  8*8*h_nTemplateElements;
		double	 *h_convection;//  used if a node is subject to convection
        double   h_convection_coeff;
		double	 *h_radiation;//  non-zero if a node is subject to radiation = Area*Sigma/#NodesOnArea
		double	 *h_radiationNonLinear;//  computed for each iteration if a node is subject to radiation
		double	  h_radiationTRef[HEXMESH_MAX_FACE_LABELS];
		double	  h_convectionTRef[HEXMESH_MAX_FACE_LABELS];// 
		bool	  h_isThereRadiation; // set to true if any of the nodes is subject to radiation

		double	 *h_f_thermal; //heat flow (Q)
		double	 *h_f_thermalWithRadiation; // add the nonlinear radiation effect

		double	 *h_dirichlet_thermal;// Dirichlet value (typically zero)
		bool	 *h_free_thermal; //is nodal temperature fixed or free; false:fixed,true:free
		double    h_temperatureMin,h_temperatureMax;//maximum temperature 
		int		  h_nodeWithMaxTemp;
		double	  h_relAccuracyThermal;

		// Deflation 
		MatrixXd		KDeflatedThermal;
		SimplicialLLT<SparseMatrix<double>> sparseSolverThermal;

		void	 copy_thermal(double *from, double *to);
		double	 Norm_thermal(double *x);
		double	 dotprod_thermal(double *x, double *y);
		double	 normalize_thermal(double *x, double *y);
		void	 zeaxpy_thermal(double alpha,double beta,double *z,double *x, double *y);
		void	 ScaleByVector_thermal(double *z,double *x, double *y);
		void	 axpy_thermal(double alpha, double *x, double *y);
		void	 scale_thermal(double alpha, double *x);
		double	 errorNorm_thermal(double *x,double *y);
		void	 enforceDirichlet_thermal(double *x);
		void	 enforceDirichletZero_thermal(double *x);
		void	 ComputeKe_thermal(double dx,double dy, double dz, double alpha, double Ke_thermal[8][8]);
		void	 initializeRandomVector_thermal(double *x);
		void	 KuCPU_thermal(double *x,double *y,double transientFactor = 1.0);// 
		void	 KuCPUElemBased_thermal(double *x,double *y,double transientFactor = 1.0) ;
		void	 KuCPUNodeBased_thermal(double *x,double *y,double transientFactor = 1.0) ;
		void     releaseCPUMemory_thermal(void);
		bool	 performCGThermal(double *f,double *u,int *niter,double transientFactor = 1.0);//attempt with preconditioner, if failure, switch to CG
		bool	 performSimpleCGThermal(double *f,double *u,int *niter,double transientFactor = 1.0);// no preconditioner
		
		void	 computeTemperatureMax(void);
		double	 computeCompliance_thermal(void);
		void	 initializeKEThermal(void);
		void     setKeTemplatesThermal(double *K);
        void	 computeRadiationNonLinear(bool JacobianTerm = false);//,bool Transient =false,double TransientFactor = 1.0);//
		void	 computeKeNodalThermal(void);
        void	 computeThermalJacobiPreconditioner(double *preconditionerJacobi);
		void	 computeElementTemperature(void);
		void	 computeResidual(double *currentSol,double *residual);
		bool	 solveThermalProblemNewtonRaphson(int maxIter);
		bool	 solveThermalProblemPicard(int maxIter);
		void	 getWorkSpaceVectorThermal(double* inVector, VectorXd &outVector);
		void     getSolutionSpaceVectorThermal(VectorXd &inVector, double* outVector);
		bool	 KreductionThermal(double transientFactor);
		bool	 sparseWKWThermal(double transientFactor);

public:
                                 ThermoElasticity();
			void	 enableThermoElasticityVerbose(void);
			void	 disableThermoElasticityVerbose(void);
            void     set_thermalStaticSol(double *sol);
			void	 setThermoElasticReferenceTemperature(double T);
			void	 setBCOnNodes_thermal(int nodeLabel,int myBCtype_thermal,double myBC_area,double myBCValue_thermal,double myBCRef_thermal);
            int      allocateMemory_thermal(void);
			void	 initializeSolution_thermal(void);	
			void	 resetBC_thermal(void);
            bool	 solveThermalProblem(bool resetTemperature = true);
            double   getThermalForce(int node);
            void	 clearThermalConvectionLoad();
            void     applyDirichletOnBaseNodes(double T0);
            void     applyThermalFluxOnAllExposedNodes(double q);
            void	 applyConvectionOnAllExposedNodes();
            void	 applyScaledConvectionOnZ0Surface(double scale);
			double   computeElemInnerProductThermal(int elem,double u1[8],double u2[8]);
            void	 setBCOnSingleNode_thermal(int node,int BCType,double BCValue);
			void	 getTemperatureRange(double *tMin,double *tMax, int *whichNodeMax = NULL);
			void	 setTemperatureRange(double tMin,double tMax);
			bool	 getTemperatureAt(double pt[3],double *temperature);
            bool     getTemperatureGradientInElem(int elem,double gradient[3]);
            bool	 getTemperatureInAllElems(double *temperature);
            bool     getTemperatureInAllNodes(double *temperature);
			double	 getTemperatureInElem(int elem);
			double   getTemperatureMax(void);
			void	 getElemNodesTemperature(int elem,double temperature[8]);
			void     addThermalForce(void);
			void     addThermalStrainDueToSolidification(double TMelting, int nElems,int *elems);
            void     getHPrime(double HPrime[24][8]);
			void     get_fThermal(double *f);
            void     getHdeltaT(int elem,double HdeltaT[24]);
			void     set_fThermal(double *f);
			void     get_convectionThermal(double *f);
			void     set_convectionThermal(double *f);
            void     setConvectionCoefficient(double h);
			bool	 isTemperatureFixedOnNode(int node);
			bool	 isThereHeatFluxOnNode(int node);
            bool	 isNodeFree_thermal(int node);
			void	 reactionFluxThermal(double *r,double transientFactor);
			bool	 pushThermalSolutionToTessellation(void);
			bool	 getTessellationThermalSolutionOfTriangle(int tri,double temperature[3]);
					 ~ThermoElasticity(void);

};

#endif
