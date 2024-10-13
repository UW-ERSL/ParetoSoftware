#include <math.h>
#include <stdlib.h>
#include <string> 
#include <omp.h>
#include "cmatrix.h"
#include "TopOptForAM.h"
#include "Timer.h"

/**********************************************************************/
TopOptForAM::TopOptForAM()
{
	if (TOPOPTFORAM_VERBOSE) printf("Creating TopOptForAM Object \n");

	ts_buildDir[0] = 0.;
	ts_buildDir[1] = 1.;
	ts_buildDir[2] = 0.;

	ts_CutoffAngleDegrees = 135.0;
	ts_desiredSupportVolume = 1.0;
	ts_maximumSupportVolume = 1.0;
	ts_currentSupportVolume = 0.0;
	ts_initialSupportVolume = 0.0;
	ts_FillRatio = 1.0;
	ts_complianceWt = 1.0;
	ts_supportWt = 0.0;
	ts_minSupportHeight = 2*h_dy;

	ts_supportStructure = NULL;
	ts_supportBeams = NULL; 
	ts_OverhangWeight = NULL;
    ts_nSupportBeams = -1;
	ts_minimizeSupportStructures = false;

}
/**********************************************************************/
void TopOptForAM::initializeTopOptForAM(void)
{
	ts_CutoffAngleDegrees = 135.0;
	ts_desiredSupportVolume = 1.0;
	ts_maximumSupportVolume = 1.0;
	ts_currentSupportVolume = 0.0;
	ts_initialSupportVolume = 0.0;
	ts_FillRatio = 1.0;
	ts_complianceWt = 1.0;
	ts_supportWt = 0.0;
	ts_nSupportBeams = -1;
	ts_minSupportHeight = h_dy;

	safeFree(ts_supportStructure);
	safeFree(ts_supportBeams);
	safeFree(ts_OverhangWeight);

	ts_supportStructure = (bool *)malloc(h_nElems*sizeof(bool));
	memset(ts_supportStructure,0,h_nElems*sizeof(bool));

	ts_supportBeams = (double *)malloc(6*h_nElems*sizeof(double));
	memset(ts_supportBeams,-1,6*h_nElems*sizeof(double));

	ts_OverhangWeight = (double *)malloc(h_nElems*sizeof(double));
	memset(ts_OverhangWeight,0,h_nElems*sizeof(double));
}

