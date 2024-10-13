#ifndef TRANSIENT_ELASTICITY_H
#define TRANSIENT_ELASTICITY_H
#include "ParetoSensor.h"
#include "TransientElasticityGPU.h"
#define TRANSIENT_ELASTICITY_VERBOSE 1
#define MAX_TIME_STEPS 10000

#define PI 3.14159265359
#define DROP_DIRICHLET 1
#define DROP_FORCE 2

class TransientElasticity:  public ParetoSensor {
protected:// inherited classes can access these variables

	transientData gpuTr;

    bool tr_verbose;
	// Transient Elasticity
	bool tr_nodeBasedAssembly;
	double tr_a0,tr_a1,tr_a2,tr_a3,tr_a4,tr_a5,tr_a6,tr_a7;
	double tr_MDcoef,tr_MDdotcoef,tr_MDdotdotcoef,tr_KDcoef,tr_KDdotcoef,tr_KDdotdotcoef;
	double tr_N_gamma;
	double tr_N_beta;
	double tr_Damp_alpha;
	double tr_Damp_beta;
	double tr_h;	// time step
	double tr_totalTime;	// time 
	double tr_CurrentTime;
	int tr_TimeIndex;
	double tr_tolerance;
	double tr_diff;
	int tr_NumDOF;
	double *tr_SolOld; 
	double *tr_SolDotOld; 
	double *tr_SolDotDotOld;
	double *tr_SolCurrent; 
	double *tr_SolDot;
	double *tr_SolDotDot;
	double *tr_MD; 
	double *tr_MDdot;
	double *tr_MDdotdot;
	double *tr_KD; 
	double *tr_KDdot;
	double *tr_KDdotdot;
	double *tr_staticForce;
	double *tr_Feff;
	double tr_M_coef;
	double tr_K_coef;
	int	tr_nTemplateElements;
	double *tr_h_KEeffTemplates;//  24*24;
	double *tr_h_MEeffTemplates;
	double *tr_h_KEdummy;
	// Weight
	bool TR_GRAVITY;
	double tr_gravity[3];
	double *tr_Weight;
	// Stress
	double tr_vonMises;
	// Drop test
	int DR_METHOD;
	bool DR_DROPTEST;
	double dr_Height;
	double dr_zg;	// z of ground
	double dr_fallingZVelocity;
	int *dr_BoundaryNodes;
	int dr_nBoundaryNodes;
	double *dr_Fcontact;
	int sampleNode;
	// Force-Based
	double *dr_Indentation;
	int *dr_ContactNodes;
	int dr_nContactNodes;
	double dr_ContactRadius; // Hetrz contact radius
	double dr_VoxelKprime; // Fcontact = kprime*sqrt(Rcontact)*indentation^1.5
	double dr_E;
	double dr_Nu;
	// Dirichlet-Based
	int *dr_tempFixed;
	int dr_nTempFixed;
	int dr_nNewFixed,dr_nOldFixed,dr_nStaticFixed;
	// Tracking
	bool tr_Track;
	int tr_ClosestNode;
	int tr_Track_DOF;
	double *tr_Track_time;
	double *tr_Track_force;
	double *tr_Track_displacement;
	double *tr_Track_velocity;
	double *tr_Track_acceleration;
	double *tr_Track_stress;
	double tr_TrackPt[3];
	double tr_TrackminDist;


    // Frequency response
    int tr_nFrequencySteps;
    double *tr_omegaRadiansIn;
    double *tr_deltaMaxOut;
    double *tr_phaseAngleRadiansOut;


public:
	 TransientElasticity();

	int  InitializeTransientOnCPU(void);
	void  setTransientParams(double newmarkGamma, double newmarkBeta,
					double dampingM, double dampingK,double transientDeltaTime,
					double transientTotalTime,double tolerance);
    void  solveElasticTransientCPUGPU(bool(*callBackFunction)(double fractionComplete));
	// Transient Elasticity On CPU 
    void  NewmarkSolver(bool(*callBackFunction)(double fractionComplete));
	void  allocateSol(void);
	int  initializeTransientElasticity(void);
	void  initializeTransientForceVector();
    void  setDampingParams(double dampingM, double dampingK);
	void  setTransientParamsOnCPU(double newmarkGamma, double newmarkBeta,
					double dampingM, double dampingK,double transientDeltaTime,double transientTotalTime, 
					double tolerance);
	void  setGravity(void);
	void  findKEeffTemplate(void);
	void  findDampingCoefficients(double *d_M, double *d_K);

