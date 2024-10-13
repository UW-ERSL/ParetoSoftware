#ifndef TRANSIENT_ELASTICITY_GPU_H
#define TRANSIENT_ELASTICITY_GPU_H
#include "AFElasticityGPUDefines.h"
#define MAX_TIME_STEPS 10000
struct transientData {

	// GPU Variables; see TransientElasticity.h for definitions
	
	double dtr_N_gamma;
	double dtr_N_beta;
	double dtr_Damp_alpha;
	double dtr_Damp_beta;
	double dtr_h;	// time step
	double dtr_totalTime;
	double dtr_CurrentTime;
	int dtr_TimeIndex;
	double dtr_tolerance;
	double *dtr_timeTrack;
	int dtr_NumDOF;
	double dtr_a0,dtr_a1,dtr_a2,dtr_a3,dtr_a4,dtr_a5,dtr_a6,dtr_a7;
	double dtr_MDcoef,dtr_MDdotcoef,dtr_MDdotdotcoef,dtr_KDcoef,dtr_KDdotcoef,dtr_KDdotdotcoef;
	double dtr_M_coef;
	double dtr_K_coef;
	double *dtr_SolOld; 
	double *dtr_SolDotOld; 
	double *dtr_SolDotDotOld;
	double *dtr_SolCurrent; 
	double *dtr_SolDot;
	double *dtr_SolDotDot;
	double *dtr_MD; 
	double *dtr_MDdot;
	double *dtr_MDdotdot;
	double *dtr_KD; 
	double *dtr_KDdot;
	double *dtr_KDdotdot;
	double *dtr_h_KEeffTemplates;//  24*24;
	double *dtr_h_KEdummy;
	/// Transient force
	double *dtr_gravity;
	double *dtr_Weight; 
	double *dtr_staticForce;
	double *dtr_Feff;
	double *initial_dtr_f;
	double *dtr_f;
	double dtr_f_value;
	/// Tracking
	bool dtr_Track;
	int dtr_ClosestNode;
	int dtr_Track_DOF;
	double *dtr_Track_time;
	double *dtr_Track_force;
	double *dtr_Track_displacement;
	double *dtr_Track_velocity;
	double *dtr_Track_acceleration;
	double *dtr_Track_stress;
	double dtr_TrackPt[3];
	double dtr_TrackminDist;
	/// Stress
	double dtr_vonMises;
} ;

// GPU functions
int InitializeTransElasticityOnGPU(int h_nNodes,transientData *gpuTr,int tr_Track_DOF);
void initializeTransientForceVectorOnGPU(GPUData *h_gpuData,transientData *gpuTr);
void SetTransientFeffVectorOnGPU(GPUData *gpuData,transientData *gpuTr);
void setTransientParamsOnGPU(transientData *gpuTr,double newmarkGamma, double newmarkBeta,
											 double dampingM, double dampingK,double transientDeltaTime,
											 double transientTotalTime,double tolerance);
void NewmarkSolverOnGPU(void);
void AllocateSolOnGPU(int h_nTemplateElements,transientData *gpuTr);
double diffNormOnGPU(transientData *gpuTr);
void UpdateSolOnGPU(transientData *gpuTr);
void setSolOnGPU(GPUData *gpuData,transientData *gpuTr);
void getSolOnGPU(GPUData *gpuData,transientData *gpuTr);
void copySol_GPUtoCPU(transientData *gpuTr,double *tr_SolCurrent);
void findVelocityAccelerationOnGPU(GPUData *gpuData,transientData *gpuTr);
void findDampingCoefficientsOnGPU(transientData *gpuTr,double tr_Damp_alpha,double tr_Damp_beta);
void setMKCoefficientsOnGPU(transientData *gpuTr,double tr_a0,double tr_a1,double tr_a2,double tr_a3,double tr_a4,double tr_a5,double tr_a6,double tr_a7,double tr_MDcoef,double tr_MDdotcoef,double tr_MDdotdotcoef,double tr_KDcoef,double tr_KDdotcoef,double tr_KDdotdotcoef);
void setGravityOnGPU(transientData *gpuTr,double *gravity);
void findWeightOnGPU(GPUData *gpuData,transientData *gpuTr,bool h_nodeBasedAssembly);
void EffeciveFOnGPU(GPUData *gpuData,transientData *gpuTr,bool h_nodeBasedAssembly);

void setGPUTrackingOn(transientData *gpuTr,int tr_Track_DOF);
void setGPUTrackingOff(transientData *gpuTr);
void initializeTrackingOnGPU(transientData *gpuTr);
void updateTrackingOnGPU(transientData *gpuTr,double Max_stress);
void NormalizeTrackedOnGPU(transientData *gpuTr);
int getNumberOfTimeStepsTrackedOnGPU(transientData *gpuTr);
void getTimeStepsTrackedOnGPU(transientData *gpuTr,double *time);
void getDisplacementTrackedOnGPU(transientData *gpuTr,double *disp);
void getStressTrackedOnGPU(transientData *gpuTr,double *stress);
void updateTimeOnGPU(transientData *gpuTr,int tr_TimeIndex,double tr_CurrentTime);

void FreeGPU(transientData *gpuTr);
#endif