/**********************************************************************/
bool TopOptForAM::isSupportStructureMinimizationEnabled(void)
{
	return(ts_minimizeSupportStructures);
}
/**********************************************************************/
void TopOptForAM::enableSupportStructureMinimization(void)
{
	ts_minimizeSupportStructures = true;
}
/**********************************************************************/
void TopOptForAM::disableSupportStructureMinimization(void)
{
	ts_minimizeSupportStructures = false;
}
/**********************************************************************/
void TopOptForAM::modifyTSField(void)
{

	if (!ts_trackSupport) 
		return;
	memset(ts_supportBeams,0,6*sizeof(double)*h_nElems);
	ts_nSupportBeams = 0;
	if (ts_minimizeSupportStructures){
		computeSupportStructureTSField();
		return;
	}
	else 
		if (ts_trackSupport){
			findSupportStructureBeams();
			return;
		}
		else
			return;// do nothing
}
///**********************************************************************/
//void TopOptForAM::setisTrackSupport(bool isTrackSupport)
//{
//	ts_trackSupport = isTrackSupport;
//}
///**********************************************************************/
//bool TopOptForAM::isTrackSupport()
//{
//	return ts_trackSupport;
//}
/**********************************************************************/
void TopOptForAM::setBuildDirection(double dir[3])
{
	double nrm = NORM3D(dir);
	if (nrm < 1e-8) {
		ts_buildDir[0] = ts_buildDir[1] = ts_buildDir[2] = 0.0;
		disableSupportStructureMinimization();
		if (ts_supportVerbose) printf("TopOptForAM: No support structure ...\n");
		return;
	}
	for (int i=0;i<3;i++)
		ts_buildDir[i] = dir[i]/nrm;
}
/**********************************************************************/
void TopOptForAM::getBuildDirection(double dir[3])
{
	for (int i=0;i<3;i++)
		dir[i] =  ts_buildDir[i];
}
/**********************************************************************/
void TopOptForAM::setCutoffAngleDegrees(double cutoffAngle)
{
	ts_CutoffAngleDegrees = cutoffAngle;
}
/**********************************************************************/
double TopOptForAM::getCutoffAngleDegrees()
{
	return ts_CutoffAngleDegrees;
}
/**********************************************************************/
void TopOptForAM::setMaximumSupportPercent(double maxPercent)
{
	ts_maximumSupportPercent = maxPercent;
}
/**********************************************************************/
double TopOptForAM::getMaximumSupportPercent(void)
{
	return ts_maximumSupportPercent;
}
/**********************************************************************/
void TopOptForAM::computeSupportStructureTSField(void)
{
	double *TS1,*TS2;
	double maxSupportTS,minSupportTS;
	int nElems;
	double ComplianceRatioAllowed = 1.5;
	double complianceWt,supportStructureWt,temp;
	 

	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return;
	}

	nElems = getNumElems();
	TS1 = (double *)malloc(nElems*sizeof(double));
	TS2 = (double *)malloc(nElems*sizeof(double));

	memset(TS2,0,nElems*sizeof(double));

	memcpy(TS1,ts_topSensElem,nElems*sizeof(double));

	findSupportStructureSensitivity(TS2);

	if (!ts_minimizeSupportStructures) return;

	maxSupportTS = ts_TSMax = -1e8;
	minSupportTS = ts_TSMin = 1e8;
	for (int elem=0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		if (TS1[elem]<ts_TSMin) ts_TSMin = TS1[elem];
		if (TS1[elem]>ts_TSMax) ts_TSMax = TS1[elem];
		if (TS2[elem]<minSupportTS) minSupportTS = TS2[elem];
		if (TS2[elem]>maxSupportTS) maxSupportTS = TS2[elem];
	}
	if (TSSupportNorm0 < 0.0){
		TSSupportNorm0 = maxSupportTS-minSupportTS;
		TSSupportNorm0 = MAX(TSSupportNorm0,1e-10);
		ts_complianceWt = 1.0;
		ts_supportWt = 0.0;
	}
	for (int elem=0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		TS1[elem] = (TS1[elem]-ts_TSMin)/(ts_TSMax - ts_TSMin + 1e-10);
		TS2[elem] = (TS2[elem]-minSupportTS)/(maxSupportTS - minSupportTS + 1e-10);
	}

	double alpha = ts_initialSupportVolume/ts_maximumSupportVolume;
	double beta = ts_currentSupportVolume/ts_maximumSupportVolume; 
	double eta = (ts_desiredSupportVolume-ts_initialSupportVolume)/(ts_maximumSupportVolume-ts_initialSupportVolume);

	supportStructureWt = MIN((beta-alpha)/(eta*(1-alpha)),1.0);
	supportStructureWt = MAX(0.5*(1-eta),supportStructureWt); // minimum weight is 0.25*(1-eta)
	complianceWt = 1.0-supportStructureWt;


	ts_complianceWt = 0.9*ts_complianceWt + 0.1*complianceWt;// slow change
	ts_supportWt = 0.9*ts_supportWt + 0.1*supportStructureWt;// slow change
	temp = ts_complianceWt + ts_supportWt;
	ts_complianceWt /= temp;
	ts_supportWt /= temp;
	if(ts_supportVerbose){
		printf("\n------------------\n");
		printf("supportStructureFrac = %g\n",ts_currentSupportVolume/ts_maximumSupportVolume);
		printf("ts_complianceWt = %g,ts_supportWt = %g\n",ts_complianceWt,ts_supportWt);
	}
	for (int elem = 0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		ts_topSensElem[elem] = ts_complianceWt*TS1[elem] + ts_supportWt*TS2[elem];
	}
	// scale
	ts_TSMax = -1e8;
	ts_TSMin = 1e8;
	for (int elem=0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		if (ts_topSensElem[elem]<ts_TSMin) ts_TSMin = ts_topSensElem[elem];
		if (ts_topSensElem[elem]>ts_TSMax) ts_TSMax = ts_topSensElem[elem];
	}
	for (int elem = 0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)< 0.5){ 
			ts_topSensElem[elem] = 0.;
			continue;
		}
		if (h_keepElem[elem]) {
			ts_topSensElem[elem] = ts_TSMax;
			continue;
		}
		ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin);
	}

	safeFree(TS1);
	safeFree(TS2);
}
/**********************************************************************/
void TopOptForAM::findSupportStructureSensitivity(double *sensitivity)
{
	int nElems;
	double normal[3],dtprd0,cutoffValue; 

	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return;
	}

	findSupportStructureBeams();

	cutoffValue = cos(ts_CutoffAngleDegrees*PI/180.0);
	nElems = getNumElems();
	memset(sensitivity,0,nElems*sizeof(double));
	double alpha = ts_CutoffAngleDegrees*PI/180.0;
	
	double TSin = 2.0*(PI-alpha-sin(alpha)*cos(alpha))/PI;
	// Interior 
	for (int elem=0;elem<nElems;elem++) {
		if (isElemOnEvolvingBoundary(elem)) continue;
		sensitivity[elem] = TSin;
	}
	// Boundary
	for (int elem=0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		if (!isElemOnEvolvingBoundary(elem)) continue;
		if (!computeEvolvingBoundaryNormalAtElement(elem,normal)) continue;
		dtprd0 = 0.0;
		for (int i=0;i<3;i++) {
			dtprd0 += normal[i]*ts_buildDir[i];
		}
		// for all boundary points
		sensitivity[elem] = 0.5*(1-dtprd0);
		// if facing up and is support
		if (!isElemSupportStructure(elem)) continue;
		if (dtprd0 < 0.0) continue;
		sensitivity[elem] = 0.5*(1+dtprd0);
	}
}
/**********************************************************************/
int TopOptForAM::findSupportElemsAlongDirection(int elem,double dir[3])
{
	int currentElem,finalElem;
	double startCenter[3],previousCenter[3],neighborCenter[3],currentCenter[3];
	int *neighboringElems;
	
	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return 0;
	}
	neighboringElems = (int *)malloc(h_StructuringSize*sizeof(int));
	memset(neighboringElems,-1,h_StructuringSize*sizeof(int));
	currentElem = elem;
	getElemCenter(currentElem,startCenter); 
	for (int i=0;i<3;i++) {
		previousCenter[i] = startCenter[i];
	}
	while (1){
        finalElem = currentElem;
		getElemCenter(currentElem,currentCenter); 
		printf("There is a problem here in findSupportElemsAlongDirection\n");
		getchar();
		//findNeighboringElems(currentElem,neighboringElems);issue
		getSupportElemOfElem(neighboringElems);

		if(ts_supportStructuringElem[1]<0) {
			getElemCenter(finalElem,neighborCenter); 
			break;
		}
		if (getElemPseudoDensity(ts_supportStructuringElem[1]) > 0.5){
			finalElem = ts_supportStructuringElem[1];
			break;
		}
		currentElem = ts_supportStructuringElem[1];
	}
	
	safeFree(neighboringElems);

	return finalElem;
}
/**********************************************************************/
double TopOptForAM::findSupportStructureBeams()
{
	int nElems,oppositeElem;
	double normal[3],dtprd0,cutoffValue,oppositeDir[3],vec[3],height,heightVec[3]; 
	double elemCenter[3],oppositeCenter[3],platformPrj[3];
	
	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return -1;
	}
	cutoffValue = cos(ts_CutoffAngleDegrees*PI/180.0);
	nElems = getNumElems();
	memset(ts_supportBeams,0,6*sizeof(double)*nElems);
	memset(ts_supportStructure,0,h_nElems*sizeof(bool));
	ts_nSupportBeams = 0;
	ts_currentSupportVolume = 0.0;
	ts_FillRatio = 0.5;

	findBuildPlatform();
	updateBuildPlatform();

	for (int i=0;i<3;i++)
		oppositeDir[i] = -ts_buildDir[i];

	for (int elem=0;elem<nElems;elem++){
		if (getElemPseudoDensity(elem)<0.5) continue;
		if (!isElemOnEvolvingBoundary(elem)) continue;
		if (!computeEvolvingBoundaryNormalAtElement(elem,normal)) continue;
		// check if overhang
		dtprd0 = 0.0;
		for (int i=0;i<3;i++) {
			dtprd0 += normal[i]*ts_buildDir[i];
		}
		if (dtprd0 > cutoffValue) continue;
		// find opposite elem
		getElemCenter(elem,elemCenter);
		oppositeElem = findSupportElemsAlongDirection(elem,oppositeDir);
		
		if (getElemPseudoDensity(oppositeElem)<0.5 || oppositeElem == elem) {
			// no elem solid found
			for (int i=0;i<3;i++) oppositeCenter[i] = elemCenter[i];
			findSupportPointOnPlatform(elemCenter,platformPrj);
			while (1){
				// move toward platform
				for (int i=0;i<3;i++) oppositeCenter[i] += oppositeDir[i]*h_dy;
				// check if below platform
				SUBTRACT3D(vec,oppositeCenter,platformPrj);;
				dtprd0 = 0.0;
				for (int i=0;i<3;i++) {
					dtprd0 += vec[i]*ts_buildDir[i]/NORM3D(vec);
				}
				if (dtprd0<1e-6){
					for (int i=0;i<3;i++) oppositeCenter[i] = platformPrj[i];
					break;
				}
				// still above platform
				oppositeElem = findElemOfPoint(oppositeCenter[0],oppositeCenter[1],oppositeCenter[2]);
				if (oppositeElem < 0) continue;
				if (getElemPseudoDensity(oppositeElem)<0.5) continue;
				// solid elem found
				getElemCenter(oppositeElem,oppositeCenter);
				// for sensitivity
				ts_supportStructure[elem] = true;
				ts_supportStructure[oppositeElem] = true;
				break;
			}
		}
		else{
			getElemCenter(oppositeElem,oppositeCenter);
			// for sensitivity
			ts_supportStructure[elem] = true;
			ts_supportStructure[oppositeElem] = true;
		}
		SUBTRACT3D(heightVec,oppositeCenter,elemCenter);
		height = NORM3D(heightVec);

		ts_currentSupportVolume += height*h_dx*h_dz*ts_FillRatio;
		// for plotting cylinders
		for (int i=0;i<3;i++){
			ts_supportBeams[6*ts_nSupportBeams+i] = elemCenter[i];
			ts_supportBeams[6*ts_nSupportBeams+i+3] = oppositeCenter[i];
		}
		ts_nSupportBeams++;
	}
	//printf("ts_currentSupportVolume = %g\n",ts_currentSupportVolume);
	
	return ts_currentSupportVolume;
}
/**********************************************************************/
void TopOptForAM::setDesiredSupportVolume(double desiredSupVol)
{
	ts_desiredSupportVolume = desiredSupVol;
}
/**********************************************************************/
double TopOptForAM::getDesiredSupportVolume(void)
{
	return ts_desiredSupportVolume;
}
/**********************************************************************/
void TopOptForAM::setMaximumSupportVolume(double maxSup)
{
	ts_maximumSupportVolume = maxSup;
}
/**********************************************************************/
double TopOptForAM::getMaximumSupportVolume(void)
{
	return ts_maximumSupportVolume;
}
/**********************************************************************/
bool TopOptForAM::isElemSupportStructure(int elem)
{
	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return false;
	}
	if (elem<0 || elem>h_nElems-1) return false;
	if (ts_supportStructure[elem])
		return true;
	else
		return false;
}
/**********************************************************************/
void TopOptForAM::enableSupportVerbose()
{
	ts_supportVerbose = true;
}
/**********************************************************************/
void TopOptForAM::disableSupportVerbose()
{
	ts_supportVerbose = false;
}
/**********************************************************************/
double TopOptForAM::getSupportVolume(void)
{
	return ts_currentSupportVolume;
}
/**********************************************************************/
void TopOptForAM::setInitialSupportVolume(double initialSupportVolFrac)
{
	ts_initialSupportVolume = initialSupportVolFrac;
}
/**********************************************************************/
double TopOptForAM::getInitialSupportVolume()
{
	return ts_initialSupportVolume;
}
/**********************************************************************/
void TopOptForAM::setBuildDirId(int buildDirID)
{
	ts_buildDirId = buildDirID;

	double buildDir[3];
	switch (buildDirID){
	case 0 : buildDir[0] = buildDir[1] = buildDir[2] = 0.0;// none
		break;
	case 1 : buildDir[0] = 1.0; //+X
		buildDir[1] = buildDir[2] = 0.0;
		break;
	case 2 : buildDir[1] = 1.0; //+Y
		buildDir[0] = buildDir[2] = 0.0;
		break;
	case 3 : buildDir[2] = 1.0; //+Z
		buildDir[0] = buildDir[1] = 0.0;
		break;
	case 4 : buildDir[0] = -1.0; //-X
		buildDir[1] = buildDir[2] = 0.0;
		break;
	case 5 : buildDir[1] = -1.0; //-Y
		buildDir[0] = buildDir[2] = 0.0;
		break;
	case 6 : buildDir[2] = -1.0; //-Z
		buildDir[0] = buildDir[1] = 0.0;
		break;
	}
	setBuildDirection(buildDir);
	findBuildPlatform();
}
/**********************************************************************/
int TopOptForAM::getNumSupportBeams()
{
	return ts_nSupportBeams;
}
/**********************************************************************/
void TopOptForAM::getBeamPoints(double start[3],double end[3],int nBeam)
{
	double offset = 0.1;
	for (int i=0;i<3;i++){
		start[i] = ts_supportBeams[6*nBeam+i];
		end[i] = ts_supportBeams[6*nBeam+i+3];
	}
	for (int i=0;i<3;i++){
		start[i] += offset*ts_buildDir[i]*h_dy;
		end[i] += -offset*ts_buildDir[i]*h_dy;
	}
}
/**********************************************************************/
void TopOptForAM::setSupportFillRatio(double fillRatio)
{
	ts_FillRatio = fillRatio;
}
/**********************************************************************/
double TopOptForAM::getSupportFillRatio()
{
	return ts_FillRatio;
}
/**********************************************************************/
void TopOptForAM::getSupportElemOfElem(int *neighbors)
{
	ts_supportStructuringElem[0] = neighbors[0];
	switch (ts_buildDirId)
	{
	case 0:
		break;
	case 1: 
		ts_supportStructuringElem[1] = neighbors[1];// -X
		ts_supportStructuringElem[2] = neighbors[4];//+X
		break;
	case 2: 
		ts_supportStructuringElem[1] = neighbors[2];// -Y
		ts_supportStructuringElem[2] = neighbors[5];//+Y
		break;
	case 3: 
		ts_supportStructuringElem[1] = neighbors[3];// -Z
		ts_supportStructuringElem[2] = neighbors[6];//+Z
		break;
	case 4: 
		ts_supportStructuringElem[1] = neighbors[4];// +X
		ts_supportStructuringElem[2] = neighbors[1];//-X
		break;
	case 5: 
		ts_supportStructuringElem[1] = neighbors[5];// +Y
		ts_supportStructuringElem[2] = neighbors[2];//-Y
		break;
	case 6: 
		ts_supportStructuringElem[1] = neighbors[6];// +Z
		ts_supportStructuringElem[2] = neighbors[3];//-Z
		break;
	}
	
}
/**********************************************************************/
double TopOptForAM::findSupportPointOnPlatform(double start[3],double end[3])
{
	double v[3],vcos[3];
	double dist;

	dist = 0.0;
	v[0] = v[1] = v[2] = 0.0;
	vcos[0] = vcos[1] = vcos[2] = 0.0;
	end[0] = end[1] = end[2] = 0.0;

	SUBTRACT3D(v,start,ts_platformCenter);
	dist = DOTPROD3D(v,ts_buildDir);
	for (int i=0;i<3;i++) 
		vcos[i] = abs(dist)*ts_buildDir[i];

	for (int i=0;i<3;i++)
		end[i] = ts_platformCenter[i]-(vcos[i]-v[i]);

	return (dist);
}
/**********************************************************************/
void TopOptForAM::findBuildPlatform()
{
	double boundingBox[3][2],xyzMin[3],xyzMax[3],normal[3],edge1[3],edge2[3],pt1[3],pt2[3],pt3[3];
	double dtprd;
	double pts[3][8];
	int faces[4][6];

	getBoundingBox(boundingBox);
	for (int i = 0; i < 3; i++){
		xyzMin[i] = boundingBox[i][0];
		xyzMax[i] = boundingBox[i][1];
	}

	pts[0][0] = xyzMin[0]; pts[1][0] = xyzMin[1]; pts[2][0] = xyzMin[2];
	pts[0][1] = xyzMax[0]; pts[1][1] = xyzMin[1]; pts[2][1] = xyzMin[2];
	pts[0][2] = xyzMin[0]; pts[1][2] = xyzMax[1]; pts[2][2] = xyzMin[2];
	pts[0][3] = xyzMax[0]; pts[1][3] = xyzMax[1]; pts[2][3] = xyzMin[2];
	pts[0][4] = xyzMin[0]; pts[1][4] = xyzMin[1]; pts[2][4] = xyzMax[2];
	pts[0][5] = xyzMax[0]; pts[1][5] = xyzMin[1]; pts[2][5] = xyzMax[2];
	pts[0][6] = xyzMin[0]; pts[1][6] = xyzMax[1]; pts[2][6] = xyzMax[2];
	pts[0][7] = xyzMax[0]; pts[1][7] = xyzMax[1]; pts[2][7] = xyzMax[2];
	
	faces[0][0] = 0; faces[1][0] = 1; faces[2][0] = 3; faces[3][0] = 2;
	faces[0][1] = 4; faces[1][1] = 6; faces[2][1] = 7; faces[3][1] = 5;
	faces[0][2] = 0; faces[1][2] = 4; faces[2][2] = 5; faces[3][2] = 1;
	faces[0][3] = 2; faces[1][3] = 3; faces[2][3] = 7; faces[3][3] = 6;
	faces[0][4] = 0; faces[1][4] = 2; faces[2][4] = 6; faces[3][4] = 4;
	faces[0][5] = 1; faces[1][5] = 5; faces[2][5] = 7; faces[3][5] = 3;
	for (int face = 0;face<6;face++){
		for (int i=0;i<3;i++){
			pt1[i] = pts[i][faces[0][face]];
			pt2[i] = pts[i][faces[1][face]];
			pt3[i] = pts[i][faces[3][face]];
		}
		SUBTRACT3D(edge1,pt2,pt1);
		for (int i=0;i<3;i++) edge1[i] /= NORM3D(edge1);
		SUBTRACT3D(edge2,pt3,pt1);
		for (int i=0;i<3;i++) edge2[i] /= NORM3D(edge2);
		CROSS3D(normal,edge1,edge2);
		for (int i=0;i<3;i++) normal[i] /= NORM3D(normal);
		dtprd = DOTPROD3D(normal,ts_buildDir);
		if (dtprd<0.9) continue;
		for(int i=0;i<4;i++){
			ts_platformX[i] = pts[0][faces[i][face]];
			ts_platformY[i] = pts[1][faces[i][face]];
			ts_platformZ[i] = pts[2][faces[i][face]];
		}
		break;
	}

	ts_platformCenter[0] = ts_platformCenter[1] = ts_platformCenter[2] = 0.0; 
	for (int i=0;i<4;i++){
		ts_platformCenter[0] += 0.25*ts_platformX[i];
		ts_platformCenter[1] += 0.25*ts_platformY[i];
		ts_platformCenter[2] += 0.25*ts_platformZ[i];
	}
}
/**********************************************************************/
void TopOptForAM::getBuildPlatform(double x[4],double y[4],double z[4])
{
	for (int i=0;i<4;i++){
		x[i] = ts_platformX[i];
		y[i] = ts_platformY[i];
		z[i] = ts_platformZ[i];
	}
}
/**********************************************************************/
double TopOptForAM::findClosestElemToPlatform(double ClosestPt[3],double TrVec[3])
{
	double dist,minDist,elemCenter[3],dtprd0,cutoffValue;
	double normal[3];
	minDist = 1e8;
	double prj[3];
	cutoffValue = cos(ts_CutoffAngleDegrees*PI/180.0);
	for (int elem=0;elem<h_nElems;elem++)
	{
		if (getElemPseudoDensity(elem)<0.5) continue;
		if (!isElemOnEvolvingBoundary(elem)) continue;
		if (!computeEvolvingBoundaryNormalAtElement(elem,normal)) continue;
		dtprd0 = 0.0;
		for (int i=0;i<3;i++) {
			dtprd0 += normal[i]*ts_buildDir[i];
		}

		if (dtprd0 > cutoffValue) continue;

		getElemCenter(elem,elemCenter);
		dist = findSupportPointOnPlatform(elemCenter,prj);
		if (dist < minDist){
			minDist = dist;
			for (int i=0;i<3;i++) {
				TrVec[i] = elemCenter[i] - prj[i] - 0.5*ts_buildDir[i]*h_dy;
				ClosestPt[i] = elemCenter[i];
			}
		}
	}
	if (minDist<h_dy) {
		TrVec[0] = TrVec[1] = TrVec[2] = 0.0;
		minDist = 0.0;
	}

	return minDist;
}
/**********************************************************************/
void TopOptForAM::updateBuildPlatform()
{
	double elemCenter[3],vec[3];
	double minDist;

	minDist = findClosestElemToPlatform(elemCenter,vec);

	for (int i=0;i<4;i++){
		ts_platformX[i] += vec[0];
		ts_platformY[i]  += vec[1];
		ts_platformZ[i]  += vec[2];
	}
	ts_platformCenter[0] = ts_platformCenter[1] = ts_platformCenter[2] = 0.0; 
	for (int i=0;i<4;i++){
		ts_platformCenter[0] += 0.25*ts_platformX[i];
		ts_platformCenter[1] += 0.25*ts_platformY[i];
		ts_platformCenter[2] += 0.25*ts_platformZ[i];
	}
}
/**********************************************************************/
double TopOptForAM::findWeightOnElem(int elem,double dir[3])
{
	// finds the weight of material above an elem in dir 
	double weight; 
	double center[3];
	int nextElem;
	double rho,volume;
	getElemCenter(elem,center);
	
	volume = getElemVolume(elem);
	rho = getDensityOfElem(elem);
	weight = rho*volume;
	while (1){
		for (int i=0;i<3;i++){
			center[i] += dir[i]*h_dx;
		}
		nextElem = findElemOfCenter(center[0],center[1],center[2]);
		if (nextElem < 0) break;
		if (getElemPseudoDensity(nextElem) < 0.5) break;
		volume = getElemVolume(nextElem);
		rho = getDensityOfElem(nextElem);
		weight += rho*volume; 
	}

	return weight;
}
/**********************************************************************/
double TopOptForAM::findWeightOnAllOverhangElems()
{
	double max = 0.0;
	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return max;
	}
	if (ts_nSupportBeams==0){
		findSupportStructureBeams();
	}
	memset(ts_OverhangWeight,0,h_nElems*sizeof(double));
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem) < 0.5) continue;
		if (!ts_supportStructure[elem]) continue;
		ts_OverhangWeight[elem] = findWeightOnElem(elem,ts_buildDir);
		if (ts_OverhangWeight[elem]>max){
			max = ts_OverhangWeight[elem];
		}
	}
	//normalize
	if (max<1e-10) return max;
	for (int elem=0;elem<h_nElems;elem++){
		if (getElemPseudoDensity(elem) < 0.5) continue;
		if (!ts_supportStructure[elem]) continue;
		ts_OverhangWeight[elem] /= max;
	}
	return max;
}
/**********************************************************************/
void TopOptForAM::getOverhangLoads(double *load)
{
	if (ts_OverhangWeight == NULL) {
		printf("find Loads first ... \n");
		return;
	}
	memcpy(load,ts_OverhangWeight,h_nElems*sizeof(double));
}
/**********************************************************************/
TopOptForAM::~TopOptForAM()
{
	if (TOPOPTFORAM_VERBOSE) printf("Releasing TopOptForAM Object \n");
	if (ts_supportStructure == NULL) {
		printf("Initialize TopOptForAM \n");
		return;
	}
	free(ts_supportStructure);
	free(ts_supportBeams);
	free(ts_OverhangWeight);
}
