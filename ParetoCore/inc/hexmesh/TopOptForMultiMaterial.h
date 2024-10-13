#ifndef TOPOPT_MULTIMATERIAL_H
#define TOPOPT_MULTIMATERIAL_H
#include "TopOptMultiLoadsMultiConstraints.h"
#include <vector>
#include "IntegerSet.h"
#include "CommonDataStructuresMultiMaterial.h"

struct MultiMaterialHistoryData{
	vector<double> costFraction;
	vector<double> objective;
	vector<double> compliance;
	vector<double> stress;
};

struct TopOptForMultiMaterialLiveVariables {
	int multimaterialObjective;
	int reset;
	int displayMode;
	int numBins;
	int nSmoothIter;
	int plot;
	int verbose;
	int stressModel;
	float maxStress;
	float pNormIndex;
	int NumberOfSTL;
	int binary;
	int savePerStep;
	char materialFileName[512];
	int XSectionPlane;
	int XSectionValue;
	int accuracy;
	int materialGrading;
	int materialSymmetry;
};

struct RankedData{
	double value;
	int element;
	int material;
};

struct MultiMaterialBackupData{
	vector<double> solution;
	vector<int> material;
	vector<double> psuedoDensity;
	vector<bool> nodeExists;
};

enum MULTIMATERIAL_STL_FILES
{
	STL_MATERIALS, 
	STL_COMPONENTS,
	STL_ASSEMBLY
};

class TopOptForMultiMaterial : public virtual TopOptMultiLoadsMultiConstraints
{
protected:

	int tomm_ParetoCounter;
	int tomm_savePerStep;
	bool tomm_verbose;
	int tomm_nFEA;

	double tomm_minAllowableCostFraction;
	TOPOPT_OBJECTIVES tomm_topoptQuantity;

	int tomm_currentIsoCounter;
	int tomm_nSensSmootheningIter;
	int tomm_numSupSteps;
	int tomm_gradingLevel;
	bool tomm_foundNodeNeighbors;
//	// common


	vector<double> tomm_Costs,tomm_CostsSorted;
	vector<int> tomm_CostsSortedID;

	vector<double> tomm_currentI,tomm_alternateI;
	vector<int> tomm_currentMaterial;
	vector<RankedData> tomm_Ranked;
    vector<bool> tomm_isChanged;

	double tomm_initalCost,tomm_targetCost,tomm_deltaCost;
    double tomm_initialCompliance,tomm_initialDeformatoin;//,tomm_initialStress;

	int tomm_nSmoothing;
	int tomm_symmetryIndex; // about which axes
	int tomm_nSymmetry; // number of symmetry axes
	bool tomm_materialSymmetry;
	// output
	Timer tomm_timer;
	MultiMaterialHistoryData tomm_HistoryData;

	// saved data
	vector<double> tomm_CostFractionsSaved;
	vector<vector<double>> tomm_PsudoDensitySaved;
	vector<vector<int>> tomm_MaterialIdsSaved;
	vector<int> tomm_MaterialPercentage;

	vector<vector<int>> tomm_NodeMaterial; // for each node, how many elems with each material , size: #nodes * #materials
	vector<vector<int>> tomm_NodeNeighbors;


	// cross-section
	vector<Pt> tomm_CrossSectionPlane;
	Pt tomm_CrossSectionCenter;
	vector<bool> tomm_showElem; // for cross-section
	bool tomm_isCrossSectionAvailable;
	// smoothing

    void smoothenElemField_MultiMaterial(vector<double>&elemField,int nIter);
	void  smoothenSensitivty_MultiMaterial(int nIter);
	// find
	void findMultiMaterialTemplates(int state = 0);
	// set
	void setMaterialIdOfAll(int mat);
	// compute
	void  computeNodalTSField_MultiMaterial();


	double computeCurrentCost();
	void computeSolidToVoidRankID();
	void computeVoidToSolidRankID();
	void computeSolidToSolidRankID_Decreasing();
	void computeSolidToSolidRankID_Increasing();

	void computeTopSensAtElemsApproximate_MultiMaterial();
	// solve 
	//
	void swapMaterials(double CostDecrement,int maxNumElems = 1e6);
	void swapMaterials_ReduceInefficiency(double CostDecrement, int maxNumElems = 1e6);
	int iterateToCost_MultiMaterial(double CostDecrement, double &objectiveConverged,int *nFEA);
	bool reduceCost(double CostDecrement,int maxNumElems = 1e6);
	bool reduceInefficiency(double CostDecrement,int maxNumElems = 1e6);
	void sortCostsDescending();
	
