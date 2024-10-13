#ifndef TOPOPT_H
#define TOPOPT_H
#include "TransientElasticity.h"
#include <list>
#include <stdio.h>


#define	MAX_ISO_SAVES 250
#define MAX_BC_SET 25
#define MAX_CONSTRAINTS 2

#define ITERATION_ERROR -1
#define ITER_TO_VOL_ISOVALUE_FAILURE -2
#define ITER_TO_VOL_STRESS_FAILURE -3
#define ITER_TO_VOL_COMPLIANCE_FAILURE -4
#define ITER_TO_VOL_FEA_FAILURE -5
#define DIVERGENCE_FAILURE -6
#define CONSTRAINT_FAILURE -7
#define MODE_CHANGE_FAILURE -8
#define MESH_TOO_THIN_FAILURE -9
#define MESH_DISCONNECTED_FAILURE -10
#define USER_REQUEST_STOP -11
#define ITER_TO_VOL_SUCCESS 1


#define DOTPROD3D(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define DIST3D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+(p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])))


struct BackupData {// saved data after every successful topology optimization (for recovery from failure)
	double *sol;
	double *pseudoDensity;
	bool	*nodeExists;
	double *topSensElem;
	double *topSensNode;
	
	// following are needed for node based Ku
	int *neighborNodes;
	int *nodeSignature;

	double maxStress[MAX_BC_SET][MAX_PARTS];// for each load set, the maximum stress for each part
	double maxDisp[MAX_BC_SET];// for each load set, the maximum displacement across all parts
    double firstEigenMode;// only one across load sets
};

enum TOPOPT_QUANTITY
{
	VOLUME =0,
    COMPLIANCE, 
	VONMISES,
	EIGENVALUE,
	BUCKLING,
	DISPLACEMENT,
	OUTPUT_COMPLIANCE,
	GEOMETRIC_ADVANTAGE,
	MULTIMATERIAL_STRESS
};

enum TOPOPT_MODE
{
    TOPOPT_MODE_ELASTIC, 
    TOPOPT_MODE_THERMO_ELASTIC,
    TOPOPT_MODE_THERMAL// not implemented
};


// Constraints that can be imposed: displacement, stress, modal, buckling. 
// Constraints are considered through weighting of TS fields

class TopOpt: public TransientElasticity {
protected:// inherited classes can access these variables

	bool		ts_verbose;
    bool		ts_useAllLoads;
	int			ts_nSmoothingIterations;
	int			ts_min_topoptIterations;// in iterate to pareto curve
	int			ts_max_topoptIterations;
	int			ts_maxAttempts;// number of failures acceptable before quitting
	bool		ts_useAnalyticalTS;
	bool		ts_isTSMinNegative;// for stress problems
	double		ts_pNormIndex; // used in max von Mises approximation
	double		ts_stressSafetyFactor;//


	double		*ts_stressAdjoint;
	double		*ts_stressAdjointForce;		
	int			ts_saveEveryNIterations;
	bool		ts_ensureIsoSurfaceConnectivity;// if set to true, use the mesh pseudo densiuty to create isosurface (coarser), else use the TS field (smoother)
	double		ts_relativeJTolerance; //acceptabe tolerance for convergence check
    double		ts_relativeJConvergence;// acceptable ratio between two consecutuve iterations
    double      ts_relativeJIncrementForStepping;// For adaptive stepping, the increment in compliance target
    double		ts_volErrorFraction; // acceptable error in volume fraction
	double		ts_minAllowableVolumeFraction;
	double		*ts_topSensElem;// center of each elem
	double		*ts_topSensNode;// at each node
    double		*ts_topSensElemUnscaled;// useful for predictions
	
	double		ts_maxDisplacementSave[MAX_ISO_SAVES];
    double		ts_complianceSave[MAX_BC_SET][MAX_ISO_SAVES];
    double		ts_maxStressSave[MAX_BC_SET][MAX_ISO_SAVES][MAX_PARTS];
    double		ts_maxTemperatureSave[MAX_BC_SET][MAX_ISO_SAVES];
	double		ts_isoValueSave[MAX_ISO_SAVES];
	double		ts_volFracSave[MAX_ISO_SAVES];
	double		ts_timingSave[MAX_ISO_SAVES];
	double		ts_firstModeSave[MAX_ISO_SAVES];
	double		*(ts_topSensNodeSave[MAX_ISO_SAVES]);
	bool		*(ts_elemExists[MAX_ISO_SAVES]);

    // thermal effects
    bool        ts_includeThermalEffects;
    // various topopt constraints
    int			ts_drawDirection;// -1 is no draw direction, 1 is x, 2 is y and 3 is z
    int			ts_axiSymmetryIndex;//about z axis,  -1:none, 2 = 180, 3 = 120, ...
    int         ts_gridPattern[3]; // for imposing grid patterns along x, y and z (= -1, none)
    int			ts_symmetryIndex;/* 1 = x-symmetry
                                    2 = y-symmetry
                                    3 = x & y symmetry
                                    4 = z-symmetry
                                    5 = x & z symmetry
                                    6 = y & z symmetry
                                    7 = x, y & z symmetry*/
    int			ts_extrudeDirection;// -1: no constyraint, 1 is x, 2 is y and 3 is z
	int			ts_AMBuildDirection;
    bool		ts_useEigenConstraint;
    bool		ts_useTemperatureConstraint;
    double		ts_minFeatureSize;// -1: no constraint
    double		ts_maxFeatureSize;// -1: no constraint
    double		ts_minEigenValueHz;// -1: no constraint
    double		ts_maxTemperatureLimit;// -1: no constraint
    bool        ts_keepTopologyConnected; // true by default


