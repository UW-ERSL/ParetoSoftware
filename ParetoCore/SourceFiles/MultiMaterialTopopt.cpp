#include <math.h>
#include <stdlib.h>
#include <string> 
#ifndef __APPLE__
#include <omp.h>
#endif
#include "cmatrix.h"
#include "MultiMaterialTopopt.h"
#include "Timer.h"
#include <fstream>
#include <iostream>

using namespace std;

/**********************************************************************/
MultiMaterialTopopt::MultiMaterialTopopt()
{
	mmto_rankID = NULL;
	mmto_currentI = NULL; // current Inefficacy
	mmto_smoothCurrentI = NULL;
	mmto_alternateI = NULL; // alternate value of Inefficacy
	mmto_currentMaterial = NULL;
	mmto_isChanged = NULL; // for imposing symmetry
	mmto_isNeighbor = NULL;
	mmto_ElemID = NULL;
	mmto_MaterialID = NULL; // keeping track of sorted rankID
	mmto_Costs = NULL;

	mmto_StressCoef = NULL;
	mmto_MaterialStrength = NULL;
	mmto_StressCriterion = NULL;
	mmto_StressCriterionAtNodes = NULL;
	mmto_StressCriterionAdjointForce = NULL;
	mmto_StressCriterionAdjoint = NULL;
	mmto_MaxStressCriterion = 0.0;
	mmto_MaxMaterialStrength = 0.0;
	mmto_principalstresses = NULL;

	mmto_StructuringElemsXYZ19 = NULL; // useMorphology
	mmto_StructuringElemsXY = NULL;
	mmto_StructuringElemsXZ = NULL;
	mmto_StructuringElemsYZ = NULL;
	mmto_StructuringElemsX = NULL;
	mmto_StructuringElemsY = NULL;
	mmto_StructuringElemsZ = NULL;
	mmto_stElemsXYZ19 = NULL;
	mmto_stElemsXY = NULL;
	mmto_stElemsXZ = NULL;
	mmto_stElemsYZ = NULL;
	mmto_stElemsX = NULL;
	mmto_stElemsY = NULL;
	mmto_stElemsZ = NULL;

	mmto_sortedCost = NULL;
	mmto_sortedcostId = NULL;
	mmto_initialCost = 0.0;
	mmto_targetCost = 0.0;
	mmto_deltaCost = 0.0;
	mmto_nSensSmootheningIter = 0;
	mmto_nFEA = 0;
	mmto_StructuringSize = 19;
	mmto_verbose = false;
}
/**********************************************************************/
void MultiMaterialTopopt::initializeMultiMaterialTopopt(double SymmetryIndex)
{
	mmto_rankID = (double *)malloc(h_nElems*sizeof(double));
	mmto_currentI = (double *)malloc(h_nElems*sizeof(double)); 
	mmto_smoothCurrentI = (double *)malloc(h_nElems*sizeof(double));
	mmto_alternateI = (double *)malloc(h_nElems*sizeof(double)); // alternate value of Inefficacy
	mmto_currentMaterial = (int *)malloc(h_nElems*sizeof(int));
	mmto_isChanged = (int *)malloc(h_nElems*sizeof(int)); // for imposing symmetry
	mmto_isNeighbor = (int *)malloc(h_nElems*sizeof(int));
	mmto_ElemID = (int *)malloc(h_nElems*sizeof(int));
	mmto_MaterialID = (int *)malloc(h_nElems*sizeof(int));
	mmto_Costs = (double *)malloc(h_nMaterials*sizeof(double));

	mmto_StressCoef = (double *)malloc(12*h_nMaterials*sizeof(double));
	mmto_MaterialStrength = (double *)malloc(9*h_nMaterials*sizeof(double));
	mmto_StressCriterion = (double *)malloc(h_nElems*sizeof(double)); 
	mmto_StressCriterionAtNodes = (double *)malloc(h_nNodes*sizeof(double));
	mmto_StressCriterionAdjointForce = (double *)malloc(3*h_nNodes*sizeof(double));
	mmto_StressCriterionAdjoint = (double *)malloc(3*h_nNodes*sizeof(double));
	mmto_principalstresses = (double *)malloc(3*h_nElems*sizeof(double));

	mmto_StructuringElemsXYZ19 = (int *)malloc(19*h_nElems*sizeof(int));
	mmto_StructuringElemsXY = (int *)malloc(9*h_nElems*sizeof(int));
	mmto_StructuringElemsXZ = (int *)malloc(9*h_nElems*sizeof(int));
	mmto_StructuringElemsYZ = (int *)malloc(9*h_nElems*sizeof(int));
	mmto_StructuringElemsX = (int *)malloc(3*h_nElems*sizeof(int));
	mmto_StructuringElemsY = (int *)malloc(3*h_nElems*sizeof(int));
	mmto_StructuringElemsZ = (int *)malloc(3*h_nElems*sizeof(int));
	mmto_stElemsXYZ19 = (int *)malloc(19*h_nElems*sizeof(int));
	mmto_stElemsXY = (int *)malloc(9*h_nElems*sizeof(int));
	mmto_stElemsXZ = (int *)malloc(9*sizeof(int)); 
	mmto_stElemsYZ = (int *)malloc(9*sizeof(int)); 
	mmto_stElemsX = (int *)malloc(3*sizeof(int));
	mmto_stElemsY = (int *)malloc(3*sizeof(int));
	mmto_stElemsZ = (int *)malloc(3*sizeof(int));

	mmto_sortedCost = (double *)malloc(h_nMaterials*sizeof(double));
	mmto_sortedcostId = (int *)malloc(h_nMaterials*sizeof(int));

	memset(mmto_rankID,0,h_nElems*sizeof(double));
	memset(mmto_currentI,0,h_nElems*sizeof(double));
	memset(mmto_smoothCurrentI,0,h_nElems*sizeof(double));
	memset(mmto_alternateI,0,h_nElems*sizeof(double));
	memset(mmto_currentMaterial,0,h_nElems*sizeof(int));
	memset(mmto_isChanged,0,h_nElems*sizeof(int));
	memset(mmto_isNeighbor,0,h_nElems*sizeof(int));
	memset(mmto_ElemID,0,h_nElems*sizeof(int));
	memset(mmto_MaterialID,0,h_nElems*sizeof(int));
	memset(mmto_Costs,0,h_nMaterials*sizeof(double));

	memset(mmto_StressCoef,0,12*h_nMaterials*sizeof(double));
	memset(mmto_MaterialStrength,0,9*h_nMaterials*sizeof(double));
	memset(mmto_StressCriterion,0,h_nElems*sizeof(double));
	memset(mmto_StressCriterionAtNodes,0,h_nNodes*sizeof(double));
	memset(mmto_StressCriterionAdjointForce,0,3*h_nNodes*sizeof(double));
	memset(mmto_StressCriterionAdjoint,0,3*h_nNodes*sizeof(double));
	memset(mmto_principalstresses,0,3*h_nElems*sizeof(double));

	memset(mmto_StructuringElemsXYZ19,-1,19*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsXY,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsXZ,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsYZ,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsX,-1,3*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsY,-1,3*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsZ,-1,3*h_nElems*sizeof(int));
	memset(mmto_stElemsXYZ19,-1,19*sizeof(int)); 
	memset(mmto_stElemsXY,-1,9*sizeof(int));
	memset(mmto_stElemsXZ,-1,9*sizeof(int));
	memset(mmto_stElemsYZ,-1,9*sizeof(int));
	memset(mmto_stElemsX,-1,3*sizeof(int));
	memset(mmto_stElemsY,-1,3*sizeof(int));
	memset(mmto_stElemsZ,-1,3*sizeof(int));

	memset(mmto_sortedCost,0,h_nMaterials*sizeof(double));
	memset(mmto_sortedcostId,0,h_nMaterials*sizeof(int));
	double eta = 0.1;
	double filterRadius = eta*h_modelScale;

	mmto_nSensSmootheningIter = (int) floor(filterRadius/sqrt(h_dx*h_dx + h_dy*h_dy + h_dz*h_dz));
	mmto_nSensSmootheningIter = MIN(MAX(1,mmto_nSensSmootheningIter),4);

	printf("TopOpt: TS smoothening iterations = %d \n",ts_nSmoothingIterations);

	mmto_initialCost = 0.0;
	mmto_targetCost = 0.0;
	mmto_deltaCost = 0.0;

	mmto_SymmetryIndex = (int)NEARESTINT(SymmetryIndex);

	// find nSymmetry for updating cost while imposing symmetry
	if (mmto_SymmetryIndex==1 || mmto_SymmetryIndex==2 || mmto_SymmetryIndex==4)
		mmto_nSymmetry = 1.0;
	else if (mmto_SymmetryIndex==3 || mmto_SymmetryIndex==5 || mmto_SymmetryIndex==6)
		mmto_nSymmetry = 2.0;
	else if (mmto_SymmetryIndex==7)
		mmto_nSymmetry = 3.0;
	else
		mmto_nSymmetry = 0.0;
}
/**********************************************************************/
void MultiMaterialTopopt::traceMultiMaterialPareto(double CostDecrement,bool useMorphology)
{
	int success = 0,Hoff_success = 0;
	computeSolidToVoidRankID();
	reduceCost(CostDecrement);
	reduceInefficacy();

	deleteStaticFEAResults();// any previous results are not valid due to topology change
	findNeighboringElemsOfNodes();
	findTemplates();
	success = solvePrimaryAndAdjointProblems(&mmto_nFEA); // solve
	if (mmto_verbose) printf("success = %i\n",success);
	if (ts_topoptQuantity == MULTIMATERIAL_STRESS){
		Hoff_success = solveStressCriterionAdjoint();
		computeStressCriterionAtNodes();
		setVonMisesAtAllNodes(mmto_StressCriterionAtNodes); // temporary
		if (mmto_verbose) printf("Hoff_success = %i\n",Hoff_success);
	}
	removeHangingElems();
}
/**********************************************************************/
int MultiMaterialTopopt::findElemOfCenter(double x,double y,double z)
{
	double center[3];
	double eps = h_dx/1000.0;
	int gridnode,node,elem,codedElem,nodeIndex;
	int Element = -1;
	// find closest grid node
	int nx = (int)NEARESTINT((x-h_xMin)/h_dx);
	int ny = (int)NEARESTINT((y-h_yMin)/h_dy);
	int nz = (int)NEARESTINT((z-h_zMin)/h_dz);
	gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
	// find mesh node
	if (gridnode<0 || gridnode>=(h_nelx+1)*(h_nely+1)*(h_nelz+1)) return -1;
	node = h_gridNodeToRealNode[gridnode];
	// loop over parenting elems
	for (int neighbor=0;neighbor<h_maxNeighboringElements;neighbor++){
		codedElem = h_neighborElems[h_nNodes*neighbor+node];
		nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
		elem = codedElem - nodeIndex*MAXELEMS;
		getElemCenter(elem,center);
		if(abs(center[0]-x)>eps) continue;
		if(abs(center[1]-y)>eps) continue;
		if(abs(center[2]-z)>eps) continue;
		Element =  elem;
		break;
	}
	if (Element == -1)
		if (mmto_verbose)
			printf("No Element with given center found...!\n");
	return Element;
}
/**********************************************************************/
void MultiMaterialTopopt::findMultiMaterialTemplates()
{
	// compute template matrices and initialize to most expensive material
	int materialindex = 0;
	int elem;
	// set all to zero
	for (elem=0;elem<h_nElems;elem++)
		setMaterialIdOfElem(materialindex,elem);
	// create one template of each material
	for (elem=0;elem<h_nMaterials;elem++){
		materialindex= elem;
		setMaterialIdOfElem(materialindex,elem);
	}
	// compute templates
	findTemplates();
	computeKeOfTemplates();
	// initialize
	initializeToMostExpensive();
	findTemplates();
	deleteStaticFEAResults();
}
/**********************************************************************/
int MultiMaterialTopopt::readCosts(char dmatFileName[])
{
	FILE *dmat = fopen(dmatFileName,"r");
	char str[256];

	if (dmat == NULL) {
		printf("Could not open file %s\n",dmatFileName);
		return 0;
	}
	printf("----------------------------------------------------\n");
	printf("reading material costs ...\n");
	while (fscanf(dmat, "%s", str) != EOF) {
		if (strcmp(str,"#Cost")==0){
			for (int i = 0; i < h_nMaterials; i++){
				fscanf(dmat,"%lf",&(mmto_Costs[i]));
				printf("Costs[%i] = %g\n",i,mmto_Costs[i]);
			}
			printf("---------------------------------------------------\n");
			fclose(dmat);
			return 1;
		}
	}
	printf("No Costs are provided!\n");
	printf("---------------------------------------------------\n");
	return 0;
}
/**********************************************************************/
void MultiMaterialTopopt::initializeToMostExpensive()
{
	printf("----------------------------------------------------\n");
	printf("initializing to most expensive material ...\n");
	int material,elem;
	double  MaxCost = findMaxValue(mmto_Costs,&material,h_nMaterials);
	for (elem=0;elem<h_nElems;elem++){
		setMaterialIdOfElem(material,elem);
		mmto_currentMaterial[elem] = material;
	}
	printf("initialized to material %i\n",material);
}
/**********************************************************************/
double MultiMaterialTopopt::findMaxValue(double *v,int *id,int size )
{
	double max = v[0];
	*id = 0;
	int i;
	for (i=1;i<size;i++){
		if (v[i]>max){
			max = v[i];
			*id = i;
		}
	}
	return max;
}
/**********************************************************************/
double MultiMaterialTopopt::computeCurrentCost()
{
	double currentCost = 0.0;
	for (int elem=0;elem < h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		currentCost += mmto_Costs[getMaterialIdOfElem(elem)];
	}
	return currentCost;
}
/**********************************************************************/
void MultiMaterialTopopt::computeSolidToVoidRankID()
{
	double dI ,dC;
	int material; 
	memcpy(mmto_currentMaterial,h_materialIdOfElem,h_nElems*sizeof(int));

	memset(mmto_rankID,0,h_nElems*sizeof(double));
	memset(mmto_ElemID,0,h_nElems*sizeof(int));
	memcpy(mmto_MaterialID,h_materialIdOfElem,h_nElems*sizeof(int));

	if (ts_topoptQuantity == MULTIMATERIAL_STRESS){
		computeStressCriterionSensitivity();
	}
	else{
		computeTopSensAtElems();
	}
	smoothenElemField(ts_topSensElem,mmto_nSensSmootheningIter);
	int elem;
	for (elem=0;elem<h_nElems;elem++){
		mmto_ElemID[elem] = elem;
		
		mmto_currentI[elem] = ts_topSensElem[elem];
		if (getElemPseudoDensity(elem)<0.5) continue;
		
		//
		
		material = mmto_currentMaterial[elem];
		dI = mmto_currentI[elem];
		if (h_keepElem[elem]) dI *= 1e8;
		dC = -mmto_Costs[material];
		mmto_rankID[elem] = -dI/dC;
		mmto_MaterialID[elem] = -1;
	}
}
/**********************************************************************/
void MultiMaterialTopopt::computeSolidToSolidRankID()
{
	int material1,material2,elem;
	double dI,dC,r;
	for (elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		material1 = mmto_currentMaterial[elem];
		material2 = getMaterialIdOfElem(elem);
		if (material1 == material2) continue;
		dC = mmto_Costs[material2]-mmto_Costs[material1];
		if (dC<0.){
			dI = mmto_smoothCurrentI[elem]-mmto_alternateI[elem];
			r = -dI/dC;
			if (mmto_rankID[elem] > r){
				mmto_rankID[elem] = r;
				mmto_MaterialID[elem] = material2;
			}
		}
		else if (dC==0.){
			dI = mmto_smoothCurrentI[elem]-mmto_alternateI[elem];
			if (dI>0.) continue;
			if (mmto_rankID[elem] > dI){
				mmto_rankID[elem] = dI;
				mmto_MaterialID[elem] = material2;
			}
		}
	}
}
/**********************************************************************/
void MultiMaterialTopopt::computeSameMaterialSwapRankID(int *InElem,int *OutElem)
{
	int o_elem;
	for (o_elem=0;o_elem<h_nElems;o_elem++){
		OutElem[o_elem] = o_elem; 
		if (mmto_isNeighbor[o_elem] == 0) continue; // not neighbor of geom
		for (int i_elem=0;i_elem<h_nElems;i_elem++){
			if (getElemPseudoDensity(i_elem)==0.) continue;
			if (getMaterialIdOfElem(i_elem)!=getMaterialIdOfElem(o_elem))continue;
			if ((mmto_currentI[i_elem]-mmto_currentI[o_elem]) < mmto_alternateI[o_elem]){
				mmto_alternateI[o_elem] = (mmto_currentI[i_elem]-mmto_currentI[o_elem]);
				InElem[o_elem] = i_elem;
			}
		}
	}
}
/**********************************************************************/
void MultiMaterialTopopt::setMaterialIdOfAll(int mat)
{
	int elem;
//#pragma omp parallel for private(elem)
	for (elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		setMaterialIdOfElem(mat,elem);
	}
}
/**********************************************************************/
void MultiMaterialTopopt::swapMaterials(double CostDecrement)
{
	int currentElem,alternateMaterial,currentMaterial;
	int i=-1;
	double dcost = 0.0;
	memset(mmto_isChanged,0,h_nElems*sizeof(int));
	while (dcost < CostDecrement){	
		i++;
		
		currentElem = mmto_ElemID[i];
		alternateMaterial = mmto_MaterialID[i];
		currentMaterial = mmto_currentMaterial[currentElem]; 
		if (getElemPseudoDensity(currentElem) == 0)continue;
		if (mmto_isChanged[currentElem] == 1)continue;

		if (alternateMaterial == -1){
			if (h_keepElem[currentElem]) continue;
			setPseudoDensityOfElem(currentElem,0.0);
			dcost += pow(2.0,mmto_nSymmetry)*mmto_Costs[currentMaterial];
			mmto_isChanged[currentElem] = 1;
			imposeSymmetryOnElem(currentElem);
		}
		else{
			setMaterialIdOfElem(alternateMaterial,currentElem);
			dcost += pow(2.0,mmto_nSymmetry)*(mmto_Costs[currentMaterial]-mmto_Costs[alternateMaterial]);
			mmto_isChanged[currentElem] = 1;
			imposeSymmetryOnElem(currentElem);
		}
		
	}
}
/**********************************************************************/
void MultiMaterialTopopt::SwapElementsInAndOut(int *InElem,int *OutElem)
{
	int counter = 0;
	int o_elem,i_elem;
	for (int elem=0;elem<h_nElems;elem++){
		counter++;
		o_elem = OutElem[elem];
		i_elem = InElem[elem];
		if (mmto_alternateI[elem]>=0.0) break;
		if (mmto_isChanged[o_elem] == 1) continue;
		if (mmto_isChanged[i_elem] == 1) continue;
		setPseudoDensityOfElem(o_elem,1.0);
		setPseudoDensityOfElem(i_elem,0.0);
		mmto_isChanged[o_elem] = 1;
		mmto_isChanged[i_elem] = 1;
		imposeSymmetryOnElem(o_elem);
		imposeSymmetryOnElem(i_elem);
		if (mmto_verbose) printf("Outside element = %i,Inside element = %i\n",o_elem,i_elem);
	}
}
/**********************************************************************/
void MultiMaterialTopopt::reduceCost(double CostDecrement)
{
	memset(mmto_alternateI,0,h_nElems*sizeof(double));
	if (ts_topoptQuantity == MULTIMATERIAL_STRESS){
		computeStressCriterionSensitivity();
	}
	else{
		computeTopSensAtElems();
	}
	connectivityTSSmoothener();
	memcpy(mmto_smoothCurrentI,ts_topSensElem,h_nElems*sizeof(double));
	for (int mat=0;mat<h_nMaterials;mat++){
		setMaterialIdOfAll(mat);
		findTemplates();
		if (ts_topoptQuantity == MULTIMATERIAL_STRESS){
			computeStressCriterionSensitivity();
		}
		else{
			computeTopSensAtElems();
		}
		connectivityTSSmoothener();
		memcpy(mmto_alternateI,ts_topSensElem,h_nElems*sizeof(double));
		computeSolidToSolidRankID();
	}
	memcpy(h_materialIdOfElem,mmto_currentMaterial,h_nElems*sizeof(int));
	bubbleSortIncreasingDualIndex(mmto_rankID,mmto_ElemID,mmto_MaterialID,h_nElems); // sort
	swapMaterials(CostDecrement);
	findTemplates();
}
/**********************************************************************/
void MultiMaterialTopopt::reduceInefficacy(void)
{
	int *InElem,*OutElem;
	InElem = (int *)malloc(h_nElems*sizeof(int));
	OutElem = (int *)malloc(h_nElems*sizeof(int));

	memset(mmto_isChanged,0,h_nElems*sizeof(int));
	memset(mmto_isNeighbor,0,h_nElems*sizeof(int));
	memset(mmto_alternateI,0,h_nElems*sizeof(double));
	memset(InElem,0,h_nElems*sizeof(int));
	memset(OutElem,0,h_nElems*sizeof(int));
	int noutNeighbors = findNeighboringElemsOfGeom();
	if (noutNeighbors>0){
		computeSameMaterialSwapRankID(InElem,OutElem);
		bubbleSortIncreasingDualIndex(mmto_alternateI,InElem,OutElem,h_nElems);
		if (mmto_alternateI[0] < 0.0){
			SwapElementsInAndOut(InElem,OutElem);
			findTemplates();
		}
	}
	safeFree(InElem);
	safeFree(OutElem);
}
/**********************************************************************/
int MultiMaterialTopopt::getNumberOfSolidNeighboringElems(int node)
{
	int elems[8];
	int nNeighbors = getNeighboringElemsOfNode(node,elems);
	int nSolidNeighbors = 0;
	for (int i=0;i<nNeighbors;i++){
		if (getElemPseudoDensity(elems[i]) == 0) continue;
		nSolidNeighbors++;
	}
	return nSolidNeighbors;
}
/**********************************************************************/
int MultiMaterialTopopt::findNeighboringElemsOfGeom()
{
	int Nodes[8];
	double bnodes;
	int nNeighbors;
	int num=0;
	memset(mmto_isNeighbor,0,sizeof(int)*h_nElems);
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem) != 0.) continue;
		 bnodes = 0.0;
		getElemNodes(elem,Nodes);
		for (int node=0;node<8;node++){
			nNeighbors = getNumberOfSolidNeighboringElems(Nodes[node]);
			if (nNeighbors<8 && nNeighbors>0)
				bnodes += 0.125;
		}
		if(bnodes > 0.49) {
			mmto_isNeighbor[elem] = 1; 
			num++;
		}
	}
	return num;
}
/********************************Stress Criteria**************************************/
int MultiMaterialTopopt::readStrengths(char dmatFileName[])
{
	FILE *dmat = fopen(dmatFileName,"r");
	char str[256];

	if (dmat == NULL) {
		printf("Could not open file %s\n",dmatFileName);
		return 0;
	}
	printf("----------------------------------------------------\n");
	printf("reading material strengths ...\n");
	while (fscanf(dmat, "%s", str) != EOF) {
		if (strcmp(str,"#Strength")==0){
			for (int i = 0; i < h_nMaterials; i++){
				printf("\nStrengths[%i] = ",i);
				fscanf(dmat,"%lf %lf %lf %lf %lf %lf %lf %lf %lf ",&(mmto_MaterialStrength[9*i]),&(mmto_MaterialStrength[9*i+1]),
					&(mmto_MaterialStrength[9*i+2]),&(mmto_MaterialStrength[9*i+3]),
					&(mmto_MaterialStrength[9*i+4]),&(mmto_MaterialStrength[9*i+5]),
					&(mmto_MaterialStrength[9*i+6]),&(mmto_MaterialStrength[9*i+7]),&(mmto_MaterialStrength[9*i+8]));
				printf("%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f \n ",(mmto_MaterialStrength[9*i]),(mmto_MaterialStrength[9*i+1]),
					(mmto_MaterialStrength[9*i+2]),(mmto_MaterialStrength[9*i+3]),
					(mmto_MaterialStrength[9*i+4]),(mmto_MaterialStrength[9*i+5]),
					(mmto_MaterialStrength[9*i+6]),(mmto_MaterialStrength[9*i+7]),(mmto_MaterialStrength[9*i+8]));
			}
			printf("---------------------------------------------------\n");
			fclose(dmat);
			findMaxMaterialStresngth(); // for scaling
			return 1;
		}
	}
	printf("No strengths are provided!\n");
	printf("---------------------------------------------------\n");
	return 0;
}
/**********************************************************************/
double MultiMaterialTopopt::findMaxMaterialStresngth ()
{
	mmto_MaxMaterialStrength = 0.0;
	for (int i=0;i<9*h_nMaterials;i++){
		mmto_MaxMaterialStrength = (mmto_MaterialStrength[i]>mmto_MaxMaterialStrength)? mmto_MaterialStrength[i]:mmto_MaxMaterialStrength;
	}
	return mmto_MaxMaterialStrength;
}
/**********************************************************************/
void MultiMaterialTopopt::computeStressCoefficients()
{
	double Xt,Yt,Zt,Xc,Yc,Zc,S12,S13,S23;
	if (mmto_verbose) printf("mmto_MaxMaterialStrength = %g\n",mmto_MaxMaterialStrength);
	int method = 1; //1:Tsai-Wu, 2:Hoffman, 3:Tsai-Hill
	for (int mat=0;mat< h_nMaterials; mat++){
		if (mmto_verbose) printf("mmto_StressCoef[%i] =",mat);
		Xt = mmto_MaterialStrength[9*mat]/mmto_MaxMaterialStrength; Xc = mmto_MaterialStrength[9*mat+1]/mmto_MaxMaterialStrength;
		Yt = mmto_MaterialStrength[9*mat+2]/mmto_MaxMaterialStrength; Yc = mmto_MaterialStrength[9*mat+3]/mmto_MaxMaterialStrength;
		Zt = mmto_MaterialStrength[9*mat+4]/mmto_MaxMaterialStrength; Zc = mmto_MaterialStrength[9*mat+5]/mmto_MaxMaterialStrength;
		S12 = mmto_MaterialStrength[9*mat+6]/mmto_MaxMaterialStrength; S13 = mmto_MaterialStrength[9*mat+7]/mmto_MaxMaterialStrength;
		S23 = mmto_MaterialStrength[9*mat+8]/mmto_MaxMaterialStrength;

		if (method == 1){
			// Tsai-Wu Criterion
			mmto_StressCoef[12*mat] = 1.0/Xt-1.0/Xc;
			mmto_StressCoef[12*mat+1] = 1.0/Yt-1.0/Yc;
			mmto_StressCoef[12*mat+2] = 1.0/Zt-1.0/Zc;
			mmto_StressCoef[12*mat+3] = -0.5/sqrt(Xt*Xc*Yt*Yc);
			mmto_StressCoef[12*mat+4] = -0.5/sqrt(Xt*Xc*Zt*Zc);
			mmto_StressCoef[12*mat+5] = -0.5/sqrt(Yt*Yc*Zt*Zc);
			mmto_StressCoef[12*mat+6] = 1.0/(Xt*Xc);
			mmto_StressCoef[12*mat+7] = 1.0/(Yt*Yc);
			mmto_StressCoef[12*mat+8] = 1.0/(Zt*Zc);
			mmto_StressCoef[12*mat+9] = 1.0/(S23*S23);
			mmto_StressCoef[12*mat+10] = 1.0/(S13*S13);
			mmto_StressCoef[12*mat+11] = 1.0/(S12*S12);
		}
		else if (method == 2){
			// Hoffman Criterion
			mmto_StressCoef[12*mat] = 1.0/Xt-1.0/Xc;
			mmto_StressCoef[12*mat+1] = 1.0/Yt-1.0/Yc;
			mmto_StressCoef[12*mat+2] = 1.0/Zt-1.0/Zc;
			mmto_StressCoef[12*mat+3] = -0.5*(1.0/(Xt*Xc)+1.0/(Yt*Yc)-1.0/(Zt*Zc));
			mmto_StressCoef[12*mat+4] = -0.5*(1.0/(Zt*Zc)+1.0/(Xt*Xc)-1.0/(Yt*Yc));
			mmto_StressCoef[12*mat+5] = -0.5*(1.0/(Yt*Yc)+1.0/(Zt*Zc)-1.0/(Xt*Xc));
			mmto_StressCoef[12*mat+6] = 1.0/(Xt*Xc);
			mmto_StressCoef[12*mat+7] = 1.0/(Yt*Yc);
			mmto_StressCoef[12*mat+8] = 1.0/(Zt*Zc);
			mmto_StressCoef[12*mat+9] = 1.0/(S23*S23);
			mmto_StressCoef[12*mat+10] = 1.0/(S13*S13);
			mmto_StressCoef[12*mat+11] = 1.0/(S12*S12);
		}
		else if(method == 3){
			// Tsai-Hill
			mmto_StressCoef[12*mat] = 0.0;
			mmto_StressCoef[12*mat+1] = 0.0;
			mmto_StressCoef[12*mat+2] = 0.0;
			mmto_StressCoef[12*mat+3] = -0.5*(1.0/(Xt*Xc)+1.0/(Yt*Yc)-1.0/(Zt*Zc));
			mmto_StressCoef[12*mat+4] = -0.5*(1.0/(Zt*Zc)+1.0/(Xt*Xc)-1.0/(Yt*Yc));
			mmto_StressCoef[12*mat+5] = -0.5*(1.0/(Yt*Yc)+1.0/(Zt*Zc)-1.0/(Xt*Xc));
			mmto_StressCoef[12*mat+6] = 1.0/(Xt*Xc);
			mmto_StressCoef[12*mat+7] = 1.0/(Yt*Yc);
			mmto_StressCoef[12*mat+8] = 1.0/(Zt*Zc);
			mmto_StressCoef[12*mat+9] = 1.0/(S23*S23);
			mmto_StressCoef[12*mat+10] = 1.0/(S13*S13);
			mmto_StressCoef[12*mat+11] = 1.0/(S12*S12);
		}
		if (mmto_verbose){
			for (int k=0;k<12;k++) printf("%g ",mmto_StressCoef[12*mat+k]);
		}
	}
}
/**********************************************************************/
void MultiMaterialTopopt::computePrincipalStressesAtElems()
{
	memset(mmto_principalstresses,0,3*h_nElems*sizeof(double));
	double s[6]; // stress components
	double I1,I2,I3,phi;
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		getElemStresses(elem,s);
		I1 = s[0]+s[1]+s[2];
		I2 = s[0]*s[1]+s[1]*s[2]+s[2]*s[0]-s[3]*s[3]-s[4]*s[4]-s[5]*s[5];
		I3 = s[0]*s[1]*s[2]-s[0]*s[5]*s[5]-s[1]*s[4]*s[4]-s[2]*s[3]*s[3]+2.0*s[3]*s[4]*s[5];
		phi = (1.0/3.0)*acos((2*I1*I1*I1-9*I1*I2+27*I3)/(2*pow(I1*I1-3.0*I2,1.5)));
		mmto_principalstresses[3*elem] = (I1/3.0)+(2.0/3.0)*(sqrt(I1*I1-3.0*I2))*cos(phi);
		mmto_principalstresses[3*elem+1] = (I1/3.0)+(2.0/3.0)*(sqrt(I1*I1-3.0*I2))*cos(phi+2.0*PI/3.0);
		mmto_principalstresses[3*elem+2] = (I1/3.0)+(2.0/3.0)*(sqrt(I1*I1-3.0*I2))*cos(phi+4.0*PI/3.0);
	}
}
/**********************************************************************/
void MultiMaterialTopopt::computeStressCriterionAtElems()
{
	double s[6]; // stress components
	int mat;
	memset(mmto_StressCriterion,0,h_nElems*sizeof(double));
	//computePrincipalStressesAtElems();
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		mat = getMaterialIdOfElem(elem); 
		getElemStresses(elem,s);
		// scale stresses
		for (int i=0;i<6;i++){
			s[i] /= mmto_MaxMaterialStrength;
		}
		// compute elem stress criterion
		mmto_StressCriterion[elem] = mmto_StressCoef[12*mat]*s[0]+mmto_StressCoef[12*mat+1]*s[1]+mmto_StressCoef[12*mat+2]*s[2];
		mmto_StressCriterion[elem] += 2*(mmto_StressCoef[12*mat+3]*s[0]*s[1]+mmto_StressCoef[12*mat+4]*s[0]*s[2]+mmto_StressCoef[12*mat+5]*s[1]*s[2]);
		mmto_StressCriterion[elem] += mmto_StressCoef[12*mat+6]*s[0]*s[0]+mmto_StressCoef[12*mat+7]*s[1]*s[1]+mmto_StressCoef[12*mat+8]*s[2]*s[2];
		mmto_StressCriterion[elem] += mmto_StressCoef[12*mat+9]*s[5]*s[5]+mmto_StressCoef[12*mat+10]*s[4]*s[4]+mmto_StressCoef[12*mat+11]*s[3]*s[3];
	}
	int whichelem=0;
	findMaxStressCriterion(&whichelem);
	if (mmto_verbose)printf("max StressCriterion @ %i = %g\n",whichelem,mmto_MaxStressCriterion);
}
/**********************************************************************/
void MultiMaterialTopopt::computeStressCriterionAtNodes()
{
	//Compute strains and stresses in the middle of every element and average out at the node
	int j,elem,node,maxElem,codedElemNum,nodeIndex,count;
	double nodeMaxStress;
	memset(mmto_StressCriterionAtNodes,0,h_nNodes*sizeof(double));
	if (mmto_verbose)
		printf("mmto: Post-processing at Nodes \n");
	nodeMaxStress = 0;
	// compute strains and stresses at each node by averaging
	for (node = 0; node < h_nNodes;node++) {
		maxElem = h_nNeighboringElems[node];// how many elements connected to node
		count = 0;
		for (j =0; j < maxElem; j++) {//
			codedElemNum = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
			nodeIndex = codedElemNum/MAXELEMS;
			elem = codedElemNum - nodeIndex*MAXELEMS;
			if (getElemPseudoDensity(elem) <0.5) continue;
			count++;
			mmto_StressCriterionAtNodes[node] += mmto_StressCriterion[elem];
		}
		count = MAX(1.0,count);
		//average out, and also find max and min across all nodes
		mmto_StressCriterionAtNodes[node] /=  count;
	}
}
/**********************************************************************/
double MultiMaterialTopopt::findMaxStressCriterion(int *whichelem)
{
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem) < 0.5) continue;
		if (mmto_MaxStressCriterion<mmto_StressCriterion[elem]){
			mmto_MaxStressCriterion = mmto_StressCriterion[elem];
			*whichelem = elem;
		}
	}
	return mmto_MaxStressCriterion;
}
/**********************************************************************/
void MultiMaterialTopopt::setStressCriterionAdjointForce()
{
	int elem,m,nodes[8],j,freeEncoded,ufree,vfree,wfree,materialID;
	double stresses[6],val,ce;

	double scalingFactor;
	memset(mmto_StressCriterionAdjointForce,0,3*h_nNodes*sizeof(double));// set the force vector to zero
	scalingFactor = 0.0;
	for (elem = 0; elem < h_nElems;elem++) {
		if (getElemPseudoDensity(elem) <0.5) continue;
		if(mmto_StressCriterion[elem]<0.0) continue;
		scalingFactor += pow(mmto_StressCriterion[elem],ts_pNormIndex);
		//printf("mmto_StressCriterion[elem] = %g\n",mmto_StressCriterion[elem]);
		//printf("\n scalingFactor = %g\n",scalingFactor);getchar();
	}
	//printf("\n scalingFactor = %g\n",scalingFactor);
	scalingFactor = pow(scalingFactor,1.0/ts_pNormIndex-1.0);
	for (elem = 0; elem < h_nElems;elem++) {
		if (getElemPseudoDensity(elem) <0.5) continue;
		getElemNodes(elem,nodes);
		getElemStresses(elem,stresses); //get stresses in the middle of element
		// scale stresses
		for (int i=0;i<6;i++) {

			stresses[i] /= mmto_MaxMaterialStrength;
			//printf("s = %g\n",stresses[i]);
		}
		if(mmto_StressCriterion[elem]<0.0) continue;
		ce = pow(mmto_StressCriterion[elem],ts_pNormIndex-1.0);
		/*printf("Fe[materialID][0][3*m+j]) = %g\n",Fe[materialID][0][3*m+j]);
		getchar();*/
		materialID = getElemMaterialID(elem);
		for (m = 0; m < 8; m++) {
			for (j = 0; j < 3;j++) {
				val = 0.0;
				val +=  mmto_StressCoef[12*materialID]*Fe[materialID][0][3*m+j];
				val += mmto_StressCoef[12*materialID+1]*Fe[materialID][1][3*m+j];
				val += mmto_StressCoef[12*materialID+2]*Fe[materialID][2][3*m+j];
				val += 2*mmto_StressCoef[12*materialID+3]*(Fe[materialID][0][3*m+j]*stresses[1]+Fe[materialID][1][3*m+j]*stresses[0]);
				val += 2*mmto_StressCoef[12*materialID+4]*(Fe[materialID][0][3*m+j]*stresses[2]+Fe[materialID][2][3*m+j]*stresses[0]);
				val += 2*mmto_StressCoef[12*materialID+5]*(Fe[materialID][1][3*m+j]*stresses[2]+Fe[materialID][2][3*m+j]*stresses[1]);
				val += 2*mmto_StressCoef[12*materialID+6]*Fe[materialID][0][3*m+j]*stresses[0];
				val += 2*mmto_StressCoef[12*materialID+7]*Fe[materialID][1][3*m+j]*stresses[1];
				val += 2*mmto_StressCoef[12*materialID+8]*Fe[materialID][2][3*m+j]*stresses[2];
				val += 2*mmto_StressCoef[12*materialID+9]*Fe[materialID][5][3*m+j]*stresses[5]; //s23
				val += 2*mmto_StressCoef[12*materialID+10]*Fe[materialID][4][3*m+j]*stresses[4]; //s13
				val += 2*mmto_StressCoef[12*materialID+11]*Fe[materialID][3][3*m+j]*stresses[3]; //s12
				mmto_StressCriterionAdjointForce[3*nodes[m]+j] += ce*val/mmto_MaxMaterialStrength;
				//printf("f = %g\n",mmto_StressCriterionAdjointForce[3*nodes[m]+j]);getchar();
			}
		}
	}
	
	for (m = 0; m < h_nNodes; m++) {
		mmto_StressCriterionAdjointForce[3*m]  *= scalingFactor;
		mmto_StressCriterionAdjointForce[3*m+1] *= scalingFactor;
		mmto_StressCriterionAdjointForce[3*m+2] *= scalingFactor;
		//printf("f = %g\n",mmto_StressCriterionAdjointForce[3*m]);getchar();
	}
	// scale
	double maxStressCriterionAdjointForce = 0.0;
	for (m = 0; m < 3*h_nNodes; m++) 
		maxStressCriterionAdjointForce = (maxStressCriterionAdjointForce<mmto_StressCriterionAdjointForce[m])? mmto_StressCriterionAdjointForce[m]:maxStressCriterionAdjointForce;

	// Remove forces on fixed dof
	for (m = 0; m < h_nNodes; m++) {
		freeEncoded = h_free[m];
		wfree = freeEncoded/4;
		vfree = (freeEncoded - wfree*4)/2;
		ufree = (freeEncoded - wfree*4 -vfree*2);
		if (ufree==0)
			mmto_StressCriterionAdjointForce[3*m] = 0;
		if (vfree==0)
			mmto_StressCriterionAdjointForce[3*m+1] = 0;
		if (wfree==0)
			mmto_StressCriterionAdjointForce[3*m+2] = 0;
	}
}
/**********************************************************************/
int MultiMaterialTopopt::solveStressCriterionAdjoint()
{
	double *solSave,*fSave;
	// assumes that the primary problem has been solved above
	computeStressCriterionAtElems();

	solSave = (double *)malloc(3*h_nNodes*sizeof(double));
	fSave = (double *)malloc(3*h_nNodes*sizeof(double));
	getSolutionVector(solSave);// save the primary solution
	copy(h_f,fSave);// save the primary force
	setStressCriterionAdjointForce();
	if (mmto_verbose) printf("StressCriterion: Adjoint Force norm = %e \n",Norm(mmto_StressCriterionAdjointForce));
	setForceVector(mmto_StressCriterionAdjointForce);// don't copy directly since AFElasticity takes care pushing to GPU if necessary
	initializeSolution();
	mmto_nFEA +=1;
	if (mmto_verbose) printf("StressCriterion: Solving adjoint problem \n");
	if (!solveElasticityStatic()){
		setSolutionAtAllNodes(solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary
		setForceVector(fSave);// copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
		safeFree(solSave);
		safeFree(fSave);
		return FAILURE;  
	}
	// don't post-process for adjoint field, stresses will be messed-up
	getSolutionVector(mmto_StressCriterionAdjoint);// copy the solution to adjoint
	if (mmto_verbose) printf("StressCriterion: Adjoint Solution norm = %e \n",Norm(mmto_StressCriterionAdjoint));

	setSolutionAtAllNodes(solSave);// copy the original solution, don't copy directly since AFElasticity takes care pushing to GPU if necessary

	setForceVector(fSave);// copy the original force don't copy directly since AFElasticity takes care pushing to GPU if necessary
	if (mmto_verbose) printf("StressCriterion: Completed primary and adjoint \n");
	safeFree(solSave);
	safeFree(fSave);
	return 1;
}
/**********************************************************************/
void MultiMaterialTopopt::computeStressCriterionSensitivity()
{
	double elemSol[24],adjointSol[24];
	double Q,Compliance,TSnorm = 1e-10;
	double wt_compliance = 0.02;
	double wt_stress = 1.0-wt_compliance;
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		getElemSolution(elem,elemSol); 
		getElemStressCriterionAdjointSolution(elem,adjointSol); 
		Q = computeElemInnerProduct(elem,elemSol,adjointSol);
		Compliance = computeElemInnerProduct(elem,elemSol,elemSol);
		//ts_topSensElem[elem] = Q/mmto_StressCriterionNorm0;
		ts_topSensElem[elem] = wt_stress*Q/mmto_StressCriterionNorm0+ wt_compliance*Compliance/TSComplianceNorm0;
	}
}
/**********************************************************************/
void MultiMaterialTopopt::computeInitialStressCriterionSensitivity()
{
	double elemSol[24],adjointSol[24];
	double Hof,Compliance;
	double minHof = 1e12,maxHof = 1e-12;
	double minCompliance = 1e12,maxCompliance = -1e12;
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		getElemSolution(elem,elemSol); 
		getElemStressCriterionAdjointSolution(elem,adjointSol); 
		Hof = computeElemInnerProduct(elem,elemSol,adjointSol);
		Compliance = computeElemInnerProduct(elem,elemSol,elemSol);
		minHof = (minHof>Hof)? Hof:minHof;
		maxHof = (maxHof<Hof)? Hof:maxHof;
		minCompliance = (minCompliance>Compliance)? Compliance:minCompliance;
		maxCompliance = (maxCompliance<Compliance)? Compliance:maxCompliance;
	}
	mmto_StressCriterionNorm0 = maxHof - minHof;
	TSComplianceNorm0 = maxCompliance - minCompliance;

}
/**********************************************************************/
void MultiMaterialTopopt::getElemStressCriterionAdjointSolution(int elem,double adjointSol[24])
{
	int node;
	// gather adjoint solution for that element
	for (int m = 0;m < 8; m++) {
		node = h_elems[h_nElems*m + elem];
		adjointSol[3*m] = mmto_StressCriterionAdjoint[3*node];
		adjointSol[3*m+1] = mmto_StressCriterionAdjoint[3*node+1];
		adjointSol[3*m+2] = mmto_StressCriterionAdjoint[3*node+2];
	}
}
/**********************************************************************/
void MultiMaterialTopopt::dumpFailureValuesToFile(char *fileName)
{
	FILE *fptr;
	int node;
	double max = -1e12;
	for (node=0;node<h_nNodes;node++){
		max = (h_nodevonmises[node]>max)? h_nodevonmises[node]:max;
	}
	fptr = fopen(fileName,"w");
	for (node = 0; node < h_nNodes;node++) {
		fprintf(fptr,"%g",h_nodevonmises[node]/max);//starting elem is 0
		fprintf(fptr,"\n");
	}
	fclose(fptr);
}
/**********************************************************************/
void MultiMaterialTopopt::getMinMaxNodeFailure(double *minVal,double *maxVal)
{
	*minVal = 1e12;
	*maxVal = -1e12;
	for (int node=0;node<h_nNodes;node++){
		*maxVal = (h_nodevonmises[node]>*maxVal)? h_nodevonmises[node]:*maxVal;
		*minVal = (h_nodevonmises[node]<*minVal)? h_nodevonmises[node]:*minVal;
	}
}
/********************************Morphological Operations**************************************/
void MultiMaterialTopopt::findAllStructuringElems()
{
	int elem0;
	memset(mmto_StructuringElemsXYZ19,-1,19*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsXY,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsXZ,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsYZ,-1,9*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsX,-1,3*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsY,-1,3*h_nElems*sizeof(int));
	memset(mmto_StructuringElemsZ,-1,3*h_nElems*sizeof(int));

	double xstart = h_xMin+0.5*h_dx;
	double xend = xstart+(h_nelx)*h_dx;
	double ystart = h_yMin+0.5*h_dy;
	double yend = ystart+(h_nely)*h_dy;
	double zstart = h_zMin+0.5*h_dz;
	double zend = zstart+(h_nelz)*h_dz;
	for (double zc = zstart;zc<zend;zc+=h_dz){
		for (double yc = ystart;yc<yend;yc+=h_dy){
			for (double xc = xstart;xc<xend;xc+=h_dx){
				elem0 = findStructuringElement(xc,yc,zc);
				if (elem0<0) continue;
				for (int i=0;i<19;i++)
					mmto_StructuringElemsXYZ19[19*elem0+i] = mmto_stElemsXYZ19[i];
				for (int i=0;i<9;i++){
					mmto_StructuringElemsXY[9*elem0+i] = mmto_stElemsXY[i];
					mmto_StructuringElemsXZ[9*elem0+i] = mmto_stElemsXZ[i];
					mmto_StructuringElemsYZ[9*elem0+i] = mmto_stElemsYZ[i];
				}
				for (int i=0;i<3;i++){
					mmto_StructuringElemsX[3*elem0+i] = mmto_stElemsX[i];
					mmto_StructuringElemsY[3*elem0+i] = mmto_stElemsY[i];
					mmto_StructuringElemsZ[3*elem0+i] = mmto_stElemsZ[i];
				}

			}
		}
	}
}
/**********************************************************************/
int MultiMaterialTopopt::findStructuringElement(double xc,double yc,double zc)
{
	memset(mmto_stElemsXYZ19,-1,19*sizeof(int)); 
	memset(mmto_stElemsXY,-1,9*sizeof(int));
	memset(mmto_stElemsXZ,-1,9*sizeof(int));
	memset(mmto_stElemsYZ,-1,9*sizeof(int));
	memset(mmto_stElemsX,-1,3*sizeof(int));
	memset(mmto_stElemsY,-1,3*sizeof(int));
	memset(mmto_stElemsZ,-1,3*sizeof(int));

	mmto_stElemsXYZ19[0] = findElemOfCenter(xc,yc,zc);
	mmto_stElemsXY[0] = mmto_stElemsXYZ19[0];
	mmto_stElemsXZ[0] = mmto_stElemsXYZ19[0];
	mmto_stElemsYZ[0] = mmto_stElemsXYZ19[0];
	mmto_stElemsX[0] = mmto_stElemsXYZ19[0];
	mmto_stElemsY[0] = mmto_stElemsXYZ19[0];
	mmto_stElemsZ[0] = mmto_stElemsXYZ19[0];

	mmto_stElemsXYZ19[1] = findElemOfCenter(xc-h_dx,yc,zc);
	mmto_stElemsXYZ19[2] = findElemOfCenter(xc,yc-h_dy,zc);
	mmto_stElemsXYZ19[3] = findElemOfCenter(xc,yc,zc-h_dz);
	mmto_stElemsXYZ19[4] = findElemOfCenter(xc+h_dx,yc,zc);
	mmto_stElemsXYZ19[5] = findElemOfCenter(xc,yc+h_dy,zc);
	mmto_stElemsXYZ19[6] = findElemOfCenter(xc,yc,zc+h_dz);
	mmto_stElemsXYZ19[7] = findElemOfCenter(xc-h_dx,yc-h_dy,zc);
	mmto_stElemsXYZ19[8] = findElemOfCenter(xc-h_dx,yc,zc-h_dz);
	mmto_stElemsXYZ19[9] = findElemOfCenter(xc,yc-h_dy,zc-h_dz);
	mmto_stElemsXYZ19[10] = findElemOfCenter(xc+h_dx,yc+h_dy,zc);
	mmto_stElemsXYZ19[11] = findElemOfCenter(xc+h_dx,yc,zc+h_dz);
	mmto_stElemsXYZ19[12] = findElemOfCenter(xc,yc+h_dy,zc+h_dz);
	mmto_stElemsXYZ19[13] = findElemOfCenter(xc-h_dx,yc+h_dy,zc);
	mmto_stElemsXYZ19[14] = findElemOfCenter(xc-h_dx,yc,zc+h_dz);
	mmto_stElemsXYZ19[15] = findElemOfCenter(xc,yc-h_dy,zc+h_dz);
	mmto_stElemsXYZ19[16] = findElemOfCenter(xc+h_dx,yc-h_dy,zc);
	mmto_stElemsXYZ19[17] = findElemOfCenter(xc+h_dx,yc,zc-h_dz);
	mmto_stElemsXYZ19[18] = findElemOfCenter(xc,yc+h_dy,zc-h_dz);

	mmto_stElemsXY[1] = findElemOfCenter(xc-h_dx,yc-h_dy,zc);
	mmto_stElemsXY[2] = findElemOfCenter(xc,yc-h_dy,zc);
	mmto_stElemsXY[3] = findElemOfCenter(xc+h_dx,yc-h_dy,zc);
	mmto_stElemsXY[4] = findElemOfCenter(xc-h_dx,yc,zc);
	mmto_stElemsXY[5] = findElemOfCenter(xc+h_dx,yc,zc);
	mmto_stElemsXY[6] = findElemOfCenter(xc-h_dx,yc+h_dy,zc);
	mmto_stElemsXY[7] = findElemOfCenter(xc,yc+h_dy,zc);
	mmto_stElemsXY[8] = findElemOfCenter(xc+h_dx,yc+h_dy,zc);

	mmto_stElemsXZ[1] = findElemOfCenter(xc-h_dx,yc,zc-h_dz);
	mmto_stElemsXZ[2] = findElemOfCenter(xc,yc,zc-h_dz);
	mmto_stElemsXZ[3] = findElemOfCenter(xc+h_dx,yc,zc-h_dz);
	mmto_stElemsXZ[4] = findElemOfCenter(xc-h_dx,yc,zc);
	mmto_stElemsXZ[5] = findElemOfCenter(xc+h_dx,yc,zc);
	mmto_stElemsXZ[6] = findElemOfCenter(xc-h_dx,yc,zc+h_dz);
	mmto_stElemsXZ[7] = findElemOfCenter(xc,yc,zc+h_dz);
	mmto_stElemsXZ[8] = findElemOfCenter(xc+h_dx,yc,zc+h_dz);

	mmto_stElemsYZ[1] = findElemOfCenter(xc,yc-h_dy,zc-h_dz);
	mmto_stElemsYZ[2] = findElemOfCenter(xc,yc,zc-h_dz);
	mmto_stElemsYZ[3] = findElemOfCenter(xc,yc+h_dy,zc-h_dz);
	mmto_stElemsYZ[4] = findElemOfCenter(xc,yc-h_dy,zc);
	mmto_stElemsYZ[5] = findElemOfCenter(xc,yc+h_dy,zc);
	mmto_stElemsYZ[6] = findElemOfCenter(xc,yc-h_dy,zc+h_dz);
	mmto_stElemsYZ[7] = findElemOfCenter(xc,yc,zc+h_dz);
	mmto_stElemsYZ[8] = findElemOfCenter(xc,yc+h_dy,zc+h_dz);

	mmto_stElemsX[1] = findElemOfCenter(xc-h_dx,yc,zc);
	mmto_stElemsX[2] = findElemOfCenter(xc+h_dx,yc,zc);

	mmto_stElemsY[1] = findElemOfCenter(xc,yc-h_dy,zc);
	mmto_stElemsY[2] = findElemOfCenter(xc,yc+h_dy,zc);

	mmto_stElemsZ[1] = findElemOfCenter(xc,yc,zc-h_dz);
	mmto_stElemsZ[2] = findElemOfCenter(xc,yc,zc+h_dz);
	return mmto_stElemsXYZ19[0];
}
/**********************************************************************/
void MultiMaterialTopopt::erosionOnMesh(int method)
{
	double *temp_PseudoDensity;
	double flag;
	temp_PseudoDensity = (double *)malloc(h_nElems*sizeof(double));
	memset(temp_PseudoDensity,1,h_nElems*sizeof(double));
	for (int elem = 0;elem<h_nElems;elem++){
		if (method == 3)
			flag = findErosionOnMeshFlag3D(elem); // flag:0 set to void, flag:1 set to solid
		else if (method == 2)
			flag = findErosionOnMeshFlag2D(elem); // flag:0 set to void, flag:1 set to solid
		else 
			flag = findErosionOnMeshFlag1D(elem); // flag:0 set to void, flag:1 set to solid

		if (h_keepElem[elem]) 
			flag =1.0;
		temp_PseudoDensity[elem] = flag;
	}
	setPseudoDensity(temp_PseudoDensity);
	findNeighboringElemsOfNodes();
	findTemplates();
	safeFree(temp_PseudoDensity);
}
/**********************************************************************/
void MultiMaterialTopopt::dilationOnMesh(int method)
{
	double *temp_PseudoDensity;
	double flag;
	temp_PseudoDensity = (double *)malloc(h_nElems*sizeof(double));
	memset(temp_PseudoDensity,1,h_nElems*sizeof(double));
	for (int elem = 0;elem<h_nElems;elem++){
		if ( method == 3)
			flag = findDilationOnMeshFlag3D(elem); // flag:0 set to void, flag:1 set to solid
		else if ( method == 2)
			flag = findDilationOnMeshFlag2D(elem);
		else
			flag = findDilationOnMeshFlag1D(elem);
		if (h_keepElem[elem]) 
			flag =1.0;
		temp_PseudoDensity[elem] = flag;
	}
	setPseudoDensity(temp_PseudoDensity);
	findNeighboringElemsOfNodes();
	findTemplates();
	safeFree(temp_PseudoDensity);
}
/**********************************************************************/
double MultiMaterialTopopt::findErosionOnMeshFlag3D(int elem)
{
	int currentelem;
	for (int i=0;i<19;i++){
		currentelem = mmto_StructuringElemsXYZ19[19*elem+i];
		if (currentelem<0) continue;
		if (getElemPseudoDensity(currentelem) < 0.5) 
			return 0.0;
	}
	return 1.0;
}
/**********************************************************************/
double MultiMaterialTopopt::findDilationOnMeshFlag3D(int elem)
{
	int currentelem;
	for (int i=0;i<19;i++){
		currentelem = mmto_StructuringElemsXYZ19[19*elem+i];
		if (currentelem<0) continue;
		if (getElemPseudoDensity(currentelem) > 0.5) 
			return 1.0;
	}
	return 0.0;
}
/**********************************************************************/
double MultiMaterialTopopt::findErosionOnMeshFlag2D(int elem)
{
	if (getElemPseudoDensity(elem) < 0.5) 
			return 0.0;
	int currentElem;
	// xy
	double xy[9];
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsXY[9*elem+i];
		if (currentElem<0) 
			xy[i]=0.0;
		else
			xy[i] = getElemPseudoDensity(currentElem); 
	} 
	// case 1
		if (xy[1]>0.5 && xy[2]>0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 2
		if (xy[1]>0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]>0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 3
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]>0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]>0.5)
			return 0.0;
	// case 4
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]>0.5 && xy[8]>0.5)
			return 0.0;
	// case 5
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 6
		if (xy[1]>0.5 && xy[2]>0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 7
		if (xy[1]<0.5 && xy[2]>0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 8
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]>0.5 && xy[8]<0.5)
			return 0.0;
	// case 9
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]>0.5 && xy[8]>0.5)
			return 0.0;
	// case 10
		if (xy[1]>0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]>0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 11
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]>0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 12
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]>0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 13
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]>0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]>0.5)
			return 0.0;
	// case 14
		if (xy[1]>0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 15
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 16
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 0.0;
	// case 17
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]<0.5 && xy[5]<0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]>0.5)
			return 0.0;

	// xz
	double xz[9];
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsXZ[9*elem+i];
		if (currentElem<0)
			xz[i]=0.0;
		else
			xz[i] = getElemPseudoDensity(currentElem); 
	} 
	// case 1
		if (xz[1]>0.5 && xz[2]>0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 2
		if (xz[1]>0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]>0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 3
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]>0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]>0.5)
			return 0.0;
	// case 4
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]>0.5 && xz[8]>0.5)
			return 0.0;
	// case 5
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 6
		if (xz[1]>0.5 && xz[2]>0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 7
		if (xz[1]<0.5 && xz[2]>0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 8
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]>0.5 && xz[8]<0.5)
			return 0.0;
	// case 9
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]>0.5 && xz[8]>0.5)
			return 0.0;
	// case 10
		if (xz[1]>0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]>0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 11
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]>0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 12
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]>0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 13
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]>0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]>0.5)
			return 0.0;
	// case 14
		if (xz[1]>0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 15
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 16
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 0.0;
	// case 17
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]<0.5 && xz[5]<0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]>0.5)
			return 0.0;
	// yz
	double yz[9];
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsYZ[9*elem+i];
		if (currentElem<0) 
			yz[i]=0.0;
		else
			yz[i] = getElemPseudoDensity(currentElem); 
	} 
	// case 1
		if (yz[1]>0.5 && yz[2]>0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 2
		if (yz[1]>0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]>0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 3
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]>0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]>0.5)
			return 0.0;
	// case 4
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]>0.5 && yz[8]>0.5)
			return 0.0;
	// case 5
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 6
		if (yz[1]>0.5 && yz[2]>0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 7
		if (yz[1]<0.5 && yz[2]>0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 8
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]>0.5 && yz[8]<0.5)
			return 0.0;
	// case 9
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]>0.5 && yz[8]>0.5)
			return 0.0;
	// case 10
		if (yz[1]>0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]>0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 11
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]>0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 12
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]>0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 13
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]>0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]>0.5)
			return 0.0;
	// case 14
		if (yz[1]>0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 15
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 16
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 0.0;
	// case 17
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]<0.5 && yz[5]<0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]>0.5)
			return 0.0;
	return 1.0;
}
/**********************************************************************/
double MultiMaterialTopopt::findDilationOnMeshFlag2D(int elem)
{
	if (getElemPseudoDensity(elem) > 0.5) 
			return 1.0;
	// xy
	double xy[9];
	int currentElem;
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsXY[9*elem+i];
		if (currentElem<0) 
			xy[i]=0.0;
		else
			xy[i] = getElemPseudoDensity(currentElem); 
	}
	// case 1
		if (xy[1]<0.5 && xy[2]<0.5 && xy[3]<0.5 && xy[4]>0.5 && xy[5]>0.5 && xy[6]>0.5 && xy[7]>0.5 && xy[8]>0.5)
			return 1.0;
	// case 2
		if (xy[1]>0.5 && xy[2]>0.5 && xy[3]<0.5 && xy[4]>0.5 && xy[5]<0.5 && xy[6]>0.5 && xy[7]>0.5 && xy[8]<0.5)
			return 1.0;
	// case 3
		if (xy[1]<0.5 && xy[2]>0.5 && xy[3]>0.5 && xy[4]<0.5 && xy[5]>0.5 && xy[6]<0.5 && xy[7]>0.5 && xy[8]>0.5)
			return 1.0;
	// case 4
		if (xy[1]>0.5 && xy[2]>0.5 && xy[3]>0.5 && xy[4]>0.5 && xy[5]>0.5 && xy[6]<0.5 && xy[7]<0.5 && xy[8]<0.5)
			return 1.0;
	// case 5
		if (xy[1]>0.5 && xy[2]>0.5 && xy[3]>0.5 && xy[4]>0.5 && xy[5]>0.5 && xy[6]>0.5 && xy[7]>0.5 && xy[8]>0.5)
			return 1.0;
	// xz
	double xz[9];
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsXZ[9*elem+i];
		if (currentElem<0)
			xz[i]=0.0;
		else
			xz[i] = getElemPseudoDensity(currentElem); 
	} 
	// case 1
		if (xz[1]<0.5 && xz[2]<0.5 && xz[3]<0.5 && xz[4]>0.5 && xz[5]>0.5 && xz[6]>0.5 && xz[7]>0.5 && xz[8]>0.5)
			return 1.0;
	// case 2
		if (xz[1]>0.5 && xz[2]>0.5 && xz[3]<0.5 && xz[4]>0.5 && xz[5]<0.5 && xz[6]>0.5 && xz[7]>0.5 && xz[8]<0.5)
			return 1.0;
	// case 3
		if (xz[1]<0.5 && xz[2]>0.5 && xz[3]>0.5 && xz[4]<0.5 && xz[5]>0.5 && xz[6]<0.5 && xz[7]>0.5 && xz[8]>0.5)
			return 1.0;
	// case 4
		if (xz[1]>0.5 && xz[2]>0.5 && xz[3]>0.5 && xz[4]>0.5 && xz[5]>0.5 && xz[6]<0.5 && xz[7]<0.5 && xz[8]<0.5)
			return 1.0;
	// case 5
		if (xz[1]>0.5 && xz[2]>0.5 && xz[3]>0.5 && xz[4]>0.5 && xz[5]>0.5 && xz[6]>0.5 && xz[7]>0.5 && xz[8]>0.5)
			return 1.0;

	// yz
	double yz[9];
	for (int i=0;i<9;i++){
		currentElem = mmto_StructuringElemsYZ[9*elem+i];
		if (currentElem<0)
			yz[i]=0.0;
		else
			yz[i] = getElemPseudoDensity(currentElem); 
	}  
	// case 1
		if (yz[1]<0.5 && yz[2]<0.5 && yz[3]<0.5 && yz[4]>0.5 && yz[5]>0.5 && yz[6]>0.5 && yz[7]>0.5 && yz[8]>0.5)
			return 1.0;
	// case 2
		if (yz[1]>0.5 && yz[2]>0.5 && yz[3]<0.5 && yz[4]>0.5 && yz[5]<0.5 && yz[6]>0.5 && yz[7]>0.5 && yz[8]<0.5)
			return 1.0;
	// case 3
		if (yz[1]<0.5 && yz[2]>0.5 && yz[3]>0.5 && yz[4]<0.5 && yz[5]>0.5 && yz[6]<0.5 && yz[7]>0.5 && yz[8]>0.5)
			return 1.0;
	// case 4
		if (yz[1]>0.5 && yz[2]>0.5 && yz[3]>0.5 && yz[4]>0.5 && yz[5]>0.5 && yz[6]<0.5 && yz[7]<0.5 && yz[8]<0.5)
			return 1.0;
	// case 5
		if (yz[1]>0.5 && yz[2]>0.5 && yz[3]>0.5 && yz[4]>0.5 && yz[5]>0.5 && yz[6]>0.5 && yz[7]>0.5 && yz[8]>0.5)
			return 1.0;

	return 0.0;
}
/**********************************************************************/
double MultiMaterialTopopt::findErosionOnMeshFlag1D(int elem)
{
	if (getElemPseudoDensity(elem) < 0.5) 
			return 0.0;
	// x
	double x[3];
	int currentElem;
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsX[3*elem+i];
		if (currentElem<0) 
			x[i]=0.0;
		else
			x[i] = getElemPseudoDensity(currentElem); 
	}
	if (x[1]<0.5 && x[2]<0.5)
			return 0.0;

		// x
	double y[3];
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsY[3*elem+i];
		if (currentElem<0) 
			y[i]=0.0;
		else
			y[i] = getElemPseudoDensity(currentElem); 
	}
	if (y[1]<0.5 && y[2]<0.5)
			return 0.0;

		// x
	double z[3];
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsZ[3*elem+i];
		if (currentElem<0) 
			z[i]=0.0;
		else
			z[i] = getElemPseudoDensity(currentElem); 
	}
	if (z[1]<0.5 && z[2]<0.5)
			return 0.0;

	return 1.0;
}
/**********************************************************************/
double MultiMaterialTopopt::findDilationOnMeshFlag1D(int elem)
{
	if (getElemPseudoDensity(elem) > 0.5) 
			return 1.0;
	// x
	double x[3];
	int currentElem;
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsX[3*elem+i];
		if (currentElem<0) 
			x[i]=0.0;
		else
			x[i] = getElemPseudoDensity(currentElem); 
	}
	if (x[1]>0.5 && x[2]>0.5)
			return 1.0;

		// x
	double y[3];
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsY[3*elem+i];
		if (currentElem<0) 
			y[i]=0.0;
		else
			y[i] = getElemPseudoDensity(currentElem); 
	}
	if (y[1]>0.5 && y[2]>0.5)
			return 1.0;

		// x
	double z[3];
	for (int i=0;i<3;i++){
		currentElem = mmto_StructuringElemsZ[3*elem+i];
		if (currentElem<0) 
			z[i]=0.0;
		else
			z[i] = getElemPseudoDensity(currentElem); 
	}
	if (z[1]>0.5 && z[2]>0.5)
			return 1.0;

	return 0.0;
}
/**********************************************************************/
void MultiMaterialTopopt::openingOnMesh(int method)
{
	erosionOnMesh(method);
	dilationOnMesh(method);
}
/**********************************************************************/
void MultiMaterialTopopt::closingOnMesh(int method)
{
	dilationOnMesh(method);
	erosionOnMesh(method);
}
/**********************************************************************/
void MultiMaterialTopopt::dilationOnMaterial()
{
	// most expensive --> least expensive
	int *temp_MaterialID;
	int currentmat0,currentmat1;
	temp_MaterialID = (int *)malloc(h_nElems*sizeof(int));
	memcpy(temp_MaterialID,h_materialIdOfElem,h_nElems*sizeof(int));
	for (int mat = 0;mat <h_nMaterials;mat++){
		currentmat0 = mmto_sortedcostId[mat];
		for (int elem = 0;elem<h_nElems;elem++){
			if (getElemPseudoDensity(elem) < 0.5) continue; 
			currentmat1 = findDilationOnMaterialFlag(elem,currentmat0); // flag:0 set to void, flag:1 set to solid

			temp_MaterialID[elem] = currentmat1;
		}
		memcpy(h_materialIdOfElem,temp_MaterialID,h_nElems*sizeof(int));
	}
	findTemplates();
	safeFree(temp_MaterialID);
}
/**********************************************************************/
int MultiMaterialTopopt::findDilationOnMaterialFlag(int elem,int mat0)
{
	int currentelem;
	for (int i=0;i<mmto_StructuringSize;i++){
		currentelem = mmto_StructuringElemsXYZ19[mmto_StructuringSize*elem+i];
		if (currentelem<0) continue;
		if (getElemPseudoDensity(currentelem) < 0.5) continue; 
		if (getMaterialIdOfElem(currentelem) == mat0) 
			return mat0;
	}
	return getMaterialIdOfElem(elem);
}
/**********************************************************************/
void MultiMaterialTopopt::erosionOnMaterial()
{
	// least expensive --> most expensive
	int *temp_MaterialID;
	int currentmat0,currentmat1;
	temp_MaterialID = (int *)malloc(h_nElems*sizeof(int));

	for (int mat = h_nMaterials-1;mat >=0;--mat){
		memcpy(temp_MaterialID,h_materialIdOfElem,h_nElems*sizeof(int));
		currentmat0 = mmto_sortedcostId[mat];
		for (int elem = 0;elem<h_nElems;elem++){
			if (getElemPseudoDensity(elem) < 0.5) continue; 
			currentmat1 = findDilationOnMaterialFlag(elem,currentmat0); // flag:0 set to void, flag:1 set to solid
			temp_MaterialID[elem] = currentmat1;
		}
		memcpy(h_materialIdOfElem,temp_MaterialID,h_nElems*sizeof(int));
	}
	findTemplates();
	safeFree(temp_MaterialID);
}
/**********************************************************************/
void MultiMaterialTopopt::openingOnMaterial()
{
	erosionOnMaterial();
	dilationOnMaterial();
}
/**********************************************************************/
void MultiMaterialTopopt::closingOnMaterial()
{
	dilationOnMaterial();
	erosionOnMaterial();
}
/**********************************************************************/
void MultiMaterialTopopt::sortCostsDecreasing()
{
	int i, j,index;
	double temp;
	memcpy(mmto_sortedCost,mmto_Costs,h_nMaterials*sizeof(double));
	for (i =0; i < h_nMaterials;i++)
		mmto_sortedcostId[i] = i;

	for (i = (h_nMaterials - 1); i > 0; i--){
		for (j = 1; j <= i; j++) {
			if (mmto_sortedCost[j-1] < mmto_sortedCost[j]) {// swap
				temp = mmto_sortedCost[j-1];
				mmto_sortedCost[j-1] = mmto_sortedCost[j];
				mmto_sortedCost[j] = temp;
				index = mmto_sortedcostId[j-1];
				mmto_sortedcostId[j-1]= mmto_sortedcostId[j];
				mmto_sortedcostId[j] = index;
			}
		}
	}
}
/**********************************************************************/
void MultiMaterialTopopt::imposeSymmetryOnElem(int elem)
{
	int otherGridElem;
	int matID = getMaterialIdOfElem(elem);
	double pseudodensity = getElemPseudoDensity(elem);
	double center[3];
	getElemCenter(elem,center);
	double x = center[0];
	double y = center[1];
	double z = center[2];
	
	if (mmto_SymmetryIndex == 1){
		// only x
		otherGridElem = findElemOfCenter(-x,y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 2){
		// only y
		otherGridElem = findElemOfCenter(x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 4){
		// only z
		otherGridElem = findElemOfCenter(x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 3){
		// only x , y
		otherGridElem = findElemOfCenter(-x,y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(-x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 5){
		// only x , z
		otherGridElem = findElemOfCenter(-x,y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(-x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 6){
		// only y , z
		otherGridElem = findElemOfCenter(x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,-y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
	else if (mmto_SymmetryIndex == 7){
		// x, y , z
		otherGridElem = findElemOfCenter(-x,y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(-x,-y,z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(-x,y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(x,-y,-z);
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
		otherGridElem = findElemOfCenter(-x,-y,-z);
		
		if (mmto_isChanged[otherGridElem] == 0) {
			setPseudoDensityOfElem(otherGridElem,pseudodensity);
			setMaterialIdOfElem(matID,otherGridElem);
			mmto_isChanged[otherGridElem] = 1;
		}
	}
}
/**********************************************************************/
bool MultiMaterialTopopt::getVerbose()
{
	return mmto_verbose;
}
/**********************************************************************/
void MultiMaterialTopopt::enableMMTOVerbose()
{
	mmto_verbose = true;
}
/**********************************************************************/
void MultiMaterialTopopt::disableMMTOVerbose()
{
	mmto_verbose = false;
}
/********************************Generating G-Code**************************************/
int MultiMaterialTopopt::generateGCode(char prjFileName[])
{
	char nMat[5];
	char gFileName[256]; 
	double layerHeight = 0.03; // mm
	double filamentDiam = 1.75; // mm
	double feedRate;
	//itoa(h_nMaterials,nMat,10);
	sprintf(nMat,"%d",h_nMaterials);
	
	int nChar = strlen(prjFileName);
	strncat(gFileName,prjFileName,nChar-4);
	strcat(gFileName,"_");
	strcat(gFileName,nMat);
	strcat(gFileName,".gcode");
	printf("gFileName = %s\n",gFileName);
	FILE *gcode = fopen(gFileName,"w");

	printf("----------------------------------------------------\n");
	printf("generating G-code ...\n");
	// header comments
	fprintf(gcode,";filename = %s \n",prjFileName);
	fprintf(gcode,";machine = daVinciF10 \n");
	fprintf(gcode,"; layer_height = 0.2 \n");
	fprintf(gcode,"; material = abs \n");

	// initialize printer
	initializeGCode(gcode);
	// slice in z and walk along x then y

	feedRate  = 1;
	SliceAndMarch(gcode,layerHeight,filamentDiam,feedRate);
	finilizeGCode(gcode);

	fclose (gcode);
	return 0;
}
/**********************************************************************/
void MultiMaterialTopopt::initializeGCode(FILE* gcode)
{
	fprintf(gcode,"G21 ; set units to millimeters\n");
	fprintf(gcode,"M160 S4; set number; of materials extruder can handle\n");
	fprintf(gcode,"M106 S0; set fan speed and start\n");
	fprintf(gcode,"M190 S100 ; wait for bed temperature to be reached\n");
	fprintf(gcode,"M104 S230 ; set temperature\n");
	fprintf(gcode,"M109 S230 ; wait for temperature to be reached\n");
	fprintf(gcode,"G90 ; use absolute coordinates\n");
	fprintf(gcode,"M82 ; use absolute distances for extrusion\n");
	fprintf(gcode,"G92 E0\n");
}
/**********************************************************************/
void MultiMaterialTopopt::finilizeGCode(FILE* gcode)
{
	fprintf(gcode,"M73 P100 ; end build progress\n");
	fprintf(gcode,"G162 X Y F2500; home X and Y axes\n");
	fprintf(gcode,"M109 S0 T0; set bed temperature to 0\n");
	fprintf(gcode,"M104 S0 T0; set 1st extruder temperature to 0\n");
	fprintf(gcode,"M104 S0 T1; set 2nd extruder temperature to 0\n");
	fprintf(gcode,"M18; disable all stepper motors\n");
	fprintf(gcode,"M70 P5; Display \"We <3 Making Things!\" message\n");
	fprintf(gcode,"M72 P1; Play Ta-Da song\n");
}
/**********************************************************************/
void MultiMaterialTopopt::SliceAndMarch(FILE* gcode,double layerHeight,double filamentDiam,double feedRate)
{ 
	int nSlicesPerElem_z = NEARESTINT(h_dz/layerHeight);
	int nSlices_z = h_nelz*nSlicesPerElem_z; // maximum # slices

	double X_start = h_xMin + h_dx/2.0 ;
	double Y_start = h_yMin + filamentDiam/2.0;
	double Z_start = layerHeight;
	double X_end = h_xMin+(h_nelx-1)*h_dx;
	double Y_end = h_yMin+(h_nely-1)*h_dy;
	double Z_end = Z_start+h_nelz*h_dz;

	printf("xs=%g,xe=%g,ys=%g,ye=%g,zs=%g,ze=%g\n",X_start,X_end,Y_start,Y_end,Z_start,Z_end);
	fprintf(gcode,"G1 Z0.300 F7800.000\n");
	fprintf(gcode,"G1 X%.3f Y%.3f E0.000 F7800.000\n",X_start,Y_start);
	fprintf(gcode,"G1 E1.00000 F1800.000\n");

	int slice = 0; // slice index
	int nx = 0; // x index
	int ny = 0; // y index
	bool isMatDiff = false;
	int nseg;
	int seg[21],mat[20];
	double x,y,z;
	for ( z = Z_start;z<=Z_end;z+=layerHeight){	
		for ( y = Y_start;y<=Y_end;y+=filamentDiam){
			for (int i=0;i<20;i++){
				seg[2*i]=0;seg[2*i+1]=0;
				mat[i] = 0;nseg = 0;
			}
			nseg = findSegmentsInXdir(seg,mat,y,z,Z_end);
			x = seg[0];

			fprintf(gcode,"G0 X%.3f Y%.3f Z%.3f\n",x,y,z); // rapid movement
			for (int n = 0;n<nseg;n++){
				printf("z = %g,nseg = %i,mat[%i] = %i\n",z,nseg,n,mat[n]);getchar();
				x = seg[n];
				// write GCode 
				if(n==0){
					if(mat[n]==-1)fprintf(gcode,"G1 X%.3f Y%.3f E%.3f,F1800\n",x,y,z);// support structure
					else fprintf(gcode,"G1 X%.3f Y%.3f E%.3f,F540.000\n",x,y,z);// solid structure
				}
				else {
					if (mat[n]>=0 && mat[n-1]>=0){
						// change extruder
						if (mat[n]==0){//change to exteruder 1
							;//ADD CODE
							fprintf(gcode,"G1 X%.3f Y%.3f E%.3f\n",x,y,z);// solid structure, different material
						}
						else if(mat[n]==1){//change to exteruder 2
							;//ADD CODE
							fprintf(gcode,"G1 X%.3f Y%.3f E%.3f\n",x,y,z);// solid structure, different material
						}
					}
					else if(mat[n]<0 && mat[n-1]>=0)fprintf(gcode,"G1 X%.3f Y%.3f E%.3f,F1800\n",x,y,z);// support structure
					else if(mat[n]>=0 && mat[n-1]<0) fprintf(gcode,"G1 X%.3f Y%.3f E%.3f,F540.000\n",x,y,z);// solid structure
				}
			}
		}
	}
}
/**********************************************************************/
int MultiMaterialTopopt::findSegmentsInXdir(int seg[21],int mat[20],double y,double z,double Z_end)
{
	double X_start = h_xMin + h_dx/2.0 ;
	double X_end = h_xMin+(h_nelx-1)*h_dx;
	double x;
	int elem,lastMat=-1,currentMat;
	int nseg = 0;
	bool iscurrentS,islastS = false; // for determining support structure

	for (x = X_start;x<=X_end;x+=h_dx){
		elem = findElemOfPoint(x,y,z);
		// check solid(which material) or void
		if (elem == -1) {
			currentMat=-1;
		}
		else if (getElemPseudoDensity(elem)<0.5){
			currentMat = -1;
		}
		else{
			currentMat = getMaterialIdOfElem(elem);
		}
		// check for support structure
		if (currentMat == -1){
			iscurrentS=isSupportStructure(x,y,z,Z_end); // false: if solid or has nothing on +Z dir, true: if (is turned off or does not exist) and has material on its +Z dir  
		}
		else{
			iscurrentS = false;
		}
		// starting point of support structure
		if (currentMat==-1 && iscurrentS && !islastS ){
			seg[nseg] = x;
			mat[nseg] = currentMat;
			nseg++;
		}
		// check for support to solid structure
		else if (currentMat>=0 && lastMat==-1) {
			seg[nseg] = x;
			mat[nseg] = currentMat;
			nseg++;
		}
		// check for solid to solid structure (different materials)
		else if (lastMat>=0 && currentMat>=0 && lastMat != currentMat){
			seg[nseg] = x;
			mat[nseg] = currentMat;
			nseg++;
		}
		lastMat = currentMat;
		islastS = iscurrentS;
	}
	if (lastMat>=0 || islastS){
		seg[nseg] = X_end;
		mat[nseg] = lastMat;
		nseg++;
	}
	return nseg;
}
/**********************************************************************/
bool MultiMaterialTopopt::isSupportStructure(double x,double y,double z,double Z_end)
{
	int elem;
	double r;
	for (r=z;r<=Z_end;r+=h_dz){
		elem = findElemOfPoint(x,y,r);
		if (elem<0) continue;
		if(getElemPseudoDensity(elem)>0.5)
			return true;
	}
	return false;
}
/**********************************************************************/
int MultiMaterialTopopt::findElemOfPoint(double x,double y,double z)
{
	int gridnode,node,elem,codedElem,nodeIndex;
	int Element = -1;
	bool Cx,Cy,Cz;
	int nodes[8];
	// find closest grid node
	int nx = (int)NEARESTINT((x-h_xMin)/h_dx);
	int ny = (int)NEARESTINT((y-h_yMin)/h_dy);
	int nz = (int)NEARESTINT((z-h_zMin)/h_dz);
	gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
	// find mesh node
	if (gridnode<0 || gridnode>=(h_nelx+1)*(h_nely+1)*(h_nelz+1)) return -1;
	node = h_gridNodeToRealNode[gridnode];
	// loop over parenting elems
	for (int neighbor=0;neighbor<h_maxNeighboringElements;neighbor++){
		codedElem = h_neighborElems[h_nNodes*neighbor+node];
		nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
		elem = codedElem - nodeIndex*MAXELEMS;
		if (elem>=h_nElems || elem <0) continue;
		getElemNodes(elem,nodes);
		Cx = (x>=h_x[nodes[0]]) && (x<=h_x[nodes[1]]); 
		Cy = (y>=h_y[nodes[0]]) && (y<=h_y[nodes[2]]); 
		Cz = (z>=h_z[nodes[0]]) && (z<=h_z[nodes[4]]);
		if (!Cx || !Cy || !Cz) continue;
		Element =  elem;
		break;
	}
	//if (Element == -1)
	//	if (mmto_verbose) printf("No Element for given point is found...!\n");
	return Element;
}
/**********************************************************************/
MultiMaterialTopopt::~MultiMaterialTopopt()
{
	safeFree(mmto_rankID);
	safeFree(mmto_currentI);
	safeFree(mmto_smoothCurrentI);
	safeFree(mmto_alternateI);
	safeFree(mmto_currentMaterial);
	safeFree(mmto_isChanged);
	safeFree(mmto_isNeighbor);
	safeFree(mmto_ElemID);
	safeFree(mmto_MaterialID);
	safeFree(mmto_Costs);
	safeFree(mmto_StressCoef); // StressCriterion
	safeFree(mmto_MaterialStrength);
	safeFree(mmto_StressCriterion);
	safeFree(mmto_StressCriterionAtNodes);
	safeFree(mmto_StressCriterionAdjointForce);
	safeFree(mmto_StressCriterionAdjoint);
	safeFree(mmto_principalstresses);
	safeFree(mmto_StructuringElemsXYZ19); // useMorphology
	safeFree(mmto_StructuringElemsXY);
	safeFree(mmto_StructuringElemsXZ);
	safeFree(mmto_StructuringElemsYZ);
	safeFree(mmto_StructuringElemsX);
	safeFree(mmto_StructuringElemsY);
	safeFree(mmto_StructuringElemsZ);
	safeFree(mmto_stElemsXYZ19);
	safeFree(mmto_stElemsXY);
	safeFree(mmto_stElemsXZ);
	safeFree(mmto_stElemsYZ);
	safeFree(mmto_stElemsX);
	safeFree(mmto_stElemsY);
	safeFree(mmto_stElemsZ);
	safeFree(mmto_sortedCost);
	safeFree(mmto_sortedcostId);
}
