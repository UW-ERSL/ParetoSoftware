#ifndef TOPOPTSINGLELOADMULTICONSTRAINTS_H
#define TOPOPTSINGLELOADMULTICONSTRAINTS_H
#include "ContourExtraction.h"
#include <list>
#include <stdio.h>

#define MAX_CONSTRAINTS 10

struct BackupData_ALM {// saved data after every successful topology optimization (for recovery from failure)
	//augmented Lagrangian algorithm parameters
	int	   StepNumber_ALM;//step number
	double Sigma[MAX_CONSTRAINTS];//for every constraint, its Lagrangian multiplier
	double Penalty[MAX_CONSTRAINTS];//for every constraint, its penalty number
};


class TopOptSingleLoadMultiConstraints: public ContourExtraction {

protected:
	//properties
	// Problem parameters
	int		ts_nConstraints; //number of constraints
	int		ts_constraintType[MAX_CONSTRAINTS]; //for every constraint, its constraints
	bool	ts_isUpperBound[MAX_CONSTRAINTS]; //for every constraint, upper bound or lower bound
	bool	ts_isHardConstraint[MAX_CONSTRAINTS]; //for every constraint, hard or soft constraint //Deng
	double	ts_UpperOrLowerLimit[MAX_CONSTRAINTS]; //for every constraint, its limit
	double	ts_xyzLocation[MAX_CONSTRAINTS][3]; //for the constraint wrt a point, (x,y,z) corrdinates of the point
	int		ts_objective; //optimazation objective

	// Algorithm parameters
	int	   ts_StepNumber_ALM;//step number
	double ts_complianceLastStep;//compliance value save in last step 
	double ts_Sigma[MAX_CONSTRAINTS];//for every constraint, its Lagrangian multiplier
	double ts_Penalty[MAX_CONSTRAINTS];//for every constraint, its penalty number
	double ts_gFunction[MAX_CONSTRAINTS];//for every constraint, its constraint value 
	double ts_gFunctionInitial[MAX_CONSTRAINTS];//for every constraint, its initialconstraint value 
	double ts_Saved_gFunction[MAX_CONSTRAINTS];//for every constraint, its constraint value in last successful iteration
	double *ts_allConstraints_TS;//storing topological sensitivity fields for all constraints
	double *ts_augmentedTS;//augmented topological sensitivity field
	double *ts_objective_TS;//topological sensitivity field of objective
	double ts_J0;//initial compliance 
	double ts_Stress0;//initial stress
	double ts_BucklingLoad0;//initial critical buckling load
	double ts_EigenVaule0;//initial lowest eigen-value
	double ts_Disp0;//initial displacement at specific point
	double ts_OJ0;//output compliance
	double ts_MJ0;//mutual compliance 
	bool   ts_verbose_SLMC;//print out debug information
	bool   ts_anyConstraint_violated; 
	bool   ts_initializeMultiplierFun;


	//functions
	void    InitializeSLMCParameters(void);
	double  compute_gfunc_perConstraint(int);
	double  compute_initialQuantity_perConstraint(int); 
	double  compute_Quantity_perConstraint(int); 
	void	compute_InitialMultiplier(void);
	void	augmentedTSFields(void);
	void	onSuccess(void);//update parameters when successful
	void	onFailure(void);//update parameters when failure
	void	updatePenalty(void);
	void	updateSigma(void);
	bool	solve_FEA_TS(int *nFEA);
	int		IterateToVolume_SLMC(double desiredVolumeFraction, double *JConverged,
							 int *nFEA, double *isoValueConverged);
	bool    constraintVerify(int *constrainingBCSet);


protected: //deng
	void	setTopoptQuantity(int quantity);

public:
	TopOptSingleLoadMultiConstraints();
	
	double  TraceParetoCurve_SLMC(double desiredVolFrac,double volDecrementMax, int maxFEA,int *nFEA, 
							  double *isoValueConverged, bool (*callBackFunction)(double isoValueConverged),
							  int drawDirection,int symmetryIndex,double *volDecrementStart,double allocatedTimeInSecs,
							  int *constraintType, int numConstraints, double (*currentConstraintsValue)[6], int topOptObjective); //deng

	~TopOptSingleLoadMultiConstraints();
};

#endif