	bool		ts_restartTopology; // at each step, restart with full topology or not
	bool		ts_scaleTSField;// should the field be scaled from 0 to 1
	BackupData	ts_backupData;
	double		ts_TSMax,ts_TSMin;// for all nodes
	char		ts_lastTopOptErrorMessage[1024];
	TOPOPT_QUANTITY	ts_topoptQuantity;
	TOPOPT_MODE		ts_topoptMode;

	//Constraints
	double		ts_maxDisplacementAllowed;

	// The TS fields are weightage average of the following
	double		ts_complianceWt;// is the TS Weight for compliance

	Timer		ts_timer;

	double		ts_compliance0;//initial compliance
	double		ts_displacement0;//initial displacement max
	double		ts_vonMisesStress0;//initial compliance
	double		ts_outputcompliance0;//initial output compliance, for compliant mechanisms
	double		ts_compliance;// compliance
	double		ts_outputcompliance;// output compliance, for compliant mechanisms
	double		ts_inputDisplacement;//  for compliant mechanisms
	double		ts_outputDisplacement;//  for compliant mechanisms
	double		ts_geometricAdvantage;//  for compliant mechanisms
	double		ts_mechanicalAdvantage;// for compliant mechanisms
	double		Win,Wout,Wmut;// for compliant mechanisms
	double		h_bucklingLoadValue;

	double		Fe[MAX_MATERIALS][6][24];
    double		TSComplianceNorm0,TSOutputComplianceNorm0,TSMutualComplianceNorm0,TSStressNorm0,TSBucklingNorm0,TSEigenNorm0; // for scaling purposes, compute initial value
	double		TSThermalNorm0;
	double		gradNxiCenter[8],gradNetaCenter[8],gradNmuCenter[8];

	void		computeTopSensAtElemsAnalytical(void);
	void		computeTopSensAtElemsApproximate(void);


	//member functions
    void	 imposeExtrudeOnElements(int dir);//only x, y or z allowed
    void	 imposeExtrude(int dir);//only x, y or z allowed
	void	 imposeDrawDirection(int dir);//only x, y or z allowed
	void	 imposeSupportStructureConstraintOnElems(int dir);//only x, y or z allowed
	void	 imposeSupportStructureConstraintOnNodes(void);
	void	 reduceGridNodeTSForSupportStructure(int gridNode,int nSupportNodes,int supportNodes[],double eta = 1);
	void	 increaseGridNodeTSForSupportStructure(int gridNode,int nSupportNodes,int supportNodes[],double eta = 1);
	void	 setVonMisesAdjointForce(void);
	void	 setOutputAdjointForce(void);
	void	 computeAdjointStrainAtElem(int elem, double strains[6]);
	void	 computeContributionOfTriangle(double x[3],double y[3],double z[3],double *vol,double *area,double center[3]);
	double	 computeAreaOfTriangle(double x[3],double y[3],double z[3]);
	void	 getTriangleNormal(double x[3],double y[3],double z[3],double normal[3]);
	void	 computeFMatrix(void);
	void	 initializets_backupData(void);
	void	 pushToSavedData(void);
	void	 retrieveFromSavedData(void);
	double	 findVolFractionBelowCutoff(double cutoff);
	double   computeBodyForceSensitivity(int elem);
	
    double   computeAllowableVolumeDecrement(double allowableRelativeChangeInCompliance);
	void	 computeTopSensInitial(void);
	void	 imposeSymmetry(int symmetryIndex);
    void	 imposeGridPattern(int gridPattern[3]);
	void	 imposeCyclicSymmetry(int axiSymmetryIndex);
    void     imposeMaxFeatureConstraint(void);

	void	 saveFEAandSolveEigen(int *nFEA);

	
    void	 fixNodalTSValues(int isoCounter = -1,bool invert = false);

    bool     getFinalModalSolutionAt(double pt[3],double modalUVW[3]);
	
	virtual void	 modifyTSField(void) {return;};//this will allow other applications to override the TS field
	virtual void	 modifyTSWeights(void) {return;};//this will allow other applications to override the TS field
	virtual bool	 userConstraintFunction(void) {return true;};//this will allow other applications to add additional constraint

public:
			 TopOpt();
	int		 InitializeTopOpt(int BCmode);
	
