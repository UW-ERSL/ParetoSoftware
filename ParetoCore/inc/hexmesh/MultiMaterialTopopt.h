#ifndef MULTIMATERIALTOPOPT_H
#define MULTIMATERIALTOPOPT_H
#include "TopOptSingleLoadMultiConstraints.h"

#define MULTIMATERIALTOPOPT_VERBOSE 0


class MultiMaterialTopopt: public TopOptSingleLoadMultiConstraints {
public:// inherited classes can access these variables

	int mmto_nFEA;
	double *mmto_rankID;
	double *mmto_currentI; // current Inefficacy
	double *mmto_smoothCurrentI;
	double *mmto_alternateI; // alternate value of Inefficacy
	int *mmto_currentMaterial;
	int *mmto_isChanged; // for imposing symmetry
	int *mmto_isNeighbor; // bringing back adjacent dead elements
	int *mmto_ElemID,*mmto_MaterialID; // keeping track of sorted rankID
	double mmto_initialCost;
	double mmto_targetCost;
	double mmto_deltaCost;
	double mmto_initialCompliance;
	double mmto_initialDeformation;
	double mmto_initialStress;
	int mmto_nSensSmootheningIter;
	double *mmto_Costs;
	int mmto_nLoop;
	int mmto_SymmetryIndex;
	double mmto_nSymmetry;
	// StressCriterion Criterion
	double mmto_StressCriterionNorm0;
	double *mmto_StressCoef;
	double *mmto_MaterialStrength;
	double *mmto_StressCriterion;
	double *mmto_StressCriterionAtNodes;
	double *mmto_StressCriterionAdjointForce;
	double *mmto_StressCriterionAdjoint;
	double mmto_MaxStressCriterion;
	double mmto_MaxMaterialStrength; // for each material, used for scaling
	double *mmto_principalstresses;// the 6 stresses in the middle of each element
	//  Morphological Operations
	int *mmto_StructuringElemsXYZ19,*mmto_StructuringElemsXY,*mmto_StructuringElemsXZ,*mmto_StructuringElemsYZ,*mmto_StructuringElemsX,*mmto_StructuringElemsY,*mmto_StructuringElemsZ; // for each element, find six neighbors (3D sphere,xy,xz,yz) 
	int *mmto_stElemsXYZ19,*mmto_stElemsXY,*mmto_stElemsXZ,*mmto_stElemsYZ,*mmto_stElemsX,*mmto_stElemsY,*mmto_stElemsZ; // local
	int mmto_StructuringSize;
	double *mmto_sortedCost; // costs in decreasing order
	int *mmto_sortedcostId;
	//	for debugging
	bool mmto_verbose; // if set to true, print progress for debugging stuff
public:
	MultiMaterialTopopt();
	void initializeMultiMaterialTopopt(double SymmetryIndex);
	void findMultiMaterialTemplates();
	int findElemOfCenter(double x,double y,double z);
	int readCosts(char dmatFileName[]);
	void initializeToMostExpensive();
	double findMaxValue(double *v,int *id,int size );
	double computeCurrentCost();
	void computeSolidToVoidRankID();
	void computeSolidToSolidRankID();
	void computeSameMaterialSwapRankID(int *InElem,int *OutElem);	
	void setMaterialIdOfAll(int mat);
	void SwapElementsInAndOut(int *InElem,int *OutElem);
	void imposeSymmetryOnElem(int elem);
	void swapMaterials(double CostDecrement);
	void traceMultiMaterialPareto(double CostDecrement,bool useMorphology);
	void reduceCost(double CostDecrement);
	void reduceInefficacy(void);
	int findNeighboringElemsOfGeom();

	int readStrengths(char dmatFileName[]);
	double findMaxStressCriterionCoef();
	double findMaxMaterialStresngth();
	void computeStressCriterionSensitivity();
	void computeInitialStressCriterionSensitivity();
	double findMaxStressCriterion(int *whichelem);
	void getElemStressCriterionAdjointSolution(int elem,double adjointSol[24]);
	int solveStressCriterionAdjoint();
	void setStressCriterionAdjointForce();
	void computeStressCriterionAtElems();
	void computeStressCriterionAtNodes();
	void computeStressCoefficients();
	void dumpFailureValuesToFile(char *fileName);
	void getMinMaxNodeFailure(double *minVal,double *maxVal);
	void computePrincipalStressesAtElems();

	void erosionOnMesh(int method);
	void dilationOnMesh(int method);
	void openingOnMesh(int method);
	void closingOnMesh(int method);
	double findErosionOnMeshFlag3D(int elem);
	double findDilationOnMeshFlag3D(int elem);
	double findErosionOnMeshFlag2D(int elem);
	double findDilationOnMeshFlag2D(int elem);
	double findErosionOnMeshFlag1D(int elem);
	double findDilationOnMeshFlag1D(int elem);

	void erosionOnMaterial();
	void dilationOnMaterial();
	void openingOnMaterial();
	void closingOnMaterial();
	int findDilationOnMaterialFlag(int elem,int mat0);

	int getNumberOfSolidNeighboringElems(int node);
	int findStructuringElement(double xc,double yc,double zc);
	void findAllStructuringElems();
	void sortCostsDecreasing();
	bool getVerbose();
	void enableMMTOVerbose();
	void disableMMTOVerbose();

	int generateGCode(char dmatFileName[]);
	void initializeGCode(FILE* gcode);
	void finilizeGCode(FILE* gcode);
	void SliceAndMarch(FILE* gcode,double layerHeight,double filamentDiam,double feedRate);
	int findSegmentsInXdir(int seg[21],int mat[20],double y,double z,double Z_end);
	bool isSupportStructure(double x,double y,double z,double Z_end);
	int findElemOfPoint(double x,double y,double z);
	~MultiMaterialTopopt();

};

#endif