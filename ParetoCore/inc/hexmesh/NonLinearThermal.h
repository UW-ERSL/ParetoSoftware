
#ifndef NONLINEARTHERMAL_H_
#define NONLINEARTHERMAL_H_

#include <IntegerSet.h>
#include "TransientThermal.h" // import Pareto functions

using namespace std;

class NonLinearThermal :public virtual TransientThermal{

private:
	// bool variables to indicate the problem to be solved
	bool nlt_isTransient;

	// material nonlinearity variables
	// k = k0 + k1*T + k2*T^2 ; T in range [TLowerLimit, TUpperLimit]
    double nlt_k[3]; //  nonlinear thermal conductivity coefficients interpolationg a quadratic function of temperature
	double nlt_c[3]; // specific heat coeffs

	double	 *nlt_CETemplates_thermal;//  8*8*h_nTemplateElements;
	
	double	 *h_KETemplates_NLThermal_0;//  8*8*h_nTemplateElements;
	double	 *h_KETemplates_NLThermal_1;//  8*8*h_nTemplateElements;
	double	 *h_KETemplates_NLThermal_2;//  8*8*h_nTemplateElements;

	double	 *tt_sol_thermalElemPrev;//

	// transient simulation
	double nlt_timeStepSize;
	double nlt_initialTemperature;
	double nlt_totalTime;

	//phase change
	int nlt_isPhaseChange;
	double nlt_latentHeat;
	double nlt_liquidusTemp ; 
	double  nlt_solidusTemp;
	double *nlt_phaseStateOfElem;

	//thermoelasticity
	bool	nlt_solveThermoElastic;
	double nlt_thermalForceTemplate[24];
	double	nlt_thermalExpansionCoeffs[3];

	//internal vol heat source
	bool  nlt_isVolumetricHeatSourcePresent ;
	double nlt_volumetricHeatSource ;

	// evolving surface heat source
	int* nlt_numNeighbouringElemsOfNodeOnEvolvingSurface;

protected:

	// radiation on exposed nodes
	void applyRadiationOnAllExposedNodes(double emissivity) ;
	//deflation
    bool	sparseWKW_NLThermal(void);
    bool	Kreduction_NLThermal(void);
	void	initializeKEnonLinearThermal(void);


    double	getNonLinearConductivityAtTemp(double T,double T_old);
    double	getNonLinearSpecificHeatAtTemp(double T,double T_prev);


	bool	solveNLThermalProblemPicard(int maxIter,int timeStep = 0);

    void	KuCPU_NLThermal(double *x,double *y);
    int 	performCG_NLThermal(double *f,double *u) ; // similar to perform CG_thermal.. couple of differences marked in the code
    int	    performSimpleCG_NLThermal(double *f,double *u);
    int		performDeflatedCG_NLThermal(double *f,double *u);
    void	ComputeNLThermalEffectiveForce(int timeStep);
	void	KuCPUForceElemBased_NLThermal(double *x,double *y);

	double	getPreviousTemperatureInElem(int elem);
	void	savePreviousTransientTemperature(void);

	// internal heat source
	void	addVolumetricHeatSource();
	//phase change
	
	int		isElementChangingPhase(double temperature);

	double	getThermalPhaseOfElem(int elem);
	void	updateThermalPhaseOfElem(int elem);
    void    updateThermalPhaseOfElements();
	//thermo-elasticity
	void	computeNLThermalForceTemplate();
	double	getThermalExpansionCoeffAtTemperature(double T);
	void	addNonLinearThermalForce();

	// evolving  surface boundaries and fluxes
	bool	findNumNeighbouringElemsOfNodeOnEvolvingBoundary(int node);
	void	applyConvectionOnExposedSurface(double h, double TRef) ;
	double	getExposedNodeAreaFraction(int node);

public:
			NonLinearThermal();
	bool	InitializeNonLinearPhaseChangeData(int isPhaseChange,double latentHeat, double Tms, double Tml);
    void	setNonLinearThermalCoefficients(float k[3], float c[3]);
	void	setThermalMaterialNonLinearityAnisotropicCoeff(int isNonLinearAnisotropic,double k[3][3],double c[3],double Tl , double Tu );
	void	setTransientNonLinearThermalData(bool isNonLinearTransient,float initialTemperature, float timeStepSize,float totalTime);
	bool	solveNLStaticThermalProblem(bool solveElastic,bool (*callBackFunction)(double nrmError));
    bool	solveNLTransientThermalProblem(double initialTemperature,double timeStepSize,double simulationTime,bool solveElastic,bool (*callBackFunction)(double nrmError));
	bool	setSolveThermoElasticity(bool solveThermoElasticity);
	void	setNonLinearThermalExpansionCoeffs(float alpha[3]);
	
	//internal Vol Heat source
	bool setInternalHeatGenerationVolumetric(double Q); 
};
#endif