	void	 getMaxMinTopSens(double *ts_TSMax,double *ts_TSMin);
	void	 getFMatrix(double FMatrix[10][6][24]);
	void	 getElemTopOptSens(int elem,double *ts);
	void	 getElemNodesTopOptSens(int elem,double ts[8]);
	double	 getInitialDisplacement(void);
	double   getInitialVonMisesStress(void);
    double   getInitialMaxStressInPart(int part);
	double	 getTSOfElem(int elem);
	void	 getTSFieldinElems(double *tsField);
	void	 getTopSensAtElemNodes(int elem, double TS[8]);
	void	 getStressAdjointSolution(int elem, double adjointSol[24]);
	int		 getCurrentIsoCounter(void);
	double	 getCurrentTSCutoff(void);
	double	 getMaxStressInPartAtCounter(int isoCounter,int part);
	double	 getMaxDispAtCounter(int isoCounter);
	double	 getFirstEigenValueAtCounter(int isoCounter);
	int		 getClosestCounterAtVolFraction(double volFrac);
	int		 getClosestCounterAtVolFraction(double volFrac,double *actualVolFrac,double *isoCutoff,double *timeInSec);
	void	 getTopSensAtElemNodesAtCounter(int elem, int counter, double TS[8]);
	double	 getTopSensAtElemAtCounter(int elem, int counter);
	double	 getTopSensAtNodeAtCounter(int node, int counter);
	bool	 doesElemExistsAtTSCounter(int elem, int counter);
	double	 get_TSMax(void);
	double	 get_TSMin(void);
	void	 getAdjoint(double *adjoint);
	void	 getElemAdjoint(int elem,double adjoint[24]);
	void	 getAllVolFracs(double *vf);
    void	 getResultsAtAllVolFracs(double *vf, double *maxDisp, double maxStress[MAX_BC_SET][MAX_ISO_SAVES], double compliance[MAX_BC_SET][MAX_ISO_SAVES], double *timeTaken);
    void     getMaxTemperatureAtAllVolFracs(double *vf,double maxTemperature[MAX_BC_SET][MAX_ISO_SAVES]);
    void	 getEigenResultsAtAllVolFracs(double *vf, double *firstEigenMode);
	double	 getMinAllowableVolFraction(void);
	bool	 getKeepElem(int elem);
	

	void	 getLastTopoptErrorMessage(char msg[]);
	
    void	 setThermalEffects(bool);
	void	 setQuantityOfInterest(TOPOPT_QUANTITY quantity);
    void	 setEigenConstraint(double minEigenValueHz);
    void     setMaxTemperatureLimit( double maxTemperatureAllowed);
	void	 setMaxAttempts(int attempts);
	void	 setSymmetryIndex(int symmetryIndex);
	void	 setDrawDirection(int drawDirection);
    void     setMinFeatureSize(double minFeatureSize);
    void     setMaxFeatureSize(double maxFeatureSize);
    void	 setExtrudeDirection(int extrudeDirection);
	void	 setStressSafetyFactor(double safetyFactor);
    void     setKeepTopologyConnected(bool connected);
	void	 setTopoptMode(TOPOPT_MODE mode);
	void	 setTStoAnalytical(bool trueOrFalse);
	void	 setMaxDisplacement(double disp);
	void	 setNumSmoothingOperations(int nSmoothing);
	void	 setAdjoint(double *adjoint);
	void	 setPNorm(double pNorm);
	void	 setMinAllowableVolFraction(double volFrac);
	bool	 saveTopOptField(double volFrac,double isoValue,bool forceSave=false);
	void	 setTopOptFieldAtNodes(double *nodalValues);
	void	 resetIsoCounter(void);
	void	 setTSScaleToUnity(bool scaleToUnity);
	bool	 setAMBuildDirection(int buildDirection);
	void	 setIterationCountForSaving(int iter);
    void     setXGridPattern(int xGrid);
    void     setYGridPattern(int yGrid);
    void     setZGridPattern(int zGrid);
    void	 setAxiSymmetry(int axiSymmetry);
	bool	 isEigenConstraintActive(void);
    void     resetTopOptConstraints(void);

	double	 getPNormStress(void);
	double	 computeOutputCompliance(void);
	double	 computeMutualCompliance(void);
	double	 computeJ(void);
	void	 computeTopSensAtElems(void);
	void	 TSSmoothener();
	void	 connectivityTSSmoothener();
	void	 kernelTSSmoothener();
	void	 computeNodalTSField();
	int		 checkConstraints(void);
	bool	 isIsoSurfaceDisconnected(void);
	double	 getBucklingLoad(void);
	
	void	 enableTSVerbose(void);
	void	 disableTSVerbose(void);
	bool	 doesTSExist(void);

	int		 solvePrimaryAndAdjointProblems(int *nFEA);

	int		 computeIsovalue(double volPercentDesired, double *isoMid, int *nElemsToRemove);
	int		 IterateToVolume(double desiredVolumeFraction, double *JConverged,int *iter,double *isoValueConverged, bool(*callBackFunction)(bool saveIsoSurface));
	double	 TraceParetoCurve(double desiredVolFrac,double volDecrement, int *nFEA, 
							double *isoValueConverged, bool (*callBackFunction)(bool saveIsoSurface),
							double *volDecrementFinal,double allocatedTimeInSecs = 3600000);
	
	
			 ~TopOpt();

};



#endif
