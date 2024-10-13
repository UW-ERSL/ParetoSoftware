#ifndef TOPOPTMULTILOADSMULTICONSTRAINTS_H
#define TOPOPTMULTILOADSMULTICONSTRAINTS_H
#include "TopOpt.h"
#include <list>
#include <stdio.h>

class TopOptMultiLoadsMultiConstraints: public TopOpt {

private:
	//properties
	double *ts_augmentedTS;//augmented topological sensitivity field
	double *ts_objective_TS;//topological sensitivity field of objective
	
	// Problem parameters
	int		ts_nConstraints[MAX_BC_SET]; //number of constraints for each load
	int		ts_nTotalConstraints;
	int		ts_nBCSet;
	int		ts_constraintType[MAX_BC_SET][MAX_CONSTRAINTS]; //for every constraint, its constraint type
	bool	ts_isUpperBound[MAX_BC_SET][MAX_CONSTRAINTS]; //for every constraint, upper bound or lower bound
	bool	ts_isHardConstraint[MAX_BC_SET][MAX_CONSTRAINTS]; //for every constraint, hard or soft constraint 
	double	ts_UpperOrLowerLimit[MAX_BC_SET][MAX_CONSTRAINTS]; //for every constraint, its limit
	double	ts_xyzLocation[MAX_BC_SET][MAX_CONSTRAINTS][3]; //for some of the constraints, a location (x,y,z)
	int		ts_objective; //optimization objective
	int		ts_BCType[MAX_BC_SET][1000][3];
    double	ts_BCValue[MAX_BC_SET][1000][6];
	int		ts_currentBCSet;
	int		ts_objectBCSet;
	int		ts_numTriGroups;
	bool	ts_activeBCSets[MAX_BC_SET];
	bool	ts_isAllConstraintsHard;

	// Algorithm parameters
	int	   ts_StepNumber_ALM;//step number
	double ts_complianceLastStep[MAX_BC_SET];//compliance value save in last step 
	double ts_Sigma[MAX_BC_SET][MAX_CONSTRAINTS];//for every constraint, its Lagrangian multiplier
	double ts_Penalty[MAX_BC_SET][MAX_CONSTRAINTS];//for every constraint, its penalty number
	double ts_gFunction[MAX_BC_SET][MAX_CONSTRAINTS];//for every constraint, its constraint value 
	double ts_Saved_gFunction[MAX_BC_SET][MAX_CONSTRAINTS];//for every constraint, its constraint value in last successful iteration
	double **ts_allConstraintsML_TS;//storing topological sensitivity fields for all constraints
	double ts_J0[MAX_BC_SET];//initial compliance 
	double ts_MLMaxStress0[MAX_BC_SET][MAX_PARTS];// for each load, for each part, the initial maximum von Mises stress
	double ts_BucklingLoad0[MAX_BC_SET];//initial critical buckling load
	double ts_EigenVaule0[MAX_BC_SET];//initial lowest eigen-value
	double ts_Disp0[MAX_BC_SET];//initial displacement at specific point
	double ts_OJ0[MAX_BC_SET];//output compliance
	double ts_MJ0[MAX_BC_SET];//mutual compliance 
	bool   ts_initializeMultiplierFun;
	double ts_J[MAX_BC_SET];
	double ts_MLMaxStress[MAX_ISO_SAVES][MAX_BC_SET][MAX_PARTS];// for each vol fraction, for each load, for each part, the maximum von Mises stress
	double ts_BucklingLoad[MAX_BC_SET];
	double ts_EigenVaule[MAX_BC_SET];
	double ts_Disp[MAX_ISO_SAVES][MAX_BC_SET];
	double ts_OJ[MAX_BC_SET];
	double ts_MJ[MAX_BC_SET];



	//functions
	double  compute_gfunc_perConstraint_perBCSet(int,int);
	double  compute_initialQuantity_perConstraint_perBCSet(int,int); 
	double  compute_Quantity_perConstraint_perBCSet(int,int); 
	void	compute_InitialMultiplier_loadMagnitude(void);
	void	augmentedTSFields(void);
	void	onSuccess(void);//update parameters when successful
	void	onFailure(void);//update parameters when failure
	void	updatePenalty(void);
	void	updateSigma(void);
	bool	solve_FEA_TS_ML(int *nFEA);
	bool    constraintVerify(int *constrainingBCSet);
	void	InitializeMLMCParameters(void);
	void	retrieveFromSavedData_ML(void);
	void	pushToSavedData_ML(void);
	void	computeJ(double J[MAX_BC_SET]);
	void	changeTopOptBC(void);
	bool	checkDispAndStressConstraintsForAllBCSets(int *constraintBCSet);
    bool    saveTopOptField_ML(double volFrac,double isoValue,bool forceSave = false);
	int		IterateToVolume_MLMC(double desiredVolumeFraction, double *JConverged,
							int *nFEA, double *isoValueConverged, bool(*callBackFunction)(bool saveIsoSurface));

public:
	TopOptMultiLoadsMultiConstraints();

	double  TraceParetoCurve_MLMC(double desiredVolFrac,double volDecrementMax, int *nFEA, 
							double *isoValueConverged, bool (*callBackFunction)(bool saveIsoSurface),double *volDecrementStart,
							int constraintType[MAX_BC_SET][MAX_CONSTRAINTS], int *numConstraints, double currentConstraintsValue[MAX_BC_SET][MAX_CONSTRAINTS][6], 
                            int topOptObjective, int numBCSet,int BCType[MAX_BC_SET][1000][3], double BCValue[MAX_BC_SET][1000][6], int numTriGroups,
							bool activeBCSets[100],double allocatedTimeInSecs = 3600000);

	void	get_allCompliances_ML(double J[MAX_BC_SET]);
	void	get_allMaxStressesAtCounterForAllLoadsAllPart_ML(int isoCounter,double Stress[MAX_BC_SET][MAX_PARTS]);
	void	get_allMaxDisplacementsAtCounter_ML(int isoCounter,double Disp[MAX_BC_SET]);
	void	getSafetyFactorsAtVolumeFraction(double volFrac,double *displacementSF,int *criticalLoadForDisplaement,double *stressSF,int *criticalLoadForStress, int *criticalPartForStress);

	~TopOptMultiLoadsMultiConstraints();
};

#endif