	void  computeTransientCoefficients(void);
	double  normDisplacementChange(void);
	void  updateSol(void);
	double  computeKineticEnergy(void);
	void  findVelocityAcceleration(void);
	void  computeEffectiveF(void);
	void  findWeight(void);

    // Transient Elasticity On GPU
	void  NewmarkSolverOnGPU(transientData *gpuTr);
	void  allocateCPUforGPU();
	// Drop Test Simulation On CPU
	void  DropTestSimulation();
	void  FreeFallSimulation(double ZPlaneLocation);
	double  findTheoreticalIndentation(double KE);
	void  initializeDropTest (void);
	void  initializeDropTestForceBased();
	void  initializeDropTestDirichletBased();
	void  initializeDropTestForceVector();
	void  findZofFloor(void);
	void  findBoundaryNodes(void);
	// Force-Based
	void  findContactForces(void);
	double  findVoxelContactRadius(void);
	void  findContactNodes(void);
	int  findCenterOfContact(double *Indentation,int nNodes);
	double  findDistanceToCenterNode(int node,int center);
	void  Sphere_PlaneContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,int *Nodes,int nNodes);
	void  Sphere_SphereContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,int *Nodes,int nNodes);
	void  Cylinder_CylinderContact(double *Fcontact,double *Indentation,double E,double Nu,double radius,double length,int *Nodes,int nNodes);
	// Dirichlet-Based
	void  findDirichletReactionForces(void);
	void  findTemporaryDirichletNodes(void);
	void  applyTemporaryDirichletToDisplacement(void);
	void  applyTemporaryDirichletToCurrentVelocityAndAcceleration(void);
	void  applyTemporaryDirichletToOldVelocityAndAcceleration(void);
	// LDC
	double  getTransientCurrentTime();
	void  setStaticForceVector();
	// Tracking
	void  setTrackingOn(double *point,int dir);
	void  setTrackingOff();
	void  initializeTracking();
	void  updateTracking();
	void  normalizeTracked();
	void  findMaxTrackedValue(double *maxval,double *TrackedVector);
	int   getNumberOfTimeStepsTracked();
	void  getTimeStepsTracked(int N,double *time);
	void  getDisplacementTracked(int N,double *disp);
	void  getStressTracked(int N,double *stress);

    // Frequency Reponse;
    int getNumFrequencyResponsePoints(void);
    int     getFrequencyData(double *omegaHz, double *deltaMax, double *phaseAngleDegrees);
    bool    frequencyResponseForLowFrequencies(int nFrequencySteps, double *omegaRadiansIn,
                                               double *deltaMaxOut, double *phaseAngleRadiansOut,
                                               bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseForHighFrequencies(int nFrequencySteps, double *omegaRadiansIn,
                                                double *deltaMaxOut, double *phaseAngleRadiansOut,
                                                bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseModalSuperPosition(int nFrequencySteps, double *omegaRadiansIn,
                                    double *deltaMaxOut, double *phaseAngleRadiansOut,
                                    bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseDirect(int nFrequencySteps, double *omegaRadiansIn,
                                    double *deltaMaxOut, double *phaseAngleRadiansOut,
                                    bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseDirectOld(int nFrequencySteps, double *omegaRadiansIn,
                                    double *deltaMaxOut, double *phaseAngleRadiansOut,
                                    bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseSimpleCG(int nFrequencySteps, double *omegaRadiansIn,
                                      double *deltaMaxOut, double *phaseAngleRadiansOut,
                                      bool (*callBackFunction)(double fractionCompleted));
    bool    frequencyResponseBiCGStab(int nFrequencySteps, double *omegaRadiansIn,
                                      double *deltaMaxOut, double *phaseAngleRadiansOut,
                                      bool (*callBackFunction)(double fractionCompleted));

    void    saveFrequencyResponseData(int nFrequencySteps, double *omegaRadians,
                                                        double *deltaMax, double *phaseAngleRadians);

    void KDynamicX(double *x, double w,double alpha, double beta,double *y);
    void    CdTimesX(double *x, double *y, double w);
    void    KdTimesX(double *x, double *y, double w);
    void    enforceDirichlet0_frequencyResponse(double *x);


	 ~TransientElasticity(void);
};

#endif
