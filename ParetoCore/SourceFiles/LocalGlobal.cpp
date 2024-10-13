#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glui.h>
#include <direct.h>
#include "HexMeshGPUDefines.h"
#include "CommonGUI.h"
#include "string.h"
#include "AdditiveManufacturing.h"
#include "MicroStructure.h"
#include "ThermoElasticity.h"
#include "Voxelizer.h"
#include "Timer.h"

/************************************************************/
void findAllElemsInsideFocusbox(double focusBox[3][2],int *vonMisesElems,int start,int end,int *ElemsInsideFocusbox)
{
	double center[3];
	for (int elem=end;elem>start;elem--){
		topopt.getElemCenter(vonMisesElems[elem],center);
		if(center[0]>=focusBox[0][0] && center[0]<=focusBox[0][1])
			if(center[1]>=focusBox[1][0] && center[1]<=focusBox[1][1])
				if(center[2]>=focusBox[2][0] && center[2]<=focusBox[2][1]){
					ElemsInsideFocusbox[elem] = 1;
				}

	}
}
/************************************************************/
int findNextCriticalElem(int start,int end,int *ElemsInsideFocusbox)
{
	int next = 0;
	for (int elem=end;elem>start;--elem)
		if (ElemsInsideFocusbox[elem]==0){
			next = elem;
			break;
		}

	return next;
}
/************************************************************/
void SolveLocalWithFocusBox(double focusBox[3][2],int localProblem,int nElemsLocal)
{
	int BCType[3],voxelElem,nNodes;
	double BCValue[3],pt[3],xietamu[3],vonmises,dMin;
	

	voxLocal.readSTL(liveVar.geomFileName[0],0,0);
	voxLocal.setMaxVoxels(vox.getMaxVoxels());
	voxLocal.resetGroups();
	voxLocal.resetHighlight(); // unselect all highlighted triangles

	voxLocal.setFocusBox(focusBox);

	voxLocal.voxelize(nElemsLocal,liveVar.maxAspectRatio,0,0,liveVar.densityCutoff);
	voxLocal.dumpVoxelData("tempLocal.txt");
	topoptLocal[localProblem].readVoxelFile("tempLocal.txt");
	remove("tempLocal.txt"); 
	topoptLocal[localProblem].createElementMatricesTemplates(liveVar.useBubbleFunctions);
	if (!topoptLocal[localProblem].isMeshConnected()){
		PRINTUTILITY("Error: Mesh is disconnected.\nTry increasing mesh size. \n");
		return;
	}
	nNodes = topoptLocal[localProblem].getNumNodes();
topoptLocal[localProblem].resetBC();
	BCType[0] = ELASTICITY_DIRICHLET;
	BCType[1] = ELASTICITY_DIRICHLET;
	BCType[2] = ELASTICITY_DIRICHLET;
	for (int i = 0; i < nNodes; i++) {

		BCValue[0] = 0.;
		BCValue[1] = 0.;
		BCValue[2] = 0.;

		if (topoptLocal[localProblem].isNodeOnBoundary(i)) {
			topoptLocal[localProblem].getNodeXYZ(i,pt);	
			dMin = vox.closestVoxelElementToPoint(pt[0],pt[1],pt[2],&(voxelElem),xietamu);
			if (voxelElem < 0 ) continue;
			if ( (xietamu[0] < 0) || (xietamu[0] > 1) ) continue;
			if ( (xietamu[1] < 0) || (xietamu[1] > 1) ) continue;
			if ( (xietamu[2] < 0) || (xietamu[2] > 1) ) continue;

			topopt.getDeformationInElemAt(voxelElem,xietamu,pt[0],pt[1],pt[2],
				&(BCValue[0]),&(BCValue[1]),&(BCValue[2]),&vonmises);
			topoptLocal[localProblem].setBCOnSingleNode(i,BCType,BCValue);
		}
	}
	topoptLocal[localProblem].setSpMVMethod(0);
	PRINTUTILITY("Starting Static FEA\n");
	topoptLocal[localProblem].setCGMaxIter(20000);
	topoptLocal[localProblem].setCGRelativeResError(pow(12.0,-1.0*liveVar.cgResidualExponent));

	topoptLocal[localProblem].disableGPU();
	topoptLocal[localProblem].disableDeflation();
	topoptLocal[localProblem].solveLinearSystem();
	topoptLocal[localProblem].printMaxMinUVW();
	topoptLocal[localProblem].postprocess();
}
/************************************************************/
void PushLocalToGlobal(double focusBox[3][2],int localProblem)
{
	int voxelElem;
	double sol[3],pt[3],xietamu[3],vonmises,dMin;
	for (int i = 0; i < topopt.getNumNodes(); i++) {
		topopt.getNodeXYZ(i,pt);	
		if ( (pt[0] < focusBox[0][0]) || (pt[0] > focusBox[0][1]) ) continue;
		if ( (pt[1] < focusBox[1][0]) || (pt[1] > focusBox[1][1]) ) continue;
		if ( (pt[2] < focusBox[2][0]) || (pt[2] > focusBox[2][1]) ) continue;

		dMin = voxLocal.closestVoxelElementToPoint(pt[0],pt[1],pt[2],&(voxelElem),xietamu);

		if (voxelElem < 0 ) continue;
		if ( (xietamu[0] < 0) || (xietamu[0] > 1) ) continue;
		if ( (xietamu[1] < 0) || (xietamu[1] > 1) ) continue;
		if ( (xietamu[2] < 0) || (xietamu[2] > 1) ) continue;
		topoptLocal[localProblem].getDeformationInElemAt(voxelElem,xietamu,pt[0],pt[1],pt[2],
			&(sol[0]),&(sol[1]),&(sol[2]),&vonmises);
		topopt.setSolutionOnNode(i,sol);
	}
}
/************************************************************/
void PushLocalToGlobalWithInterior(double focusBox[3][2],int localProblem,int *InteriorElems,int *vonMisesElems)
{
	int nodes[8],voxelElem,InteriorElem;
	double sol[3],pt[3],xietamu[3],vonmises,dMin;
	for (int elem=0;elem<topopt.getNumElems();elem++){
		if (InteriorElems[elem]==1){
			topopt.getElemNodes(elem,nodes);
			for (int i=0;i<8;i++){
				topopt.getNodeXYZ(nodes[i],pt);
				dMin = voxLocal.closestVoxelElementToPoint(pt[0],pt[1],pt[2],&(voxelElem),xietamu);
				if (voxelElem < 0 ) continue;
				if ( (xietamu[0] < 0) || (xietamu[0] > 1) ) continue;
				if ( (xietamu[1] < 0) || (xietamu[1] > 1) ) continue;
				if ( (xietamu[2] < 0) || (xietamu[2] > 1) ) continue;
				topoptLocal[localProblem].getDeformationInElemAt(voxelElem,xietamu,pt[0],pt[1],pt[2],
					&(sol[0]),&(sol[1]),&(sol[2]),&vonmises);
				topopt.setSolutionOnNode(nodes[i],sol);
			}
		}
	}
}
/************************************************************/
bool IsElemInterior(double focusBox[3][2],int elem)
{
	int ElemNode[8];
	topopt.getElemNodes(elem,ElemNode);
	for (int i=0;i<8;i++)
		if(topopt.isNodeOnBoundary(ElemNode[i])) return false;

	double center[3],voxelSize[3];
	topopt.getElemCenter(elem,center);
	vox.getVoxelSize(voxelSize);
	if ( (center[0] < focusBox[0][0]) || (center[0] > focusBox[0][1]) ) return false;
	if ( (center[1] < focusBox[1][0]) || (center[1] > focusBox[1][1]) ) return false;
	if ( (center[2] < focusBox[2][0]) || (center[2] > focusBox[2][1]) ) return false;
	if (fabs(center[0]-focusBox[0][0])>1.5*voxelSize[0] && fabs(center[0]-focusBox[0][1])>1.5*voxelSize[0]) 
		if (fabs(center[1]-focusBox[1][0])>1.5*voxelSize[1] && fabs(center[1]-focusBox[1][1])>1.5*voxelSize[1]) 
			if (fabs(center[2]-focusBox[2][0])>1.5*voxelSize[2] && fabs(center[2]-focusBox[2][1])>1.5*voxelSize[2])
				return true;

}
/************************************************************/
int findAllInteriorElems(double focusBox[3][2],int *InteriorElems)
{
	int nInterior = 0;
	for (int i=0;i<topopt.getNumElems();i++)
		if (IsElemInterior(focusBox,i)) {
			InteriorElems[i] = 1;
			nInterior++;
		}
		return nInterior;
}
/************************************************************/
void UpdateGlobal(double *GlobalSol,double *KuTilde,double *InitialForce,double *UpdatedForce,int *vonMisesElems,int *InteriorElems)
{
	topopt.getSolutionVector(GlobalSol);
	topopt.KuCPU(GlobalSol,KuTilde);

	int ElemNodes[8],node;
	for (int i=0;i<topopt.getNumElems();i++){
		if(InteriorElems[i]==1){
			topopt.getElemNodes(vonMisesElems[i],ElemNodes);
			for (int j=0;j<8;j++){
				node = ElemNodes[j];
				UpdatedForce[3*node] += KuTilde[3*node] - InitialForce[3*node];
				UpdatedForce[3*node+1] += KuTilde[3*node+1] - InitialForce[3*node+1];
				UpdatedForce[3*node+2] += KuTilde[3*node+2] - InitialForce[3*node+2];

			}
		}
	}
	topopt.setForceVector(UpdatedForce);
	topopt.solveLinearSystem();
}
/************************************************************/
void localDefectCorrection(void)
{
	double focusBox[3][2],eta;
	int nElemsLocal,nElemsGlobal,nNodesGlobal,lastelem,elem,nInteriorElems,ratio,meshScale;
	int localProblem,*vonMisesElems,*ElemsInsideFocusbox,*InteriorElems;
	double *vonMises,center[3],voxelSize[3],*GlobalSol,*KuTilde,*InitialForce,*UpdatedForce;
	if (guiState < STATICFEA_SUCCESFUL) {
		printf("Please solve static problem first \n");
		printf("---------------------------------------------\n");
		return;
	}
	center[0] = 0.;
	center[1] = 0.;
	center[2] = 0.;
	focusBox[0][0] = 0.;focusBox[0][1] = 0.;
	focusBox[1][0] = 0.;focusBox[1][1] = 0.;
	focusBox[2][0] = 0.;focusBox[2][1] = 0.;

	nElemsGlobal = topopt.getNumElems();
	nNodesGlobal = topopt.getNumNodes();

	vonMises = NULL;
	vonMisesElems = NULL;
	ElemsInsideFocusbox = NULL;
	InteriorElems = NULL;
	GlobalSol = NULL;
	KuTilde = NULL;
	InitialForce = NULL;
	UpdatedForce = NULL;

	vonMises = (double *)malloc(nElemsGlobal*sizeof(double));
	vonMisesElems = (int *)malloc(nElemsGlobal*sizeof(int));
	memset(vonMises,0,nElemsGlobal*sizeof(int));
	memset(vonMisesElems,0,nElemsGlobal*sizeof(int));

	for (int i=0;i<nElemsGlobal;i++) vonMisesElems[i] = i;
	
	ElemsInsideFocusbox = (int *)malloc(nElemsGlobal*sizeof(int));
	memset(ElemsInsideFocusbox,0,nElemsGlobal*sizeof(int));
	InteriorElems = (int *)malloc(nElemsGlobal*sizeof(int));
	memset(InteriorElems,0,nElemsGlobal*sizeof(int));
	
	GlobalSol = (double *)malloc(3*nNodesGlobal*sizeof(double));
	KuTilde = (double *)malloc(3*nNodesGlobal*sizeof(double));
	InitialForce = (double *)malloc(3*nNodesGlobal*sizeof(double));
	UpdatedForce = (double *)malloc(3*nNodesGlobal*sizeof(double));

	memset(GlobalSol,0,3*nNodesGlobal*sizeof(double));
	memset(KuTilde,0,3*nNodesGlobal*sizeof(double));
	memset(InitialForce,0,3*nNodesGlobal*sizeof(double));
	memset(UpdatedForce,0,3*nNodesGlobal*sizeof(double));

	topopt.getForceVector(InitialForce);
	topopt.getForceVector(UpdatedForce);

	topopt.getAllElemVonMisesStresses(vonMises);

	topopt.quickSort(vonMises,vonMisesElems,0,nElemsGlobal-1);

	vox.getVoxelSize(voxelSize);
	localProblem = 0;
	meshScale = 4;
	nElemsLocal= meshScale*liveVar.nElemsToMesh;
	eta = 1;
	ratio = 5;
	lastelem = nElemsGlobal-2;
	elem = lastelem;
	while (localProblem < MAX_LOCAL_PROBLEMS){
		if (vonMises[elem]<eta*vonMises[lastelem])	break;
		printf("Solving local problem: %d \n",localProblem);
		topopt.getElemCenter(vonMisesElems[elem],center);

		focusBox[0][0] = center[0] - ratio*voxelSize[0];
		focusBox[0][1] = center[0] + ratio*voxelSize[0];
		focusBox[1][0] = center[1] - ratio*voxelSize[1];
		focusBox[1][1] = center[1] + ratio*voxelSize[1];
		focusBox[2][0] = center[2] - ratio*voxelSize[2];
		focusBox[2][1] = center[2] + ratio*voxelSize[2];

		findAllElemsInsideFocusbox(focusBox,vonMisesElems,0,elem,ElemsInsideFocusbox);
		nInteriorElems = findAllInteriorElems(focusBox,InteriorElems);
		printf("nInteriorElems %i\n",nInteriorElems);

		SolveLocalWithFocusBox(focusBox,localProblem,nElemsLocal);
		//PushLocalToGlobal(focusBox,localProblem);
		PushLocalToGlobalWithInterior(focusBox,localProblem,InteriorElems,vonMisesElems);

		elem = findNextCriticalElem(0,elem,ElemsInsideFocusbox);

		localProblem++;
	}
	printf(" Number of Local Problems solved: %i\n",localProblem);
	
	//UpdateGlobal(GlobalSol,KuTilde,InitialForce,UpdatedForce,vonMisesElems,InteriorElems);

	topopt.postprocess();
	getMaxDeformationAndStressOverMesh(&maxDeformation,&maxStress);
	PRINTUTILITY("Max Displacement: %0.2e \n",maxDeformation); 
	PRINTUTILITY("Max Von Mises: %0.2e \n",maxStress); 

	//Set plotting routine mesh
	guiState = STATICFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_VONMISES;
	setPlotRoutine(1);
	printReactionForces();
	topopt.setForceVector(InitialForce);
	PRINTUTILITY("---------------------------------------------\n");
	free(vonMises);
	free(vonMisesElems);
	free(ElemsInsideFocusbox);
	free(InteriorElems);
	free(GlobalSol);
	free(KuTilde);
	free(InitialForce);
	free(UpdatedForce);
}