	bool checkConstraintsForMultiMaterial();
	bool transferToTopOptLibrary_StressCriterionSensitivity_MultiMaterial();
	void sortRankIDs(vector<RankedData> &tomm_Ranked);
	void pushToSavedData_MultiMaterial(void);
	void retrieveFromSavedData_MultiMaterial(int *nFEA);
	bool removeDisconnectedElements_MultiMaterial(double cutoff);

	int imposeManufacturingConstraintsOnElem(int elem);
	void imposeCyclicSymmetry_MultiMaterial(int index);
	int imposeSymmetryOnElem(int elem);
	int imposeDrawDirectionOnElem(int elem);
    int imposeExtrudeOnElem(int elem);
	
	

public:
	TopOptForMultiMaterial();
	~TopOptForMultiMaterial();
	// debug
	void enableMultiMaterialVerbose();
	void disableMultiMaterialVerbose();
	// check 
	bool isSavingMultiMaterialDesign(int skipStep);
	bool isNodeNeighborsFound();
	// initialize
	void initializeTopOptForMultiMaterial();
	void initializeToMostExpensiveMaterial();
	void initializeToLeastExpensiveMaterial();
	void initializeToRandomDistributionMaterial();
	// read/write
	int readCosts_MultiMaterial(char dmatFileName[]);

	// get
	int getClosestCounterAtCostFraction(double volFrac);
	double getCostFractionAtCounter(int counter);
	void getMultiMaterialHistoryData(size_t &size,vector<double> &volFractions,vector<double> &Compliance,vector<double> &maxStress,vector<double> &Stress_pNorm);
	bool getMultiMaterialTessellationElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3],double FI[3],double SR[3],double Gamma[3]);
    bool getMultiMaterialFinalIsoElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3]);
	TOPOPT_STRESSMODELS getMultiMaterialStressModel();
	bool getMultiMaterialVerbose(void);
	void getMultiMaterialQuantityOfInterest(TOPOPT_OBJECTIVES &quantity);
	void getMultiMaterialTopSens_Elem(vector<double> &TSElem);
	double getCurrentCostFraction_MultiMaterial();
	void getElemStressCriterionAdjointSolution_MultiMaterial(int elem,double adjointSol[24]);
	int getNumElemsOfMaterial(int matId);
	// get Failure Index
	// get for plotting
	void getMultiMaterialNodeDeformation(int node,double *u,double *v,double *w,double *MultiMaterial_stress);// at a node the (u, v, w) stress
    bool getMultiMaterialElasticitySolutionAt(Pt &pt,Vec &uvw,double stresses[6]);// at a point find the displacement and stress
	bool getMultiMaterialElasticitySolutionInCenterOfElem(int elem,double uvw[3],double stresses[6],double *MultiMaterial_stress);// at a point find the displacement and stress
	// set
	void setSymmetry_MultiMaterial(int symmetryIndex);
	void setTopOptMultiMaterialToCounter(int counter);
	void setNodeNeighborsStatus(bool status);
	// set liveVar 
	void setMultiMaterialNumberOfSmoothing(int nSmoothing);
	void setMultiMaterialQuantityOfInterest(const TOPOPT_OBJECTIVES &quantity);
	void setMultiMaterialNumSubSteps(int num);
	void setMultiMaterialSavePerStep(int savePerStep);
	void setMaterialGradingLevel(int gradinglevel);
	void setMaterialSymmetry(int materialSymmetry);
	// find
	void findMaterialPercentages();
	void findAllNodeMaterials();
	void findAllNodeNeighbors(int recursion);
	void getNodeMaterials(const int node,vector<int> &nodeMaterials);
	int getNeighboringElementsOfNodeRecursive(const int node,vector<int> &neighboringElemsOfElem,int nRecursions);
	// solve
    int solvePrimaryAndAdjointProblems_MultiMaterial(int *nFEA);

	int initializeToStiffestDesign(double CostDecrement, int *nFEA,int drawDirection,int symmetryIndex, bool keepElems = false);
	double TraceParetoForMultiMaterial(double desiredCostFrac,double CostDecrementMax, int maxFEA,int *nFEA, 
										bool(*callBackFunction)(bool saveIsoSurface),int drawDirection,
										int symmetryIndex,double *CostDecrementStart,double allocatedTimeInSecs);
	

	bool pushMultiMaterialInitialStaticSolutionToTessellation();
	bool pushMultiMaterialFinalStaticSolutionToIsoSurface(void);
	void saveTopOptMultiMaterial();

	// XSection
	void findCrossSectionPlane(int plane, int value);
	bool getCrossSectionPlane(vector<Pt> &crossSectionPlane,Pt &crossSectionCenter);
	void imposeCrossSection(int plane, int value);
	bool isElementShown(int elem);

};
#endif
