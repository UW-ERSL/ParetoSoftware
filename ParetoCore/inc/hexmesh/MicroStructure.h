#ifndef MICROSTRUCTURE_H
#define MICROSTRUCTURE_H
#include "TopOptForAM.h"


#define MICROSTRUCTURE_VERBOSE 0

enum MICRO_OBJECTIVE
{
	BULK_MODULUS,
	SHEAR_MODULUS,
	X_RIGIDITY,
	BULK_MODULUS_2D,
	NEG_POISSON
};

class MicroStructure: public TopOptForAM {
protected:// inherited classes can access these variables
	
	MICRO_OBJECTIVE mu_objective;		
	double			xMin,yMin,zMin;
	double			xMax,yMax,zMax;
	bool			mu_exploitSymmetry;

	double			H[6][6];// for topological sensitivity
	double			T1[6], T2[6],T3[6],T4[6],T5[6],T6[6];

	double			mu_JExactSave[MAX_ISO_SAVES];
	double			mu_JComputedSave[MAX_ISO_SAVES];

	double			*(mu_elemstresses[6][6]);// the 6 strains applied x 6 stress components in the middle of each element
	
	
	int		 IterateMicroStructureToVolume(double desiredVolumeFraction, double *JConverged,
							  double *isoValueConverged);

	void	 doubleContract(double A[6][6],double B[6][6],double C[6][6]);
	void	 doubleContract(double A[6][6],double B[6],double C[6]);
	void	 doubleContract(double A[6],double B[6],double *C);
	double	 computeTopologicalSensitivity(void);
	int		 solveConstantStrainProblem(int problem,double CMatrix[6][6]);
	void	 saveMicrostructureResult(double volFrac,double isoValue,double J,bool forceSave);

public:
			 MicroStructure();
	void	 initializeMicroStructure(MICRO_OBJECTIVE objective,double xyzMin[3],double xyzMax[3]);
	bool	 isNodeOnCubeBoundary(int node);
	int		 findInverseOfMaterialMatrix(double C[6][6],double CInv[6][6]);
	void	 applyStrainMatrix(double strain[6]);
	int		 computeCMatrix(double CMatrix[6][6]);
	void	 printCMatrix(double CMatrix[6][6]);
	void	 printHashinSthrikmanBounds(void);
	double	 getHashinSthrikmanBound(double volFrac);
	void	 shiftByHalfCell(void);
	void	 stackCell(void);
	void	 resetCell(void);
	void	 getJatAllVolumeFractions(double *vf, double *JValuesRelative,double *JExactRelative);
	void	 extractMaterialProperties(double CMatrix[6][6],double *E,double *nu, double *K,double *G);
	double	 optimizeMicrostructure(double desiredVolFrac,double volDecrementMax, int maxFEA,int *nFEA,
													bool (*callbackFunction)(double isoValueConverged),
													double *isoValueConverged,double *volDecrementStart);
	double	 findDeterminantOfCMatrix(double C[6][6]);
	
			 ~MicroStructure();

};

#endif