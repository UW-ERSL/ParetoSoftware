#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> 
#include <iostream> 
#ifdef _WIN32
#include <Windows.h>
#endif
#ifndef __APPLE__
#include <omp.h>
#endif
#include "STLGeom.h"
#include "OBJ_Loader.h"
#include "queue.h"
#include <Eigen/Dense>
#include <vector>

using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)

void findTriNeighborsGPU(int stl_nTriangles,int *triNodes,int *triNeighbors);

extern void writeMessageToGUIWindow(const char format[],...);
FILE *STL_LOGFILEPTR = NULL;

/****************************************************************************/
void writeMessageToSTLLog(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);
    cout << tempStr << endl;

    if (STL_LOGFILEPTR == NULL)return;

    fprintf(STL_LOGFILEPTR,"%s \n", tempStr);
    fflush(STL_LOGFILEPTR);
}
/**********************************************************************/
STLGeom::STLGeom()
{
	stl_verbose = false;

	stl_ntriPoints = 0;
	stl_nTriangles = 0;
	stl_nTriGroups = 1;
	stl_nConcaveEdges = 0;
	stl_modelScale = 1.0;
	stl_nSharpCorners = 0;
	stl_nSharpEdges = 0;
	stl_nParts = 0;
	stl_symmetry[0] = false;
	stl_symmetry[1] = false;
	stl_symmetry[2] = false;
    stl_segmentModel = true;
	stl_smallestCylinderRadius = HUGE_VAL;

	memset(triPart,-1,MAX_TRIANGLES*sizeof(int));

	for (int i=0; i < 3;i++)
		stl_STLGeomCenter[i] = 0;
}
/**********************************************************************/
void STLGeom::enableSTLVerbose(void)
{
    writeMessageToSTLLog("STLGeom: Enabling STL Verbose ");
	stl_verbose = true;
}
/**********************************************************************/
void STLGeom::disableSTLVerbose(void)
{
    writeMessageToSTLLog("STLGeom: Disabling STL Verbose ");
	stl_verbose = false;
}
/**********************************************************************/
bool STLGeom::createBoundingBoxSTLFile(double boundingBox[3][2],char *fileName)
{
	/*
solid Beam
   facet normal 0.000000e+000 -1.000000e+000 -3.491483e-015
      outer loop
         vertex 3.000000e-001 3.491483e-017 0.000000e+000
         vertex 3.000000e-001 0.000000e+000 1.000000e-002
         vertex 0.000000e+000 3.491483e-017 0.000000e+000
      endloop
   endfacet
   */
	
	double x[] = {boundingBox[0][0], boundingBox[0][1], boundingBox[0][1], boundingBox[0][0], boundingBox[0][0],boundingBox[0][1], boundingBox[0][1], boundingBox[0][0]};
	double y[] = {boundingBox[1][0], boundingBox[1][0], boundingBox[1][1], boundingBox[1][1], boundingBox[1][0], boundingBox[1][0], boundingBox[1][1], boundingBox[1][1]};
	double z[] = {boundingBox[2][0], boundingBox[2][0], boundingBox[2][0], boundingBox[2][0], boundingBox[2][1], boundingBox[2][1], boundingBox[2][1], boundingBox[2][1]};
	int *nodes;
	double v1[3],v2[3],v3[3],nrm;
	int tri[12][3] =  {{0, 3, 1}, {1, 3, 2}, {0, 5, 4}, {0, 1, 5}, {1, 6, 5}, {1, 2, 6}, {2, 7, 6},{ 2, 3, 7}, {3, 0, 4}, {3, 4, 7}, {4, 5, 7},{ 5, 6, 7}};
	FILE *stlFptr = fopen(fileName,"w");

	if (stlFptr == NULL) return false;
    writeMessageToSTLLog("STLGeom: Creating STL file ");
	fprintf(stlFptr,"solid part\n");
	for (int i = 0; i < 12; i++) {
		nodes = tri[i];
		v1[0] = x[nodes[1]] - x[nodes[0]];
		v1[1] = y[nodes[1]] - y[nodes[0]];
		v1[2] = z[nodes[1]] - z[nodes[0]];

		v2[0] = x[nodes[2]] - x[nodes[0]];
		v2[1] = y[nodes[2]] - y[nodes[0]];
		v2[2] = z[nodes[2]] - z[nodes[0]];

		CROSS3D(v3,v1,v2);
		nrm = NORM3D(v3);
		for (int j = 0; j < 3;j++)
			v3[j] = v3[j]/nrm;

		fprintf(stlFptr,"\tfacet normal %f %f %f \n",v3[0],v3[1],v3[2]);
		fprintf(stlFptr,"\t\touter loop \n");

		for (int j = 0; j < 3; j++)
			fprintf(stlFptr,"\t\t\t vertex %f %f %f\n",x[nodes[j]],y[nodes[j]],z[nodes[j]]);
		
		fprintf(stlFptr,"\t\tendloop \n");
		fprintf(stlFptr,"\tendfacet \n");
	}
	fprintf(stlFptr,"endsolid \n");
	fclose(stlFptr);
	return true;
}
/**********************************************************************/
void STLGeom::deleteCurrentModel(void)
{

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Deleting current model ");

	resetGroups();
	stl_ntriPoints = 0;
	stl_nTriangles = 0;
	stl_nTriGroups = 1;
	stl_modelScale = 1.0;
	stl_nSharpCorners = 0;
	stl_nSharpEdges = 0;
	stl_smallestCylinderRadius = HUGE_VAL;
	for (int i=0; i < 3;i++)
		stl_STLGeomCenter[i] = 0;
	
	stl_nParts = 0;

	memset(triPart,-1,MAX_TRIANGLES*sizeof(int));
}
/**********************************************************************/
int STLGeom::getNumParts()
{
	return (stl_nParts);
}
/**********************************************************************/
int STLGeom::getNumTriPoints()
{
	return stl_ntriPoints;
}
/**********************************************************************/
int STLGeom::getNumTriangles()
{
	return stl_nTriangles;
}
/**********************************************************************/
void STLGeom::getNeighboringTrianglesOfTriangle(int tri,int neighTri[3])
{
    for (int i = 0; i < 3; i++)
        neighTri[i] = triNeighbors[3*tri+i];
}
/**********************************************************************/
double	STLGeom::getZHeightOfSTLGeom(void)
{
	return stl_XYZMax[2]-stl_XYZMin[2];
}
/**********************************************************************/
void STLGeom::getTriNode(int node,double pt[3])
{
	if ( (node < 0) || (node >= stl_ntriPoints)) return;
	pt[0] = triXYZ[3*node];
    pt[1] = triXYZ[3*node+1];
	pt[2] = triXYZ[3*node+2];
	
}
/**********************************************************************/
void STLGeom::getTriNodes(int tri,int nodes[3])
{
	int i;
	for (i =0; i <3; i++) 
		nodes[i] = triNodes[3*tri+i];
}
/**********************************************************************/
void STLGeom::getTriCenter(int tri,double center[3])
{
    /* //old method ... not sure where the bug is???
	int i,trinode;

    center[0] = 0.0;
    center[1] = 0.0;
    center[2] = 0.0;
	for (i =0; i <3; i++) {
		trinode = triNodes[3*tri+i];
        center[0] += triXYZ[3*trinode];
        center[1] += triXYZ[3*trinode+1];
        center[2] += triXYZ[3*trinode+2];
	}
    for (i =0; i <3; i++)
        center[i] /= 3.0;
    */
    double x[3],y[3],z[3];
    getTriPts( tri,x,y,z);
    center[0] = (x[0] + x[1] + x[2])/3.0;
    center[1] = (y[0] + y[1] + y[2])/3.0;
    center[2] = (z[0] + z[1] + z[2])/3.0;
}
/**********************************************************************/
void STLGeom::getTriPts(int tri,double x[3],double y[3],double z[3])
{
	int i,trinode;
	for (i =0; i <3; i++) {
		trinode = triNodes[3*tri+i];
		x[i] = triXYZ[3*trinode];
        y[i] = triXYZ[3*trinode+1];
		z[i] = triXYZ[3*trinode+2];
	}
}
/**********************************************************************/
void STLGeom::getPtOnTriangleAt(int tri,double baryCentricCoords[3],double pt[3])
{
    int i,trinode;
    double x[3],y[3],z[3];

    pt[0] =0;pt[1] =0;pt[2] =0;
    if (fabs(SUM3D(baryCentricCoords) - 1) > 1e-6) return;
    for (i =0; i <3; i++) {
        trinode = triNodes[3*tri+i];
        x[i] = triXYZ[3*trinode];
        y[i] = triXYZ[3*trinode+1];
        z[i] = triXYZ[3*trinode+2];
    }



    for (i =0; i <3; i++) {
        pt[0] += baryCentricCoords[i]*x[i];
        pt[1] += baryCentricCoords[i]*y[i];
        pt[2] += baryCentricCoords[i]*z[i];
    }
}
/**********************************************************************/
void STLGeom::getRandomPtInTri(int tri,double pt[3])
{
	int i,trinode;
	double x[3],y[3],z[3],wts[3],temp;

	for (i =0; i <3; i++) {
		trinode = triNodes[3*tri+i];
		x[i] = triXYZ[3*trinode];
        y[i] = triXYZ[3*trinode+1];
		z[i] = triXYZ[3*trinode+2];
	}

	wts[0] = 1.0*rand()/RAND_MAX;
	wts[1] = 1.0*rand()/RAND_MAX;
	wts[2] = 1.0*rand()/RAND_MAX;
	temp = wts[0] +wts[1] +wts[2] ;

	wts[0] /= temp;
	wts[1] /= temp;
	wts[2] /= temp;

	pt[0] =0;pt[1] =0;pt[2] =0;
	for (i =0; i <3; i++) {
		pt[0] += wts[i]*x[i];
		pt[1] += wts[i]*y[i];
		pt[2] += wts[i]*z[i];
	}
}
/**********************************************************************/
void	STLGeom::getMinAndMaxLengths(double *LMin,double *LMax)
{
	*LMin = HUGE_VAL;
	*LMax = -HUGE_VAL;
	for (int i = 0; i < 3; i++) {
		*LMin = MIN(*LMin,stl_XYZMax[i]- stl_XYZMin[i]);
		*LMax = MAX(*LMax,stl_XYZMax[i]- stl_XYZMin[i]);
	}
}
/**********************************************************************/
void	STLGeom::getBoundingBox(double xyzMin[3],double xyzMax[3])
{
	for (int i = 0; i < 3; i++) {
		xyzMin[i] = stl_XYZMin[i];
		xyzMax[i] = stl_XYZMax[i];
	}
}
/**********************************************************************/
void	STLGeom::getBoundingBox(double boundingBox[3][2])
{
	for (int i = 0; i < 3; i++) {
		boundingBox[i][0] = stl_XYZMin[i];
		boundingBox[i][1] = stl_XYZMax[i];
	}
}
/**********************************************************************/
void	STLGeom::incrementTriGroups(void)
{
	 stl_nTriGroups++;
	 if (stl_nTriGroups >= STL_MAX_GROUPS)
        writeMessageToSTLLog("STLGeom: number of triangle groups exceeds allowable MAX of %d ",STL_MAX_GROUPS);
	 stl_nTriGroups = MIN(stl_nTriGroups,STL_MAX_GROUPS-1);
}
/**********************************************************************/
void	STLGeom::decrementTriGroups(void)
{
	 stl_nTriGroups--;
	 stl_nTriGroups = MAX(stl_nTriGroups,0);
}
/**********************************************************************/
void	STLGeom::setNumTriGroups(int nGroups)
{
	stl_nTriGroups = MAX(nGroups,0);
}
/**********************************************************************/
void STLGeom::setModelScale(double scale)
{
    stl_modelScale = scale;
}
/**********************************************************************/
int	STLGeom::getNumTriGroups(void)
{
	return stl_nTriGroups;
}
/**********************************************************************/
int STLGeom::getTriGroup(int tri)
{
	if (tri > stl_nTriangles-1) return 0;
	return triGroup[tri];
}
/**********************************************************************/
double STLGeom::getModelScale(void)
{
	return stl_modelScale;
}
/**********************************************************************/
int STLGeom::find_word(FILE *filein, const char *word)
{
	char str[256];
	fpos_t pos;
	fgetpos(filein, &pos); 
	while(fscanf(filein, " %s", str) !=EOF) {
		if (strcmp(str,word) == 0)
			return 1;
	}
	// failed, return back to original position
	fsetpos(filein,&pos);
	return 0;
}
/**********************************************************************/
double STLGeom::distanceOfPointFromLine3D(double *Pt,double *Pt1,double *Pt2,double *PtClosest)
{

	// Let the nearest point on the triangle be Pt0
	// Pt0 = Pt1*t + Pt2*(1-t)
	// Pt0 = Pt2 + t(Pt1-Pt2)
	// Vector from Pt to Pt0 is
	// Pt2-Pt + t(Pt1-Pt2)
	// Distance^2 from Pt0 to Pt is
	// distsqr = (Pt2-Pt + t(Pt1-Pt2)).(Pt2-Pt + t(Pt1-Pt2))
	// Setting the derivative with respect to  t to zero, we have
	// (Pt2-Pt + t(Pt1-Pt2)).(Pt1-Pt2) = 0
	// t = -(Pt2-Pt).(Pt1-Pt2) /(Pt1-Pt2)).(Pt1-Pt2)
	// Foce t to lie between 0 and 1


	double pt1_pt2[3], pt2_pt[3], t;
	double a,b, distance;
	int i;
	for ( i=0; i < 3; i++) {
		pt1_pt2[i] = Pt1[i] - Pt2[i];
		pt2_pt[i] = Pt2[i] - Pt[i];
	}

	a = DOTPROD3D(pt1_pt2,pt2_pt);
	b = DOTPROD3D(pt1_pt2,pt1_pt2);
	

	t = -a/b;
	t = MIN(1.,MAX(0,t));
	for ( i=0; i < 3; i++) 
		PtClosest[i] = Pt1[i]*t + Pt2[i]*(1-t);

	distance = DIST3D(Pt,PtClosest);
	return distance;

}
/**********************************************************************/
double STLGeom::distanceOfPointToTri(double *Pt,int tri)
{
    double pt0[3],pt1[3],pt2[3],ptClosest[3],stu[3],dist;
	int trinodes[3];

	for (int i =0; i <3; i++) 
		trinodes[i] = triNodes[3*tri+i];

	for (int i =0; i <3; i++) {
		pt0[i] = triXYZ[3*trinodes[0]+i];
		pt1[i] = triXYZ[3*trinodes[1]+i];
		pt2[i] = triXYZ[3*trinodes[2]+i];

	}
	dist = distanceOfPointFromTriangle3D(Pt,pt0,pt1,pt2,ptClosest, stu);
	return dist;
}
/**********************************************************************/
double STLGeom::distanceOfPointFromTriangle3D(double *Pt,double *Pt1,double *Pt2,
												double *Pt3, double *Pt0, double *stu)
{

	// Let the nearest point on the triangle be Pt0
	// Using barycentric coordinates
	// Pt0 = Pt1*s + Pt2*t + Pt3*(1-s-t)
	// Pt0 = s(Pt1-Pt3)+t(Pt2-Pt3) + Pt3
	// Distance^2 from Pt0 to Pt is
	// distsqr = (s(Pt1-Pt3)+t(Pt2-Pt3)+Pt3-Pt).(s(Pt1-Pt3)+t(Pt2-Pt3)+Pt3-Pt)
	// Setting the derivative with respect to s and t to zero, we have
	// (Pt1-Pt3).(s(Pt1-Pt3)+t(Pt2-Pt3)+Pt3-Pt)=0
	// (Pt2-Pt3).(s(Pt1-Pt3)+t(Pt2-Pt3)+Pt3-Pt)=0
	// Thus we solve a 2x2 system for s and t
	// However, the resulting Pt may be invalid in that it might lie
	// outside the domain. Depending on the values of the barycentric coordinates
	// one can resolve 7 different cases and find the closest boundary
	// point on the triangle

	double pt1_pt3[3], pt2_pt3[3], pt3_pt[3], s, t, u;
	double a11,a12,a21,a22,f1,f2,det, distance;
	int i;
	for ( i=0; i < 3; i++) {
		pt1_pt3[i] = Pt1[i] - Pt3[i];
		pt2_pt3[i] = Pt2[i] - Pt3[i];
		pt3_pt[i] = Pt3[i] - Pt[i];
	}
	a11 = DOTPROD3D(pt1_pt3,pt1_pt3);
	a12 = DOTPROD3D(pt1_pt3,pt2_pt3);
	a21 = DOTPROD3D(pt2_pt3,pt1_pt3);
	a22 = DOTPROD3D(pt2_pt3,pt2_pt3);

	det = a11*a22 - a12*a21;

	if (fabs(det)== 0) {
        if (stl_verbose)writeMessageToSTLLog("STLGeom: Error in distanceOfPointFromTriangle3D ... invalid input");
		return HUGE_VAL;
	}

	f1 = - DOTPROD3D(pt1_pt3,pt3_pt);
	f2 = - DOTPROD3D(pt2_pt3,pt3_pt);

	s = (a22*f1-a12*f2)/det;
	t = (-a21*f1+a11*f2)/det;
	u = (1-s-t);

	double s1, t1,u1;
	if ((s>=0)&&(t>=0)&&(u>=0)) 
		{;}// nothing to do, point closest to interior of triangle
	else if ((s>=0)&&(t<0)&&(u<0))
		{ s = 1; t = 0; u =0; }// point is closest to Pt1
	else if ((s<0)&&(t>=0)&&(u<0)) 
		{ s = 0; t = 1; u =0; }// point is closest to Pt2
	else if ((s<0)&&(t<0)&&(u>=0)) 
		{s = 0; t = 0; u =1;}// point is closest to Pt1
	else if ((s>=0)&&(t>=0)&&(u<0)) 
		{s1 = s/(s+t); t1 = t/(s+t); s = s1; t = t1; u =0;}// point is closest to line Pt1-Pt2
	else if ((s<0)&&(t>=0)&&(u>=0)) 
		{s = 0; t1 = t/(u+t); u1 = u/(u+t); t = t1; u = u1;}// point is closest to line Pt2-Pt3
	else if ((s>=0)&&(t<0)&&(u>=0)) 
		{s1 = s/(s+u); t = 0; u1 =u/(s+u); s= s1; u = u1;}// point is closest to line Pt3-Pt1
	else {
		;//printf("STLGeom: ERROR; Cannot find distance from point to Triangle");
	}
	for (i=0; i < 3; i++)
		Pt0[i] = s*Pt1[i] + t*Pt2[i] + u*Pt3[i];

	distance = DIST3D(Pt0,Pt);
	stu[0] = s;
	stu[1] = t;
	stu[2] = u;

	return (distance);

}
/**********************************************************************/
double STLGeom::getAreaOfTriangle(double pt1[3],double pt2[3],double pt3[3])
{
	double x[3],y[3],z[3];
	
	x[0] = pt1[0]; x[1] = pt2[0]; x[2] = pt3[0];
	y[0] = pt1[1]; y[1] = pt2[1]; y[2] = pt3[1];
	z[0] = pt1[2]; z[1] = pt2[2]; z[2] = pt3[2];

	return (computeAreaOfTriangle( x,y,z));
}
/**********************************************************************/
double STLGeom::computeAreaOfTriangle(double x[3],double y[3],double z[3])
{
	// Compute the area of a single triangle on GPU or CPU (common code)
	// It uses the accurate version (avoid round off errors)
	double asqr,bsqr,csqr,A,B,C,temp,area;
	asqr = ( (x[1]-x[0])*(x[1]-x[0]) + (y[1]-y[0])*(y[1]-y[0]) + (z[1]-z[0])*(z[1]-z[0]));
	bsqr = ( (x[2]-x[1])*(x[2]-x[1]) + (y[2]-y[1])*(y[2]-y[1]) + (z[2]-z[1])*(z[2]-z[1]));
	csqr = ( (x[0]-x[2])*(x[0]-x[2]) + (y[0]-y[2])*(y[0]-y[2]) + (z[0]-z[2])*(z[0]-z[2]));

	// sort the 3 numbers
	if ((asqr > bsqr) && (bsqr > csqr)) {
		A = sqrt(asqr); B = sqrt(bsqr);C = sqrt(csqr);
	}
	else if ((asqr > csqr) && (csqr > bsqr)) {
		A = sqrt(asqr); B = sqrt(csqr);C = sqrt(bsqr);
	}
	else if ((bsqr > asqr) && (asqr > csqr)) {
		A = sqrt(bsqr); B = sqrt(asqr);C = sqrt(csqr);
	}
	else if ((bsqr > csqr) && (csqr > asqr)) {
		A = sqrt(bsqr); B = sqrt(csqr);C = sqrt(asqr);
	}
	else if ((csqr > asqr) && (asqr > bsqr)) {
		A = sqrt(csqr); B = sqrt(asqr);C = sqrt(bsqr);
	}
	else {
		A = sqrt(csqr); B = sqrt(bsqr);C = sqrt(asqr);
	}

	temp = (A+(B+C))*(C*C-(A-B)*(A-B))*(A+(B-C));
	area = (double) 0.25*sqrt(temp);
	area = MAX(area,10*TOL);
	return area;
}
/**********************************************************************/
void STLGeom::getTriangleNormal(double x[3],double y[3],double z[3],double normal[3])
{
	double v1[3], v2[3],norm;

	v1[0] = x[1]-x[0];
	v1[1] = y[1]-y[0];
	v1[2] = z[1]-z[0];

	v2[0] = x[2]-x[0];
	v2[1] = y[2]-y[0];
	v2[2] = z[2]-z[0];

	normal[0] = v1[1]*v2[2]-v2[1]*v1[2];
	normal[1] = -(v1[0]*v2[2]-v2[0]*v1[2]);
	normal[2] = v1[0]*v2[1]-v2[0]*v1[1];

	norm = sqrt(normal[0]*normal[0] +normal[1]*normal[1]+normal[2]*normal[2]);

	norm = MAX(norm,TOL);
	for (int i = 0; i < 3; i++)
		normal[i] /=norm;
}
/**********************************************************************/
double STLGeom::computeTriGroupArea(int group)
{
	double area = 0;
	for (int tri = 0; tri < stl_nTriangles; tri++) 
		if (triGroup[tri] == group)
			area += getAreaOfTriangle(tri);
	
	return area;
}
/**********************************************************************/
double STLGeom::getAreaOfTriangle(int tri)
{
	double x[3],y[3],z[3],area;
	getTriPts(tri,x,y,z);
	area = computeAreaOfTriangle(x,y,z);
	if (fabs(area) < TOL)
        writeMessageToSTLLog("STLGeom:  Warning, zero area triangle detected");

	area = MAX(area,10*TOL);
	return area;
}
/**********************************************************************/
void STLGeom::getCenterOfTriangle(int tri,double center[3])
{
	double x[3],y[3],z[3];
	getTriPts(tri,x,y,z);
	center[0] = (x[0]+x[1]+x[2])/3.0;
	center[1] = (y[0]+y[1]+y[2])/3.0;
	center[2] = (z[0]+z[1]+z[2])/3.0;
}
/**********************************************************************/
bool STLGeom::getNormalAtPoint(double pt[3],double normal[3])
// Assumption: pt is very close/on the surface
{
    double dist,triNormal[3],nrm,triNormalPrev[3];
    vector<int> triangles;

    triangles.clear();
    for (int tri = 0; tri < stl_nTriangles; tri++){
        dist = distanceOfPointToTri(pt,tri);
        if (dist < TOL*stl_modelScale){
            triangles.push_back(tri);
        }
    }
    if (triangles.size() == 0)
        return false;

    // only consider triangles with unique normals
    // this avoid skewed normals at vertices

    for (int j =0; j < 3;j++)
        normal[j] = 0;
    bool repeat;

    for (int i = 0; i < triangles.size();i++){
        getNormalOfTriangle(triangles[i],triNormal);
        repeat = false;
        for (int k = 0; k < i; k++){
            getNormalOfTriangle(triangles[k],triNormalPrev);
            if (DOTPROD3D(triNormal,triNormalPrev) > 0.999){
                repeat = true;
                break;
            }
        }
        if (repeat) continue;
        for (int j =0; j < 3;j++)
            normal[j] += triNormal[j];
    }
    nrm = NORM3D(normal);
    for (int j =0; j < 3;j++)
        normal[j] /= nrm;

    return true;
}
/**********************************************************************/
void STLGeom::getNormalOfTriangle(int tri,double normal[3])
{
	double x[3],y[3],z[3];
	getTriPts(tri,x,y,z);
	getTriangleNormal(x,y,z,normal);
}
/**********************************************************************/
void STLGeom::getActiveGroups(int activeGroups[STL_MAX_GROUPS])
{
	for (int g = 0; g < STL_MAX_GROUPS; g++)
		activeGroups[g] = 0;

	for (int tri = 0; tri < stl_nTriangles; tri++) 
		activeGroups[triGroup[tri]] = 1;
}
/**********************************************************************/
void STLGeom::resetGroups(void)
{
	for (int tri = 0; tri < stl_nTriangles; tri++) 
		triGroup[tri] = 0;
}
/**********************************************************************/
void STLGeom::resetHighlight(void)
{
	for (int tri = 0; tri < stl_nTriangles; tri++) 
		triHighlight[tri] = false;
}
/**********************************************************************/
bool  STLGeom::findCanonicalDirectionOfGroup(int group, Point3d &direction)
{
    // Find various directions associated with a group of triangles: normal to a plane, cylidner axis, etc
    // This is used for AM build direction
    double x[3], y[3], z[3], normal[3], nrm, dtprd, meanXYZ[3], vec[3], radius, radiusAvg, normal0[3], center[3], triArea, tempVec[3];
    bool isPlanar, isCylinder;
    int i, nTriSelected;
    double	cylinderRadius, area, averageNormal[3], cylinderAxis[3], axisPoint[3],totalArea;

    area = 0;
    totalArea = getAreaOfSTLPart(0);
    for (i = 0; i < 3; i++) {
        averageNormal[i] = 0.0;//used for pressure loading
        cylinderAxis[i] = 0.0;//used for cylinder surfaces
        meanXYZ[i] = 0;//used for cylinder surfaces
        normal0[i] = 0;
    }
    isPlanar = true;
    isCylinder = false;
    nTriSelected = 0;
    radiusAvg = 0.0;

    for (int tri = 0; tri < stl_nTriangles; tri++) {
        if (triGroup[tri] == group) {
            getTriCenter(tri, center);
            getTriPts(tri, x, y, z);
            triArea = computeAreaOfTriangle(x, y, z);

            meanXYZ[0] += triArea*center[0];//area weighted center
            meanXYZ[1] += triArea*center[1];
            meanXYZ[2] += triArea*center[2];

            area += triArea;
            getTriangleNormal(x, y, z, normal);
            dtprd = DOTPROD3D(averageNormal, normal);
            averageNormal[0] += normal[0];
            averageNormal[1] += normal[1];
            averageNormal[2] += normal[2];
            // for planes
            dtprd = DOTPROD3D(averageNormal, normal) / NORM3D(averageNormal);
            if (dtprd < 0.99) isPlanar = false;
            // for cylinders
            if (nTriSelected == 0) {// the first triangle
                normal0[0] = normal[0];
                normal0[1] = normal[1];
                normal0[2] = normal[2];
            }
            else {
                // take cross-product
                tempVec[0] = (normal0[1] * normal[2] - normal0[2] * normal[1]);
                tempVec[1] = (normal0[2] * normal[0] - normal0[0] * normal[2]);
                tempVec[2] = (normal0[0] * normal[1] - normal0[1] * normal[0]);
                nrm = NORM3D(tempVec);

                if ((tempVec[0] + tempVec[1] + tempVec[2]) > 0) {// avoid cancellations with opposing vectors
                    cylinderAxis[0] += tempVec[0] / nrm;
                    cylinderAxis[1] += tempVec[1] / nrm;
                    cylinderAxis[2] += tempVec[2] / nrm;
                }
            }
            nTriSelected++;
        }
    }

    if (nTriSelected > 0) {//at least one triangle selected
        averageNormal[0] /= nTriSelected;
        averageNormal[1] /= nTriSelected;
        averageNormal[2] /= nTriSelected;
    }
    if ((isPlanar) && (area > 0.01*totalArea)){
        nrm = NORM3D(averageNormal);
        direction.x = averageNormal[0]/nrm;
        direction.y = averageNormal[1]/nrm;
        direction.z = averageNormal[2]/nrm;
        return true;

    }
    meanXYZ[0] /= (area);
    meanXYZ[1] /= (area);
    meanXYZ[2] /= (area);
    axisPoint[0] = meanXYZ[0];
    axisPoint[1] = meanXYZ[1];
    axisPoint[2] = meanXYZ[2];
    if ((nTriSelected > 0) && (!isPlanar) && (NORM3D(averageNormal) < 0.02)) {//could be cylinder
        nrm = NORM3D(cylinderAxis);
        cylinderAxis[0] /= nrm;
        cylinderAxis[1] /= nrm;
        cylinderAxis[2] /= nrm;

        isCylinder = true;
        for (int tri = 0; tri < stl_nTriangles; tri++) {
            if (triGroup[tri] == group) {
                getTriPts(tri, x, y, z);
                vec[0] = x[0] - meanXYZ[0];// consider the vector from point on axis to (x,y,z)
                vec[1] = y[0] - meanXYZ[1];
                vec[2] = z[0] - meanXYZ[2];
                // project the vector onto the axis
                dtprd = vec[0] * cylinderAxis[0] + vec[1] * cylinderAxis[1] + vec[2] * cylinderAxis[2];
                // create the perpendicular vector
                vec[0] -= dtprd*cylinderAxis[0];
                vec[1] -= dtprd*cylinderAxis[1];
                vec[2] -= dtprd*cylinderAxis[2];
                radius = NORM3D(vec);

                if (radiusAvg == 0)
                    cylinderRadius = radius;
                else
                    cylinderRadius = (radiusAvg + radius) / 2;
                if ((fabs(cylinderRadius - radius) / (cylinderRadius)) > 0.05) {
                    isCylinder = false;
                    break;
                }
            }
        }
    }


    if (isCylinder) {
        nrm = NORM3D(cylinderAxis);
        direction.x = cylinderAxis[0]/nrm;
        direction.y = cylinderAxis[1]/nrm;
        direction.z = cylinderAxis[2]/nrm;
        return true;
    }
    return false;
}
/**********************************************************************/
void  STLGeom::computeSmallestRadius(int group)
{
	double x[3], y[3], z[3], normal[3], nrm, dtprd, meanXYZ[3], vec[3], radius, radiusAvg, normal0[3], center[3], triArea, tempVec[3];
	bool isPlanar, isCylinder;
	int i, nTriSelected;
	double	cylinderRadius, area, averageNormal[3], cylinderAxis[3], axisPoint[3];

	area = 0;
	
	for (i = 0; i < 3; i++) {
		averageNormal[i] = 0.0;//used for pressure loading
		cylinderAxis[i] = 0.0;//used for cylinder surfaces
		meanXYZ[i] = 0;//used for cylinder surfaces
		normal0[i] = 0;
	}
	isPlanar = true;
	isCylinder = false;
	nTriSelected = 0;
	radiusAvg = 0.0;

	for (int tri = 0; tri < stl_nTriangles; tri++) {
		if (triGroup[tri] == group) {
			getTriCenter(tri, center);
			getTriPts(tri, x, y, z);
			triArea = computeAreaOfTriangle(x, y, z);

			meanXYZ[0] += triArea*center[0];//area weighted center
			meanXYZ[1] += triArea*center[1];
			meanXYZ[2] += triArea*center[2];

			area += triArea;
			getTriangleNormal(x, y, z, normal);
			dtprd = DOTPROD3D(averageNormal, normal);
			averageNormal[0] += normal[0];
			averageNormal[1] += normal[1];
			averageNormal[2] += normal[2];
			// for planes
			dtprd = DOTPROD3D(averageNormal, normal) / NORM3D(averageNormal);
			if (dtprd < 0.99) isPlanar = false;
			// for cylinders
			if (nTriSelected == 0) {// the first triangle
				normal0[0] = normal[0];
				normal0[1] = normal[1];
				normal0[2] = normal[2];
			}
			else {
				// take cross-product
				tempVec[0] = (normal0[1] * normal[2] - normal0[2] * normal[1]);
				tempVec[1] = (normal0[2] * normal[0] - normal0[0] * normal[2]);
				tempVec[2] = (normal0[0] * normal[1] - normal0[1] * normal[0]);
				nrm = NORM3D(tempVec);

				if ((tempVec[0] + tempVec[1] + tempVec[2]) > 0) {// avoid cancellations with opposing vectors
					cylinderAxis[0] += tempVec[0] / nrm;
					cylinderAxis[1] += tempVec[1] / nrm;
					cylinderAxis[2] += tempVec[2] / nrm;
				}
			}
			nTriSelected++;
		}
	}
	if (nTriSelected > 0) {//at least one triangle selected
		averageNormal[0] /= nTriSelected;
		averageNormal[1] /= nTriSelected;
		averageNormal[2] /= nTriSelected;
	}
	meanXYZ[0] /= (area);
	meanXYZ[1] /= (area);
	meanXYZ[2] /= (area);
	axisPoint[0] = meanXYZ[0];
	axisPoint[1] = meanXYZ[1];
	axisPoint[2] = meanXYZ[2];
	if ((nTriSelected > 0) && (!isPlanar) && (NORM3D(averageNormal) < 0.02)) {//could be cylinder
		nrm = NORM3D(cylinderAxis);
		cylinderAxis[0] /= nrm;
		cylinderAxis[1] /= nrm;
		cylinderAxis[2] /= nrm;

		isCylinder = true;
		for (int tri = 0; tri < stl_nTriangles; tri++) {
			if (triGroup[tri] == group) {
				getTriPts(tri, x, y, z);
				vec[0] = x[0] - meanXYZ[0];// consider the vector from point on axis to (x,y,z) 
				vec[1] = y[0] - meanXYZ[1];
				vec[2] = z[0] - meanXYZ[2];
				// project the vector onto the axis
				dtprd = vec[0] * cylinderAxis[0] + vec[1] * cylinderAxis[1] + vec[2] * cylinderAxis[2];
				// create the perpendicular vector
				vec[0] -= dtprd*cylinderAxis[0];
				vec[1] -= dtprd*cylinderAxis[1];
				vec[2] -= dtprd*cylinderAxis[2];
				radius = NORM3D(vec);

				if (radiusAvg == 0)
					cylinderRadius = radius;
				else
					cylinderRadius = (radiusAvg + radius) / 2;
				if ((fabs(cylinderRadius - radius) / (cylinderRadius)) > 0.05) {
					isCylinder = false;
					break;
				}
			}
		}
	}

	stl_smallestCylinderRadius = 10 * stl_modelScale;
	if (isCylinder) {
		stl_smallestCylinderRadius =  MIN(stl_smallestCylinderRadius, cylinderRadius);
	}
}
/**********************************************************************/
SURFACE_TYPE  STLGeom::assignHighlightedTrianglesToGroup(int group,double averageNormal[3], double *area,
														   double cylinderAxis[3],double axisPoint[3],double *cylinderRadius)
{
	double x[3],y[3],z[3],normal[3],nrm,dtprd,meanXYZ[3],vec[3],radius,radiusAvg,normal0[3],center[3],triArea,tempVec[3];
	bool isPlanar,isCylinder;
	int i,nTriSelected;
	SURFACE_TYPE surfaceType;

	*area = 0;
	for (i =0; i < 3; i++) {
		averageNormal[i] = 0.0;//used for pressure loading
		cylinderAxis[i] = 0.0;//used for cylinder surfaces
		meanXYZ[i] = 0;//used for cylinder surfaces
		normal0[i] = 0;
	}
	isPlanar = true;
	isCylinder = false;
	nTriSelected = 0;
	radiusAvg = 0.0;
	
    if (stl_verbose) writeMessageToSTLLog("STLGeom: In  assignHighlightedTrianglesToGroup");
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		if (triHighlight[tri]) {	
			triGroup[tri]= group;
			getTriCenter(tri,center);
			getTriPts(tri,x,y,z);
			triArea = computeAreaOfTriangle(x,y,z);
			
			meanXYZ[0] += triArea*center[0];//area weighted center
			meanXYZ[1] += triArea*center[1];
			meanXYZ[2] += triArea*center[2];

			*area += triArea;
			getTriangleNormal(x,y,z,normal);
			dtprd = DOTPROD3D(averageNormal,normal);
			averageNormal[0] += normal[0];
			averageNormal[1] += normal[1];
			averageNormal[2] += normal[2];
			// for planes
			dtprd = DOTPROD3D(averageNormal,normal)/NORM3D(averageNormal);
			if (dtprd < 0.99) isPlanar = false;
			// for cylinders
			if (nTriSelected == 0) {// the first triangle
				normal0[0] = normal[0];
				normal0[1] = normal[1];
				normal0[2] = normal[2];
			}
			else {
				// take cross-product
				tempVec[0] = (normal0[1]*normal[2] -normal0[2]*normal[1]);
				tempVec[1] = (normal0[2]*normal[0] -normal0[0]*normal[2]);
				tempVec[2] = (normal0[0]*normal[1] -normal0[1]*normal[0]);
				nrm = NORM3D(tempVec);

				if ( (tempVec[0]+tempVec[1]+tempVec[2]) > 0){// avoid cancellations with opposing vectors
					cylinderAxis[0] += tempVec[0]/nrm;
					cylinderAxis[1] += tempVec[1]/nrm;
					cylinderAxis[2] += tempVec[2]/nrm;
				}
			}
			nTriSelected++;
		}
	}
	if (nTriSelected > 0) {//at least one triangle selected
		stl_nTriGroups++;
		averageNormal[0] /=nTriSelected;
		averageNormal[1] /=nTriSelected;
		averageNormal[2] /=nTriSelected;
	}
	meanXYZ[0] /=(*area);
	meanXYZ[1] /=(*area);
	meanXYZ[2] /=(*area);
	axisPoint[0] = meanXYZ[0];
	axisPoint[1] = meanXYZ[1];
	axisPoint[2] = meanXYZ[2];
    if ((nTriSelected > 0) && (!isPlanar) && (NORM3D(averageNormal) < 0.1)) {//could be cylinder
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Could be cylinder");
        double radiusError, maxRadius,minRadius;
		nrm = NORM3D(cylinderAxis);
		cylinderAxis[0] /=nrm;
		cylinderAxis[1] /=nrm;
		cylinderAxis[2] /=nrm;

		isCylinder = true;

        maxRadius = -HUGE;
        minRadius = HUGE;
        if (stl_verbose) writeMessageToSTLLog("STLGeom: cylinderAxis = %f %f %f",cylinderAxis[0],cylinderAxis[1],cylinderAxis[2]);
        if (stl_verbose) writeMessageToSTLLog("STLGeom: meanXYZ = %f %f %f",meanXYZ[0],meanXYZ[1],meanXYZ[2]);
		for (int tri = 0; tri < stl_nTriangles; tri++) {
			if (triHighlight[tri]) {
				getTriPts(tri,x,y,z);
				vec[0] = x[0] - meanXYZ[0];// consider the vector from point on axis to (x,y,z) 
				vec[1] = y[0] - meanXYZ[1];
				vec[2] = z[0] - meanXYZ[2];
				// project the vector onto the axis
				dtprd = vec[0]*cylinderAxis[0] +  vec[1]*cylinderAxis[1] + vec[2]*cylinderAxis[2];
				// create the perpendicular vector
				vec[0] -= dtprd*cylinderAxis[0];
				vec[1] -= dtprd*cylinderAxis[1];
				vec[2] -= dtprd*cylinderAxis[2];
				radius = NORM3D(vec);
				


                maxRadius = MAX(maxRadius,radius);
                minRadius = MIN(minRadius,radius);

                radiusError = 2*(maxRadius-minRadius)/(maxRadius+minRadius);
                //if (stl_verbose) writeMessageToSTLLog("STLGeom: radiusError = %f", radiusError);

                if (radiusError > 0.05){
					isCylinder = false;
					break;
				}
			}
            *cylinderRadius = (maxRadius+minRadius)/2;
		}
	}
	if (isPlanar) surfaceType = PLANAR;
	else if (isCylinder) {

		stl_smallestCylinderRadius = MIN(stl_smallestCylinderRadius,*cylinderRadius);
        if (stl_verbose) writeMessageToSTLLog("STLGeom: smallest radius = %e ",stl_smallestCylinderRadius);
		surfaceType = CYLINDER;
	}
	else 
		surfaceType = OTHER;
	return surfaceType;
}
/**********************************************************************/
void STLGeom::setTriGroup(int tri,int group)
{
	triGroup[tri]= group;
	stl_nTriGroups = MAX(stl_nTriGroups,group+1);
    //if (stl_verbose) writeMessageToSTLLog("STLGeom: tri = %d, group= %d, stl_nTriGroups = %d ",tri,group,stl_nTriGroups);

}
/**********************************************************************/
void STLGeom::setLogFilePtr(FILE *logFilePtr)
{
    STL_LOGFILEPTR = logFilePtr;
}
/**********************************************************************/
void STLGeom::dumpTriGroups(void)
{
    writeMessageToSTLLog("----------------------------------------------");
    writeMessageToSTLLog("STL: nTriGroups = %d ",stl_nTriGroups);
	for (int group = 1; group < stl_nTriGroups; group++) {//skip group 0
        writeMessageToSTLLog("%d:  ",group);
		for (int t = 0; t < stl_nTriangles; t++) {
			if (triGroup[t] == group)
                writeMessageToSTLLog("%d ",t);
		}
        writeMessageToSTLLog(" ");
	}
}
/**********************************************************************/
bool	STLGeom::isTriangleHighlighted(int tri)
{
    if ((tri <0) || (tri >= stl_nTriangles)) return false;
	return (triHighlight[tri]);
}
/**********************************************************************/
void STLGeom::highlightTriangle(int tri)
{
    if ((tri <0) || (tri >= stl_nTriangles)) return;
    triHighlight[tri] = true;
}
/**********************************************************************/
void STLGeom::findLargestPlaneNormal(double planeNormal[3])
{
	double cutoffAngleDegrees,area,areaMax,triNormal[3];
	int seedTriangle,depth;

	cutoffAngleDegrees = 1; //degree
	
	for ( seedTriangle = 0; seedTriangle < stl_nTriangles; seedTriangle++) 
		triHighlight[seedTriangle] = false;	
	
	areaMax = 0;
	depth = 0;
	for (seedTriangle = 0; seedTriangle < stl_nTriangles; seedTriangle++) {
		if (triHighlight[seedTriangle]) continue;
		area = getAreaOfTriangle(seedTriangle);
		getNormalOfTriangle(seedTriangle,triNormal);
		highlightTrianglesRecursive(seedTriangle,depth,triNormal,cutoffAngleDegrees,&area);
		if (area > areaMax) {
			areaMax = area;
			planeNormal[0] = triNormal[0];
			planeNormal[1] = triNormal[1];
			planeNormal[2] = triNormal[2];
		}
	}
	for ( seedTriangle = 0; seedTriangle < stl_nTriangles; seedTriangle++) 
		triHighlight[seedTriangle] = false;
}
/**********************************************************************/
void STLGeom::unhighlightTriangle(int triangle)
{
	triHighlight[triangle] =false;
}
/**********************************************************************/
void STLGeom::highlightTrianglesRecursive(int seedTriangle,int depth,double triNormal[3],double cutoffAngleDegrees,double *cumulativeArea)
{
	// use the angle between triangles to select neighbors recursively
	double n1[3],n2[3],cosTheta;
	int previousGroup,currentTri,neighborTri;

	if (cutoffAngleDegrees > 150) {// select all triangles
		*cumulativeArea = 0;
		for (int t = 0; t < stl_nTriangles; t++){
			triHighlight[t] = true;
			*cumulativeArea += getAreaOfTriangle(t);
		}
		return;
	}

	Queue Q;

	Q = CreateQueue(stl_nTriangles);
	MakeEmpty(Q);
	triHighlight[seedTriangle] = true;
	cosTheta = cos(cutoffAngleDegrees*3.1415/180);

	Enqueue(seedTriangle, Q);
	while (!IsEmpty(Q)) {
		currentTri = Front(Q);
		getNormalOfTriangle(currentTri,n1);
		Dequeue(Q);
		for (int i =0; i < 3; i++) {
			neighborTri = triNeighbors[3*currentTri + i];
			getNormalOfTriangle(neighborTri,n2);
			if ( !(triHighlight[neighborTri]) && (DOTPROD3D(n1,n2) > cosTheta) ) {
				triHighlight[neighborTri] = true;
				Enqueue(neighborTri, Q);
			}
		}
	}

	DisposeQueue(Q);

	previousGroup = triGroup[seedTriangle];
	if (previousGroup > 0) { //highlight all triangles with this group
		for (int tri = 0; tri < stl_nTriangles; tri++) 
			if (triGroup[tri] == previousGroup)
				triHighlight[tri] = true;
	}
}
/**********************************************************************/
void STLGeom::unHighlightTrianglesRecursive(int seedTriangle, int depth, double triNormal[3], double cutoffAngleDegrees, double *cumulativeArea)
{
	// use the angle between triangles to unselect neighbors recursively
	double n1[3], n2[3], cosTheta;
	int  currentTri, neighborTri;

	if (cutoffAngleDegrees > 150) {// unselect all triangles
		*cumulativeArea = 0;
		for (int t = 0; t < stl_nTriangles; t++) {
			triHighlight[t] = false;
			*cumulativeArea += getAreaOfTriangle(t);
		}
		return;
	}

	Queue Q;

	Q = CreateQueue(stl_nTriangles);
	MakeEmpty(Q);
	triHighlight[seedTriangle] = false;
	cosTheta = cos(cutoffAngleDegrees*3.1415 / 180);

	Enqueue(seedTriangle, Q);
	while (!IsEmpty(Q)) {
		currentTri = Front(Q);
		getNormalOfTriangle(currentTri, n1);
		Dequeue(Q);
		for (int i = 0; i < 3; i++) {
			neighborTri = triNeighbors[3 * currentTri + i];
			getNormalOfTriangle(neighborTri, n2);
			if ((triHighlight[neighborTri]) && (DOTPROD3D(n1, n2) > cosTheta)) {
				triHighlight[neighborTri] = false;
				Enqueue(neighborTri, Q);
			}
		}
	}

	DisposeQueue(Q);
}
/**********************************************************************/
int STLGeom::highlightTriangleGroup(int group, double *area)
{
	int nTriangles = 0;
	*area = 0;
	for (int t = 0; t < stl_nTriangles; t++) {
		if (triGroup[t] == group){
			triHighlight[t] = true;
			*area += getAreaOfTriangle(t);
			nTriangles++;
		}
		else
			triHighlight[t] = false;
	}
	return nTriangles;
}

/**********************************************************************/
void STLGeom::assignTrianglesToGroupRecursive(int seedTriangle, double triNormal[3],
                                              double cutoffAngleDegrees, int group, int depth)
{
	double n1[3],n2[3],n3[3],cosTheta;
	int t1,t2,t3;

    //if (stl_verbose) writeMessageToSTLLog("STLGeom: assignTrianglesToGroupRecursive; seedTriangle = %d; group = %d,depth = %d", seedTriangle,group,depth);
    if (depth > 1000) return;
	if (triGroup[seedTriangle] >= 0) return;

	triGroup[seedTriangle] = group;
	cosTheta = cos(cutoffAngleDegrees*3.1415/180);
	t1 = triNeighbors[3*seedTriangle];
	t2 = triNeighbors[3*seedTriangle+1];
	t3 = triNeighbors[3*seedTriangle+2];
    //if (stl_verbose) writeMessageToSTLLog("STLGeom: Neighboring triangles %d %d %d ",t1,t2,t3);
    if ((t1 >= 0) && (triGroup[t1] <0)) {//unassigned
		getNormalOfTriangle(t1,n1);
		if ((DOTPROD3D(triNormal,n1) > cosTheta) ){
            assignTrianglesToGroupRecursive(t1,n1,cutoffAngleDegrees,group,depth+1);
		}
	}
    if ((t2 >= 0) &&(triGroup[t2] < 0)) {//unhighlighted
		getNormalOfTriangle(t2,n2);
		if ( (DOTPROD3D(triNormal,n2) > cosTheta) ){
            assignTrianglesToGroupRecursive(t2,n2,cutoffAngleDegrees,group,depth+1);
		}
	}
    if ((t3 >= 0) && (triGroup[t3] < 0)) {//unhighlighted
		getNormalOfTriangle(t3,n3);
		if ( (DOTPROD3D(triNormal,n3) > cosTheta) ){
            assignTrianglesToGroupRecursive(t3,n3,cutoffAngleDegrees,group,depth+1);
		}
	}
}
/**********************************************************************/
void STLGeom::computeContributionOfTriangle(double x[3],double y[3],double z[3],double *vol,double *area,double center[3])
{
	//uses divergence theorem
	double xBar,yBar,zBar,nz,normal[3];
	*area = computeAreaOfTriangle(x,y,z);
	*area = MAX(*area,10*TOL);
	getTriangleNormal(x,y,z,normal);
	nz = normal[2];
	xBar = (x[0]+x[1]+x[2])/3.0;
	yBar = (y[0]+y[1]+y[2])/3.0;
	zBar = (z[0]+z[1]+z[2])/3.0;

	*vol  = *area*nz*zBar;

    center[0] = *area*xBar*xBar*normal[0]/2;// approximation
    center[1] = *area*yBar*yBar*normal[1]/2;// approximation
    center[2] = *area*zBar*zBar*normal[2]/2;// approximation

}
/**********************************************************************/
double STLGeom::getMinimumThicknessOfPart(int part)
{
	double x[3],y[3],z[3],range[3][2],minThickness;
	
	for (int i =0; i < 3; i++){
		range[i][0] = HUGE_VAL;
		range[i][1] = -HUGE_VAL;
	}
	
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		if (triPart[tri] != part) continue;
		getTriPts(tri,x,y,z);
		for (int i =0; i < 3;i++){
			range[0][0] = MIN(range[0][0],x[i]);
			range[0][1] = MAX(range[0][1],x[i]);

			range[1][0] = MIN(range[1][0],y[i]);
			range[1][1] = MAX(range[1][1],y[i]);

			range[2][0] = MIN(range[2][0],z[i]);
			range[2][1] = MAX(range[2][1],z[i]);
		}
	}
	minThickness = range[0][1]-range[0][0];
	minThickness = MIN(minThickness,range[1][1]-range[1][0]);
	minThickness = MIN(minThickness,range[2][1]-range[2][0]);
	return minThickness;
}
/**********************************************************************/
void STLGeom::getCenterOfSTLPart(int part, double center[3])
{
	for (int i =0; i < 3; i++)
		center[i] = stl_partCenter[part][i];
}
/**********************************************************************/
double STLGeom::getAreaOfSTLPart(int part)
{
	return stl_partArea[part];
}
/**********************************************************************/
double STLGeom::getVolumeOfSTLPart(int part)
{
	return stl_partVolume[part];
}
/**********************************************************************/
double STLGeom::computeVolumeOfSTLPart(int part,double *totalArea, double center[3])
{
	double x[3],y[3],z[3], vol,area,volTri,centerTri[3];
	
	vol = 0.0;
	*totalArea= 0.0;
	for (int i = 0; i <3; i++)
		center[i] = 0.0;
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		if (triPart[tri] != part) continue;
		getTriPts(tri,x,y,z);
		computeContributionOfTriangle(x,y,z,&volTri,&area,centerTri);
		vol += volTri;
		*totalArea += area;
		for (int i = 0; i <3; i++)
			center[i] += centerTri[i];
	}
	for (int i = 0; i <3; i++)
        center[i] /=(vol);

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Part (%d) volume = %e; area = %e ",part,vol,*totalArea);

    if (stl_verbose) writeMessageToSTLLog("STLGeom: CG (%.3g,%.3g, %.3g) ",center[0],center[1],center[2]);
	return vol;
}
/**********************************************************************/
void STLGeom::getBoundingBoxOfPart(int partNo,double boundingBox[3][2])
{
	if(partNo > getNumParts() )
	{
        writeMessageToSTLLog("STL: Invalid part ");
		return;
	}
	double x[3],y[3],z[3];
	for(int i =0 ; i  <3; i++)
	{
		boundingBox[i][0] = 1e12;
		boundingBox[i][1] = -1e12;
	}
	for(int tri = 0 ; tri < stl_nTriangles ; tri++)
	{
		if(triPart[tri] != partNo)	 continue;
		getTriPts(tri,x,y,z);
		for(int j = 0 ; j < 3 ; j++)
		{
			boundingBox[0][0] = MIN(boundingBox[0][0],x[j]);
			boundingBox[1][0] = MIN(boundingBox[1][0],y[j]);
			boundingBox[2][0] = MIN(boundingBox[2][0],z[j]);
			boundingBox[0][1] = MAX(boundingBox[0][1],x[j]);
			boundingBox[1][1] = MAX(boundingBox[1][1],y[j]);
			boundingBox[2][1] = MAX(boundingBox[2][1],z[j]);
		}
	}
}
/**********************************************************************/
bool STLGeom::checkForHollowBodies(void)
{
	// If a part is hollow, then the triangle walking method will identify the part as two different parts 
	// Incorrectly identified parts will have negative volume
	int nPositiveBodies=0,nNegativeBodies=0;
	double totalVol=0, totalArea=0,center[] = {0,0,0};

	for (int part = 0; part < stl_nParts; part++) {	
		if (stl_partVolume[part] > 0){
			nPositiveBodies++;
		}
		else
			nNegativeBodies++;

		totalVol += fabs(stl_partVolume[part]);
		totalArea += fabs(stl_partArea[part]);
		for (int i =0; i < 3; i++)
			center[i] +=  fabs(stl_partVolume[part])*stl_partCenter[part][i];
	}
	for (int i =0; i < 3; i++)
		center[i] /=totalVol;

	if (nNegativeBodies == 0)
		return true;//no hollow parts, no issue

	//else
	if (nPositiveBodies != 1){
        writeMessageToSTLLog("STLGeom: nNegativeBodies = %d,nPositiveBodies = %d",nNegativeBodies,nPositiveBodies);
		return false;// there are more than one positive part and non-zero negative parts, not implemented as yet
	}

	//else simply push all the negative bodies to the positive part (part 0)
    writeMessageToSTLLog("STLGeom: Reassigning parts to part 0 ");
	stl_nParts = 1;
	for(int t1 =0; t1 < stl_nTriangles; t1++) 
		triPart[t1] = 0;

	precomputePartProperties();
    writeMessageToSTLLog("STLGeom: Number of parts = 1");
	return true;
}
/**********************************************************************/
void STLGeom::precomputePartProperties(void)
{
	double  partVol,partArea,center[3];

    writeMessageToSTLLog("STLGeom: In precomputePartProperties; stl_nParts %d",stl_nParts);
	for (int part = 0; part < stl_nParts; part++) {	
		partVol = computeVolumeOfSTLPart(part,&partArea,center);
		stl_partArea[part] = partArea;
		stl_partVolume[part] = partVol;
        writeMessageToSTLLog("STLGeom: part %d, area = %.2e, vol = %.2e",part,partArea,partVol);
		for (int i =0; i < 3; i++)
			stl_partCenter[part][i] = center[i];
	}
}
/**********************************************************************/
double STLGeom::getVolumeOfSTLGeom(double *totalArea)
{
	double  vol;
	
	vol = 0.0;
	*totalArea= 0.0;
	for (int part = 0; part < stl_nParts; part++) {	
		vol += stl_partVolume[part];
		*totalArea += stl_partArea[part];
	}
    if (stl_verbose) writeMessageToSTLLog("STLGeom: STLGeom volume = %e; area = %e ",vol,*totalArea);
	return vol;
}
/**********************************************************/
int STLGeom::findCommonElements(int n,int *v1, int *v2, int *vCommon)
{
    int nCommon = 0,i,k;
    for (i = 0; i < n; i++) {
        for (k =0; k < n; k++) {
			if (v1[i] == v2[k]){
                vCommon[nCommon] = v1[i];
				nCommon++;
			}
        }
    }
    return (nCommon);
}
/**********************************************************/
int STLGeom::findNumCommonElements(int n,int *v1, int *v2)
{
    int nCommon = 0,i,k;
    for (i = 0; i < n; i++) {
        for (k =0; k < n; k++) {
            if (v1[i] == v2[k])
                nCommon++;
        }
    }
    return (nCommon);
}
/**********************************************************/
bool STLGeom::findTriNeighbors(int useGPU)
/* compute the neighbors of triangles
triNodes: (n1,n2,n3,...) 3 nodes for each triangle, followed by next triangle
triNeighbors: (t1,t2,t3,...)3 neighbors for each triangle, followed by next triangle
*/
{
   
    int i, nCommonNodes,t1,t2;
    bool manifoldGeometry;
	
	for( t1 =0; t1 < stl_nTriangles; t1++) {
        for (i = 0; i < 3;i++)
            triNeighbors[3*t1+i] = -1;
    }
#ifdef COMPILE_GPUCODE
	if (useGPU) {// GPU
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Finding neighbors using GPU");
		findTriNeighborsGPU(stl_nTriangles,triNodes,triNeighbors);
        return true;
	}
	
#endif
    manifoldGeometry = true;
	int *nNeighbors = (int *)malloc(stl_nTriangles*sizeof(int));
	memset(nNeighbors,0,stl_nTriangles*sizeof(int));
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Finding neighbors using CPU");
	for( t1 =0; t1 < stl_nTriangles; t1++) {
		if(nNeighbors[t1]==3){
			continue;//all three have been found
		}
		for( t2 = t1+1; t2 < stl_nTriangles; t2++) {
			nCommonNodes = findNumCommonElements(3,&(triNodes[t1*3]),&(triNodes[t2*3]));
			if (nCommonNodes == 2) {
				triNeighbors[t2*3+nNeighbors[t2]] = t1;
				nNeighbors[t2]++;
				triNeighbors[t1*3+nNeighbors[t1]] = t2;
				if(++nNeighbors[t1]==3) {
					break;//all three have been found
				}
			}
		}
        if(nNeighbors[t1]<3){
            writeMessageToSTLLog("STLGeom: Non-manifold geometry; triangle %d with less than 3 neighbors",t1);
            manifoldGeometry = false;
        }
	}
	free(nNeighbors);
    return (manifoldGeometry);
}

/**********************************************************************/
void	STLGeom::snapPointToTriangulation(double *pt,double tolerance)
{
	int i,j,nodes[3];
	double *pt1,*pt2,*pt3,d,stu[3],ptClosest[3],ptTemp[3];

	for (j = 0; j < 3; j++)
		ptTemp[j] = pt[j];

	for ( i = 0; i < stl_nTriangles; i++) {
		//if (triGroup[i] == 0) continue;//// don't worry about triangles that have no boundary conditions
		for (j = 0; j < 3; j++)
			nodes[j] = triNodes[3*i+j];
		pt1 = &(triXYZ[3*nodes[0]]);
		pt2 = &(triXYZ[3*nodes[1]]);
		pt3 = &(triXYZ[3*nodes[2]]);
		d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3, ptClosest,stu);
		if (d < tolerance){
			for (j = 0; j < 3; j++)
				ptTemp[j] = ptClosest[j];
			tolerance = d;
		}
	}
	for (j = 0; j < 3; j++)
		pt[j] = ptTemp[j];
	return;
}
/**********************************************************************/
void	STLGeom::snapPointToTriangle(double *pt,int tri,double tolerance)
{
	int j,nodes[3];
	double *pt1,*pt2,*pt3,d,stu[3],ptClosest[3],ptTemp[3];

	for (j = 0; j < 3; j++)
		ptTemp[j] = pt[j];

	for (j = 0; j < 3; j++)
		nodes[j] = triNodes[3*tri+j];
	pt1 = &(triXYZ[3*nodes[0]]);
	pt2 = &(triXYZ[3*nodes[1]]);
	pt3 = &(triXYZ[3*nodes[2]]);
	d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3, ptClosest,stu);
	if (d < tolerance){
		for (j = 0; j < 3; j++)
			ptTemp[j] = ptClosest[j];
		tolerance = d;
	}
	for (j = 0; j < 3; j++)
		pt[j] = ptTemp[j];
	return;
}
/**********************************************************************/
void	STLGeom::snapPointToGeometry(double *pt,double tolerance)
{
    snapPointToTriangulation(pt,tolerance);
    snapPointToEdge(pt,1.65*tolerance);
    snapPointToCorner(pt,1.9*tolerance);
}
/**********************************************************************/
void	STLGeom::snapPointToCorner(double *pt,double tolerance)
{
	int i,j,node;
	double *pt1,d,ptTemp[3];

	for (j = 0; j < 3; j++)
		ptTemp[j] = pt[j];
	for ( i = 0; i < stl_nSharpCorners; i++) {
		node = triCorners[i];
		pt1 = &(triXYZ[3*node]);
		d = DIST3D(pt,pt1);
		if (d < tolerance){
			for (j = 0; j < 3; j++)
				ptTemp[j] = pt1[j];
			tolerance = d;
		}
	}
	for (j = 0; j < 3; j++)
		pt[j] = ptTemp[j];
	return;
}
/**********************************************************************/
bool	STLGeom::isPointCloseToAnSTLEdge(double *pt,double tolerance)
{
	int nodes[2];
	double *pt1,*pt2,d,ptClosest[3];

	for (int i = 0; i < stl_nSharpEdges; i++) {
		for (int j = 0; j < 2; j++)
			nodes[j] = triEdges[2*i+j];
		pt1 = &(triXYZ[3*nodes[0]]);
		pt2 = &(triXYZ[3*nodes[1]]);
		d = distanceOfPointFromLine3D(pt,pt1,pt2, ptClosest);
		if (d < tolerance) return true;
	}
	return false;
}
/**********************************************************************/
void	STLGeom::snapPointToEdge(double *pt,double tolerance)
{
	int i,j,nodes[2];
	double *pt1,*pt2,d,ptClosest[3],ptTemp[3];


	for (j = 0; j < 3; j++)
		ptTemp[j] = pt[j];
	for ( i = 0; i < stl_nSharpEdges; i++) {
		for (j = 0; j < 2; j++)
			nodes[j] = triEdges[2*i+j];
		pt1 = &(triXYZ[3*nodes[0]]);
		pt2 = &(triXYZ[3*nodes[1]]);
		d = distanceOfPointFromLine3D(pt,pt1,pt2, ptClosest);
		if (d < tolerance){
			for (j = 0; j < 3; j++)
				ptTemp[j] = ptClosest[j];
			tolerance  = d;
		}
	}
	for (j = 0; j < 3; j++)
		pt[j] = ptTemp[j];
	return;
}
/**********************************************************************/
void STLGeom::segmentModel(double creaseAngleInDegrees)
{
    // Use neighbor information to segment
    int t1, currentGroup;
    double n1[3];

    if (stl_verbose)
        writeMessageToSTLLog("STLGeom: In segmentModel, stl_nTriangles = %d ",stl_nTriangles);

    for( t1 =0; t1 < stl_nTriangles; t1++)
        triGroup[t1] = -1; //

    currentGroup = 0;
    int depth = 0;
    for( t1 =0; t1 < stl_nTriangles; t1++) {
        if (triGroup[t1] >=0 ) continue;//already assigned to a group
        getNormalOfTriangle(t1,n1);
        assignTrianglesToGroupRecursive(t1, n1,creaseAngleInDegrees,currentGroup,depth);
        computeSmallestRadius(currentGroup);
        currentGroup++;
    }
    writeMessageToSTLLog("STLGeom: #Groups = %d",currentGroup);
    writeMessageToSTLLog("STLGeom: Smallest radius = %e ", stl_smallestCylinderRadius);
    //reset
    for (t1 = 0; t1 < stl_nTriangles; t1++)
        triGroup[t1] = -1; //
}
/**********************************************************************/
bool STLGeom::findExtremeVerticesAlongDirection(double dir[3], double minPt[3],double maxPt[3],double minMaxHeights[2])
{
    // find the limits of the model along the given direction
    // return true if successful, else returns false
    // normalize the direction
    double nrm = NORM3D(dir);
    double direction[3],pt[3],projection;
    if (nrm < TOL) return false;

    for (int i = 0; i < 3; i++)
        direction[i] = dir[i]/ nrm;

    minMaxHeights[0] = 1e8;
    minMaxHeights[1] = -1e8;
    for (int i = 0; i < stl_ntriPoints; i++){
        pt[0] = triXYZ[3*i+0];
        pt[1] = triXYZ[3*i+1];
        pt[2] = triXYZ[3*i+2];

        projection = DOTPROD3D(direction,pt);

        if (projection < minMaxHeights[0] ){
            minMaxHeights[0] = projection;
            for (int j = 0; j < 3; j++)
                minPt[j] = pt[j];
        }

        if (projection > minMaxHeights[1] ){
            minMaxHeights[1] = projection;
            for (int j = 0; j < 3; j++)
                maxPt[j] = pt[j];
        }
    }
}
/**********************************************************************/
int STLGeom::segmentModelAndFindCanonicalDirections(double creaseAngleInDegrees, double directionTol, std::vector<Point3d> &canonicalDirections)
{
    // find normals to planes, and cylinder axis
	// Use neighbor information to segment 
    int t1, currentGroup;
    double n1[3];
    Point3d direction;
    Point3d dir1;
    double dotPrd;
	
	if (stl_verbose) 
        writeMessageToSTLLog("STLGeom: In segmentModel, stl_nTriangles = %d ",stl_nTriangles);

	for( t1 =0; t1 < stl_nTriangles; t1++)
		triGroup[t1] = -1; //

    currentGroup = 0;
    int depth = 0;

    canonicalDirections.clear();
    // add all the cartesian directions
    for (int i = 0; i < 3; i++){
        direction.x = direction.y = direction.z = 0;
        if (i ==0) direction.x = 1.0;
        else if (i ==1) direction.y = 1.0;
        else direction.z = 1.0;
        canonicalDirections.push_back(direction);

        direction.x = direction.y = direction.z = 0;
        if (i ==0) direction.x = -1.0;
        else if (i ==1) direction.y = -1.0;
        else direction.z = -1.0;
        canonicalDirections.push_back(direction);
    }
	for( t1 =0; t1 < stl_nTriangles; t1++) {
		if (triGroup[t1] >=0 ) continue;//already assigned to a group
		getNormalOfTriangle(t1,n1);
        assignTrianglesToGroupRecursive(t1, n1,creaseAngleInDegrees,currentGroup,depth);
        if (findCanonicalDirectionOfGroup(currentGroup,direction)){
            // check if that direction already exists
            bool exists;
            exists = false;
            for (int i = 0; i < canonicalDirections.size(); i++){
                 dir1 = canonicalDirections.at(i);
                 // we will assume that these directions are normalized
                 dotPrd = dir1.x*direction.x + dir1.y*direction.y + dir1.z*direction.z;
                 // if the dotPrd is sufficiently large then we disregard
                 if (dotPrd > directionTol){
                     exists = true;
                     break;
                 }
            }
            if (!exists){
                canonicalDirections.push_back(direction);
                // reverse direction and add to list
                direction.x = -direction.x;
                direction.y = -direction.y;
                direction.z = -direction.z;
                canonicalDirections.push_back(direction);
            }
        }

		currentGroup++;
	}

	for (t1 = 0; t1 < stl_nTriangles; t1++)
		triGroup[t1] = -1; //

    return canonicalDirections.size();
}
/**********************************************************************/
int	STLGeom:: getNumberOfConcaveEdges(void)
{
	return stl_nConcaveEdges;
}
/**********************************************************************/
bool STLGeom::isEdgeConcave(int edge)
{
	if ((edge < 0) || (edge >= stl_nSharpEdges)) return false;

	return edgeConcavity[edge];
}
/**********************************************************************/
void STLGeom::findSharpEdgesAndCorners(double creaseAngleInDegrees)
{
	// Use neighbor information to segment 
	int t1, i,t2,commonNodes[2],nCommonNodes,ii,jj;
	double n1[3],n2[3],cosTheta,center1[3],center2[3],vec[3],nrm;
	
	stl_nSharpEdges = 0;
    if (stl_verbose) writeMessageToSTLLog("STLGeom: In findSharpEdgesAndCorners ");

	cosTheta = cos(3.1415*creaseAngleInDegrees/180);
	for( t1 =0; t1 < stl_nTriangles; t1++) {
		getNormalOfTriangle(t1,n1);
		for (i = 0; i < 3; i++) {
			t2  = triNeighbors[3*t1 + i];
			if (t2 < t1) continue;// avoids duplication
			getNormalOfTriangle(t2,n2);

			if (DOTPROD3D(n1,n2) < cosTheta ){// this is a sharp edge
				nCommonNodes= 0;
				// find the common nodes
				for (ii = 0; ii < 3; ii++){
					for (jj = 0; jj < 3; jj++){
						if (triNodes[3*t1 +ii] == triNodes[3*t2 + jj]){
							commonNodes[nCommonNodes] = triNodes[3*t1 +ii] ;
							nCommonNodes++;
						}
					}
				}
				
				triEdges[stl_nSharpEdges*2 + 0] = commonNodes[0];
				triEdges[stl_nSharpEdges*2 + 1] = commonNodes[1];

                triEdgesParents[stl_nSharpEdges*2 + 0] = t1;
                triEdgesParents[stl_nSharpEdges*2 + 1] = t2;

				// now check is it a rentrant corner
				edgeConcavity[stl_nSharpEdges] = false;
				
				getTriCenter(t1,center1);
				getTriCenter(t2,center2);
				SUBTRACT3D(vec,center2,center1);
				nrm = NORM3D(vec);
				SCALE(vec,nrm);
				if (DOTPROD3D(vec,n1) > 0.01){
					edgeConcavity[stl_nSharpEdges] = true;
					stl_nConcaveEdges++;
				}
				stl_nSharpEdges++;
				stl_nSharpEdges = MIN(stl_nSharpEdges,MAX_EDGES-1);
			}
		}
	}
	findSharpCorners();
    writeMessageToSTLLog("STLGeom: #Sharp edges = %d ",stl_nSharpEdges);
}
/**********************************************************************/
void STLGeom::setNumberOfPartsToZero(void)
// assigned by the user
{
    stl_nParts = 0;
}
/**********************************************************************/
bool STLGeom::assignPartNumberToTriangles(int startTri, int endTri, int partId)
// assigned by the user
{
	if ((startTri > endTri) || (endTri >= stl_nTriangles))
		return false;

	for (int tri = startTri; tri <= endTri; tri++)
		triPart[tri] = partId;

	stl_nParts = MAX(stl_nParts,partId+1);
	return true;
}
/**********************************************************************/
void STLGeom::findStartEndTrianglesOfParts(void)
// assigned by the user
{
	int partId;

	for (partId = 0; partId < MAX_STL_PARTS; partId++){
		triStartEnd[partId][0] = stl_nTriangles-1;//start
		triStartEnd[partId][1] = 0;//end
	}
	for(int tri =0; tri < stl_nTriangles; tri++) {
        partId = MIN(MAX(0,triPart[tri]),MAX_STL_PARTS-1);
		triStartEnd[partId][0] = MIN(triStartEnd[partId][0],tri);
		triStartEnd[partId][1] = MAX(triStartEnd[partId][1],tri);
	}
}
/**********************************************************************/
void STLGeom::autoAssignPartNumberToTriangles(void)
// find the number of components (parts) in an assembly
{
	int t1, nParts,currentTri,neighborTri,i;
	bool *inQueue;
	Queue Q;
	
	inQueue = (bool *)malloc((stl_nTriangles)*sizeof(bool));
	memset(inQueue,false,stl_nTriangles*sizeof(bool));
	
	for( t1 =0; t1 < stl_nTriangles; t1++) 
		triPart[t1] = -1;

	Q = CreateQueue(stl_nTriangles);
	MakeEmpty(Q);
	nParts = 0;

	for( t1 =0; t1 < stl_nTriangles; t1++) {
		if (triPart[t1] == -1) {	
			Enqueue(t1, Q);
			while (!IsEmpty(Q)) {
				currentTri = Front(Q);
				Dequeue(Q);
				triPart[currentTri] = nParts;
				for (i =0; i < 3; i++) {
					neighborTri = triNeighbors[3*currentTri + i];
					if ((!inQueue[neighborTri]) && (triPart[neighborTri] == -1)) {
						Enqueue(neighborTri, Q);
						inQueue[neighborTri] = true;
					}
				}
			}
			nParts++;
		}
	}
    writeMessageToSTLLog("STLGeom: Number of Parts = %d ",nParts);
	stl_nParts = nParts;
	DisposeQueue(Q);
	free(inQueue);
}
/**********************************************************************/
void STLGeom::findSharpCorners(void)
{

    if (stl_verbose) writeMessageToSTLLog("STLGeom: In findSharpCorners ");
	int *nodesVisited,node1,node2;
	// use sharp edges to find sharp corners
	nodesVisited = (int *)malloc(stl_ntriPoints*sizeof(int));
	memset(nodesVisited,0,stl_ntriPoints*sizeof(int));
	for (int e = 0; e < stl_nSharpEdges;e++) {
		node1 = triEdges[e*2 + 0];
		node2 = triEdges[e*2 + 1];
		nodesVisited[node1]++;
		nodesVisited[node2]++;
	}
	// if a node has been visited more than twice, it is a sharp corner
	stl_nSharpCorners = 0;
	for (int i =0; i < stl_ntriPoints;i++){
		if (nodesVisited[i] >= 3) {
			triCorners[stl_nSharpCorners] = i;
			stl_nSharpCorners++;
			stl_nSharpCorners = MIN(stl_nSharpCorners,MAX_CORNERS-1);
		}
	}
	free(nodesVisited);
}
/**********************************************************************/
int STLGeom::getNumSharpEdges(void)
{
	return stl_nSharpEdges;
}
/**********************************************************************/
void STLGeom::getSharpEdgeNodes(int edge,int nodes[2])
{
	edge = MIN(edge,stl_nSharpEdges-1);

	nodes[0] = triEdges[edge*2 + 0];
	nodes[1] = triEdges[edge*2 + 1];
}
/**********************************************************************/
void STLGeom::getSharpEdgePoints(int edge,double x[],double y[],double z[])
{
	int n1,n2;
	edge = MIN(edge,stl_nSharpEdges-1);

	n1 = triEdges[edge*2 + 0];
	n2 = triEdges[edge*2 + 1];

	x[0]= triXYZ[3*n1 + 0];
	y[0]= triXYZ[3*n1 + 1];
	z[0]= triXYZ[3*n1 + 2];

	x[1]= triXYZ[3*n2 + 0];
	y[1]= triXYZ[3*n2 + 1];
	z[1]= triXYZ[3*n2 + 2];


	return;
}

/**********************************************************************/
void STLGeom::dumpTriangleSegmentation(char *fileName)
{

	int t1;
	FILE *fptr = fopen(fileName,"w");

	for( t1 =0; t1 < stl_nTriangles; t1++) 
		fprintf(fptr,"%d %d \n",t1,triGroup[t1]);
	fclose(fptr);
}
/**********************************************************************/
int STLGeom::rayIntersectTri(double orig[3], double dir[3],
			double vert0[3], double vert1[3], double vert2[3],
			double *t, double *u, double *v)
{
	// Ray-triangle intersection

   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUBTRACT3D(edge1, vert1, vert0);
   SUBTRACT3D(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS3D(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOTPROD3D(edge1, pvec);

   /* calculate distance from vert0 to ray origin */
   SUBTRACT3D(tvec, orig, vert0);
   inv_det = 1.0 / det;

   if (det > TOL)
   {
	   /* calculate U parameter and test bounds */
	   *u = DOTPROD3D(tvec, pvec);
	   if (*u < 0.0 || *u > det)
		   return 0;
	   /* prepare to test V parameter */
	   CROSS3D(qvec, tvec, edge1);

	   /* calculate V parameter and test bounds */
	   *v = DOTPROD3D(dir, qvec);
	   if (*v < 0.0 || *u + *v > det)
		   return 0;
   }
   else if(det < -TOL)
   {
	   /* calculate U parameter and test bounds */
	   *u = DOTPROD3D(tvec, pvec);
	   if (*u > 0.0 || *u < det)
		   return 0;
	   /* prepare to test V parameter */
	   CROSS3D(qvec, tvec, edge1);

	   /* calculate V parameter and test bounds */
	   *v = DOTPROD3D(dir, qvec) ;
	   if (*v > 0.0 || *u + *v < det)
		   return 0;
   }
   else {
       return 0;  /* ray is parallel to the plane of the triangle */
   }
   /* calculate t, ray intersects triangle */
   *t = DOTPROD3D(edge2, qvec) * inv_det;
    if (*t < 0) return 0;// not in the ray direction
   (*u) *= inv_det;
   (*v) *= inv_det;


   return 1;
}
void STLGeom::findSymmetry(void)
{
	// find  volume integral of moments x^n, y^n,z^n for even values of n about center.
	// These should be zero for symmetry.
	// To find volume integrals, Gaussian integration is used along with divergence theorem
    double cx,cy,cz,totalArea;
	double area,normal[3],xTri[3],yTri[3],zTri[3];
	double xN[] = {0,0,0,0};
	double yN[] = {0,0,0,0};
    double zN[] = {0,0,0,0};
    double xyzMin[3],xyzMax[3],lengthScales[3];
	double x,y,z,xi,eta,lambda,wt;
	double result[] = {0,0,0},maxResult;
    int N,jMax;
	// gauss points and weight (7 points)
	double xi_eta_wt[7][3]={{0.33333333333333, 0.33333333333333, 0.22500000000000},
	{0.47014206410511, 0.47014206410511, 0.13239415278851},
	{0.47014206410511, 0.05971587178977, 0.13239415278851},
	{0.05971587178977, 0.47014206410511, 0.13239415278851},
	{0.10128650732346, 0.10128650732346, 0.12593918054483},
	{0.10128650732346, 0.79742698535309, 0.12593918054483},
	{0.79742698535309, 0.10128650732346, 0.12593918054483}};
			
	
	stl_symmetry[0] = false;
	stl_symmetry[1] = false;
	stl_symmetry[2] = false;

    for (int i = 0; i < 3; i++)
        lengthScales[i] = (stl_XYZMax[i] - stl_XYZMin[i]);



	cx = (stl_XYZMax[0] + stl_XYZMin[0])/2;
	cy = (stl_XYZMax[1] + stl_XYZMin[1])/2;
	cz = (stl_XYZMax[2] + stl_XYZMin[2])/2;
	
	
	totalArea = 0;
    jMax = 3;
    for(int i = 0; i < stl_nTriangles; i++){
		area = getAreaOfTriangle(i);
		totalArea = totalArea + area;
		getNormalOfTriangle(i,normal);
        getTriPts(i,xTri,yTri,zTri);
		
		double totalWt = 0;
		for (int gp = 0; gp < 7; gp++) {
			xi = xi_eta_wt[gp][0];
			eta = xi_eta_wt[gp][1];
			wt = xi_eta_wt[gp][2];
			lambda = 1-xi-eta;
			totalWt = totalWt+ wt;
			x = lambda*xTri[0] + xi*xTri[1] + eta*xTri[2];
			y = lambda*yTri[0] + xi*yTri[1] + eta*yTri[2];
			z = lambda*zTri[0] + xi*zTri[1] + eta*zTri[2];
			
            for (int j = 0; j <= jMax; j++) {
                N = 4 + j*2;
				xN[j] += area*normal[0]*wt*pow((x-cx),N);
				yN[j] += area*normal[1]*wt*pow((y-cy),N);
				zN[j] += area*normal[2]*wt*pow((z-cz),N);
			}
		}
    }

    for (int j = 0; j <= jMax; j++) {// scale correctly
        N = 4 + j*2;
		xN[j] = fabs(xN[j]);
		yN[j] = fabs(yN[j]);
		zN[j] = fabs(zN[j]);
        xN[j] /= totalArea*pow(lengthScales[0],N);
        yN[j] /= totalArea*pow(lengthScales[1],N);
        zN[j] /= totalArea*pow(lengthScales[2],N);
		result[0] = MAX(result[0],xN[j]);
		result[1] = MAX(result[1],yN[j]);
		result[2] = MAX(result[2],zN[j]);

	}	
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Symmetry errors: %e %e %e ",result[0],result[1],result[2]);
	maxResult = MAX(result[2],MAX(result[1],result[0]));
	// condition 1: if any of the results are small then symmetry
	// condition 2: relative to maximum also 
    if ((result[0] < 1e-6) || (result[0]/maxResult < 1e-6))
		stl_symmetry[0] = true;
    if ((result[1] < 1e-6) || (result[1]/maxResult < 1e-6))
		stl_symmetry[1] = true;
    if ((result[2] < 1e-6) || (result[2]/maxResult < 1e-6))
		stl_symmetry[2] = true;
}

/**********************************************************************/
int STLGeom::findClosestTriangleIntesectingRay(double origin[3],double dir[3], double *tMin)
{
    // Input: origin of the ray, and the direction of the ray
    // Output: the t value corresponding to the intersection (positive value)
    //         The pt of intersection is origin + t*dir
    // Returns: the triangle, else -1 if no triangle was found

    int i,tri,trinode,triIntersected;
    double tripts[3][3],u,v,t;

    triIntersected = -1;
    *tMin = 1e14;
    for (tri = 0; tri < stl_nTriangles; tri++) {
        for (i =0; i <3; i++) {
            trinode = triNodes[3*tri+i];
            tripts[i][0] = triXYZ[3*trinode];
            tripts[i][1] = triXYZ[3*trinode+1];
            tripts[i][2] = triXYZ[3*trinode+2];
        }
        if(rayIntersectTri(origin,dir,tripts[0],tripts[1],tripts[2],&t,&u,&v)) {
            if (t < *tMin) {
                *tMin = t;
                triIntersected = tri;
            }
        }
    }
    return triIntersected;
}
/**********************************************************************/
int STLGeom::findClosestTriangleIntesectingRaySkippingParts(vector<int> partsToSkip,double origin[3],double dir[3], double *tMin)
{
    // Input: origin of the ray, and the direction of the ray
    // Output: the t value corresponding to the intersection (positive value)
    //         The pt of intersection is origin + t*dir
    // Returns: the triangle, else -1 if no triangle was found

    int i,tri,trinode,triIntersected,part;
	double tripts[3][3],u,v,t;

	triIntersected = -1;
    *tMin = 1e14;

    std::vector<int>::iterator it;
	for (tri = 0; tri < stl_nTriangles; tri++) {
        part = getParentPartOfTriangle(tri);
        it = std::find (partsToSkip.begin(), partsToSkip.end(), part);
        if (it != partsToSkip.end()) // part is in list
            continue;
		for (i =0; i <3; i++) {
			trinode = triNodes[3*tri+i];
			tripts[i][0] = triXYZ[3*trinode];
			tripts[i][1] = triXYZ[3*trinode+1];
			tripts[i][2] = triXYZ[3*trinode+2];
		}
		if(rayIntersectTri(origin,dir,tripts[0],tripts[1],tripts[2],&t,&u,&v)) {
			if (t < *tMin) {
				*tMin = t;
				triIntersected = tri;
			}
		}
	}
	return triIntersected;
}
/**********************************************************************/
bool STLGeom::isPointInsidePart(double pt[3],int part)
{
   double **intersectionPts;
   int *intersectionTris,i,nIntersections;
   double dir[3],center[3],nrm;

   intersectionTris = (int *)malloc(MAXINTERSECTIONS*sizeof(int));
   intersectionPts = (double **)malloc(MAXINTERSECTIONS*sizeof(double *));
   for (i = 0;i < MAXINTERSECTIONS; i++)
       intersectionPts[i] = (double *)malloc(3*sizeof(double));

   getCenterOfSTLPart(part,center);
   for (i = 0;i < 3; i++)
       dir[i] = center[i] - pt[i];

   nrm = NORM3D(dir);
   for (i = 0;i < 3; i++)
       dir[i] /= nrm;

   nIntersections = findIntersectionPointsWithSTLGeom(part,pt,dir,intersectionPts,intersectionTris);

   for (i = 0;i < MAXINTERSECTIONS; i++)
       free(intersectionPts[i]);
   free(intersectionTris);
   free(intersectionPts);

   if (nIntersections%2 == 0) return false; // odd number of intersections
   return true;
}
/**********************************************************************/
int STLGeom::findIntersectionPointsWithSTLGeom(int part,double origin[3],double dir[3],double **intersectionPts, int *intersectingTriangles)
{
    // Assumes that memory has been allocated for intersectionPts
    // intersectionPts[MAXINTERSECTIONS][3]
    // intersectingTriangles[MAXINTERSECTIONS];
    // return the number of intersections
    int i,tri,trinode,nIntersections,duplicateFound;
    double tripts[3][3],u,v,t,pt[3];

    nIntersections = 0;

    for (tri = 0; tri < stl_nTriangles; tri++) {
        if (triPart[tri] != part) continue;
        for (i =0; i <3; i++) {
            trinode = triNodes[3*tri+i];
            tripts[i][0] = triXYZ[3*trinode];
            tripts[i][1] = triXYZ[3*trinode+1];
            tripts[i][2] = triXYZ[3*trinode+2];
        }
        if(rayIntersectTri(origin,dir,tripts[0],tripts[1],tripts[2],&t,&u,&v)) {
            for (i =0; i <3; i++)
                pt[i] = origin[i] + t*dir[i];
            // check if this point has already been detected
            duplicateFound = 0;
            for (i = 0; i < nIntersections; i++) {
                if (DIST3D(pt,intersectionPts[i]) < TOL) {
                    duplicateFound = 1;
                    break;
                }
            }
            if (duplicateFound==0){
                for (i = 0; i < 3; i++)
                    intersectionPts[nIntersections][i] = pt[i];
                intersectingTriangles[nIntersections] = tri;
                nIntersections++;
            }
        }
        if (nIntersections >= MAXINTERSECTIONS-1){
            writeMessageToSTLLog("STLGeom: Max intersections exceeded ");
            break;
        }
    }
    return nIntersections;
}
/**********************************************************************/
int STLGeom::findRayIntersectionSTL(double origin[3],double dir[3],double *tParams)
{
    // Assumes that memory has been allocated for tParams
    int i,tri,trinode,nIntersections,sortIndex[MAXINTERSECTIONS];
    double tripts[3][3],u,v,t;
    bool duplicateFound;

	nIntersections = 0;

	for (tri = 0; tri < stl_nTriangles; tri++) {
		for (i =0; i <3; i++) {
			trinode = triNodes[3*tri+i];
			tripts[i][0] = triXYZ[3*trinode];
			tripts[i][1] = triXYZ[3*trinode+1];
			tripts[i][2] = triXYZ[3*trinode+2];
		}
		if(rayIntersectTri(origin,dir,tripts[0],tripts[1],tripts[2],&t,&u,&v)) {
			// check if this point has already been detected
            duplicateFound = false;
			for (i = 0; i < nIntersections; i++) {
                if (fabs(tParams[i]-t) < TOL) {
                    duplicateFound = true;
					break;
				}
			}
            if (!duplicateFound){
                tParams[nIntersections] = t;
				nIntersections++;
			}
		}
		if (nIntersections >= MAXINTERSECTIONS-1){
            writeMessageToSTLLog("STLGeom: Max intersections exceeded ");
			break;
		}
	}

    for (i =0; i < nIntersections; i++)
        sortIndex[i] = i;
    // now sort the parameters in increasing order
    bubbleSortIncreasing(tParams,sortIndex,nIntersections);

	return nIntersections;
}
/**********************************************************************/
bool STLGeom::isPointIn2DTriangle(double *A,double *B,double *C,double *P, double *u, double *v)
{
	double v0[2],v1[2],v2[2],dot00,dot01,dot02,dot11,dot12,invDenom;

	if (P[0] < MIN(A[0],MIN(B[0],C[0]))) return false;// bounding box checks
	if (P[1] < MIN(A[1],MIN(B[1],C[1]))) return false;// bounding box checks
	if (P[0] > MAX(A[0],MAX(B[0],C[0]))) return false;// bounding box checks
	if (P[1] > MAX(A[1],MAX(B[1],C[1]))) return false;// bounding box checks


	// Compute vectors  
	for (int i = 0; i < 2; i++) {
		v0[i] = C[i] - A[i];
		v1[i] = B[i] - A[i];
		v2[i] = P[i] - A[i];
	}

#define DOT2D(a,b) (a[0]*b[0]+a[1]*b[1])
	// Compute dot products
	dot00 = DOT2D(v0, v0);
	dot01 = DOT2D(v0, v1);
	dot02 = DOT2D(v0, v2);
	dot11 = DOT2D(v1, v1);
	dot12 = DOT2D(v1, v2);

	// Compute barycentric coordinates
	invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
	*u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	*v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	return (*u >= 0) && (*v >= 0) && (*u + *v <= 1);
}
/**********************************************************************/
int STLGeom::findIntersectionPointsWithSTLGeom_ZDirRay(int part,double x,double y,double **intersectionPts, int *intersectingTriangles)
{
	// Assumes that memory has been allocated for intersectionPts
	// intersectionPts[MAXINTERSECTIONS][3]
	// Assumes that the ray direction is along z
	int i,tri,trinode,nIntersections;
	double tripts[3][3],u,v,pt[3];

	nIntersections = 0;

	pt[0] = x;
	pt[1] = y;
	for (tri = triStartEnd[part][0]; tri <= triStartEnd[part][1]; tri++) {
		if (triPart[tri] != part) continue;
		for (i =0; i <3; i++) {
			trinode = triNodes[3*tri+i];
			tripts[i][0] = triXYZ[3*trinode];
			tripts[i][1] = triXYZ[3*trinode+1];
			tripts[i][2] = triXYZ[3*trinode+2];	
		}
		// now check if point lies within triangle
		if(isPointIn2DTriangle(tripts[0],tripts[1],tripts[2],pt,&u,&v)) {
			// P = A + u * (C - A) + v * (B - A)
			pt[2] = tripts[0][2]  + u*(tripts[2][2]-tripts[0][2]) + v*(tripts[1][2]-tripts[0][2]);
			for (i = 0; i < 3; i++)
				intersectionPts[nIntersections][i] = pt[i];
			intersectingTriangles[nIntersections] = tri;
			nIntersections++;
		}
	}
	return nIntersections;
}
/**********************************************************************/
int STLGeom::findIntersectionPointsWithTriangleSet_ZDirRay(int part,double x,double y,IntegerSet &triangleSet,double **intersectionPts, int *intersectingTriangles)
{
	// Assumes that memory has been allocated for intersectionPts
	// intersectionPts[MAXINTERSECTIONS][3]
	// Assumes that the ray direction is along z
	int tri,trinode,nIntersections;
	double tripts[3][3],u,v,pt[3],dist;
	int k,i;
	bool duplicate;

	pt[0] = x;
	pt[1] = y;
	nIntersections = 0;
	for ( k = 0; k < triangleSet.size(); k++) {
		tri = triangleSet.elementAt(k);
		if (triPart[tri] != part) continue;
		for ( i =0; i <3; i++) {
			trinode = triNodes[3*tri+i];
			tripts[i][0] = triXYZ[3*trinode];
			tripts[i][1] = triXYZ[3*trinode+1];
			tripts[i][2] = triXYZ[3*trinode+2];	
		}
		// now check if point lies within triangle
		if(isPointIn2DTriangle(tripts[0],tripts[1],tripts[2],pt,&u,&v)) {
			// P = A + u * (C - A) + v * (B - A)
			pt[2] = tripts[0][2]  + u*(tripts[2][2]-tripts[0][2]) + v*(tripts[1][2]-tripts[0][2]);
			
			// check for duplicates
			duplicate = false;
			for (int jj = 0; jj < nIntersections; jj++) {
				dist = DIST3D(intersectionPts[jj],pt);
				if (dist < TOL)
					duplicate = true;
			}
			if (duplicate) 
				continue;
			// else copy over
			for ( i = 0; i < 3; i++)
				intersectionPts[nIntersections][i] = pt[i];

			intersectingTriangles[nIntersections]  = tri;
			nIntersections++;
			if (nIntersections >= MAXINTERSECTIONS-1){
                writeMessageToSTLLog("STLGeom: Max intersections exceeded ");
				break;
			}
		}
	}
	return nIntersections;
}
/**********************************************************/
void STLGeom::bubbleSortIncreasing(double *numbers, int *sortIndex,int array_size)
{ // sort in increasing order
    int i, j,index;
    double temp;
    
	for (i =0; i < array_size;i++)
		sortIndex[i] = i;

    for (i = (array_size - 1); i > 0; i--){
        for (j = 1; j <= i; j++) {
            if (numbers[j-1] > numbers[j]) {// swap
                temp = numbers[j-1];
                numbers[j-1] = numbers[j];
                numbers[j] = temp;
                index = sortIndex[j-1];
                sortIndex[j-1]= sortIndex[j];
                sortIndex[j] = index;
            }
        }
    }
}
/**********************************************************************/
double STLGeom::distanceOfPointToSTLGeom(double pt[3],double ptClosest[3],int *triClosest)
{
	double x[3],y[3],z[3],pt1[3],pt2[3],pt3[3],d,stu[3],tempPt[3],dMin;
	
	*triClosest = 0;
	dMin = 1e8;
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		getTriPts(tri,x,y,z);
		pt1[0] = x[0];pt1[1] = y[0];pt1[2] = z[0];
		pt2[0] = x[1];pt2[1] = y[1];pt2[2] = z[1];
		pt3[0] = x[2];pt3[1] = y[2];pt3[2] = z[2];
		d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3,tempPt,stu);
		if (d < dMin) {
			dMin = d;
			*triClosest = tri;
			for (int i = 0; i < 3; i++)
				ptClosest[i] = tempPt[i];
		}
	}
	return dMin;
}
/**********************************************************************/
double STLGeom::distanceOfPointToSTLGeomTriGroup(double pt[3],int group,double ptClosest[3])
{
	double x[3],y[3],z[3],pt1[3],pt2[3],pt3[3],d,stu[3],tempPt[3],dMin;

	dMin = 1e8;
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		if (triGroup[tri] != group) continue;
		getTriPts(tri,x,y,z);
		pt1[0] = x[0];pt1[1] = y[0];pt1[2] = z[0];
		pt2[0] = x[1];pt2[1] = y[1];pt2[2] = z[1];
		pt3[0] = x[2];pt3[1] = y[2];pt3[2] = z[2];
		d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3,tempPt,stu);
		if (d < dMin) {
			dMin = d;
			for (int i = 0; i < 3; i++)
				ptClosest[i] = tempPt[i];
		}
	}
	return dMin;
}
//*********************************************
double STLGeom::findRadiusOfCylindricalFace(int faceGroup,double axis[3])
// given an axis direction, find the radius of a potentially cylindrical face
// If the face fails to meet the tolerance of a cylindrical face, return -1
// This is slow since given a face, we have to perform a linear search to
// find all associated triangles
// Does not work for partial cylinder
{
	int i, tri, node, nNodes=0, *faceNodes;
	double xMean = 0.0,yMean=0.0,zMean=0.0,x,y,z, radius,radiusRef, err,vec[3],d;

	faceNodes= (int *)malloc(stl_ntriPoints*sizeof(int));// worst case
	for (i = 0; i < stl_ntriPoints;i++)
		faceNodes[i] = 0;
	
	for (tri = 0; tri < stl_nTriangles; tri++) { 
		if (triGroup[tri] == faceGroup) {
			for (i = 0; i < 3;i++) {
				node = triNodes[3*tri+i];
				xMean += triXYZ[3*node];
				yMean += triXYZ[3*node+1];
				zMean += triXYZ[3*node+2];
				faceNodes[node] = 1;
				nNodes++;
			}
		}
	}
	xMean /= nNodes;// a point on the axis
	yMean /= nNodes;
	zMean /= nNodes;

	radiusRef = -1;
	for (node = 0; node < stl_ntriPoints;node++) {
		if (faceNodes[node] == 1) {
			x = triXYZ[3*node];
			y = triXYZ[3*node+1];
			z = triXYZ[3*node+2];
			vec[0] = x - xMean;// consider the vector from point on axis to (x,y,z) 
			vec[1] = y - yMean;
			vec[2] = z - zMean;
			// project the vector onto the axis
			d = vec[0]*axis[0] +  vec[1]*axis[1] + vec[2]*axis[2];
			// create the perpendicular vector
			vec[0] -= d*axis[0];
			vec[1] -= d*axis[1];
			vec[2] -= d*axis[2];

			radius = NORM3D(vec);
			if (radiusRef == -1){
				radiusRef = radius;
			}
			else {
				err = fabs(radius - radiusRef);
				if (err > radiusRef*0.05) // not a cylinder
					return -1;
			}
		}
	}

	free(faceNodes);
	return radiusRef;
}

//*********************************************
int STLGeom::findDuplicatePt(double pt[3],int partNum)
// find duplicate vertices within a part number
{	
	double temp[3];
	double *ptr;
	int node = -1;

	for (int i = 0; i < stl_ntriPoints;i++) {
		ptr = &(triXYZ[3*i]);
		SUBTRACT3D(temp,pt,ptr);
		if (NORM3D(temp)< TOL){
			node = i;
			break;
		}
	}
	return node;
}
//********************************************
void  STLGeom::getSTLGeomCenter(double center[3])
{
	for (int i=0; i < 3;i++)
		center[i] = stl_STLGeomCenter[i];
}
//********************************************
void  STLGeom::centerSTLGeomModel(void)
{
	int i;
	double range[3],aspect;

	stl_modelScale = 0;
    if (stl_verbose) writeMessageToSTLLog("STLGeom: #Points = %d ",stl_ntriPoints);
    if (stl_verbose) writeMessageToSTLLog("STLGeom: #Triangles = %d ",stl_nTriangles);

	for (i=0; i < 3;i++) {
		range[i] = stl_XYZMax[i] - stl_XYZMin[i];
		stl_STLGeomCenter[i] = (stl_XYZMax[i] + stl_XYZMin[i])/2;
        // due to STL discretization, the geom center may be offset slightly. And this will affect symmetry detection.
        // Correct this
        if (fabs(stl_STLGeomCenter[i]) < 1e-4)// close to origin.
            stl_STLGeomCenter[i] = 0;// assume origin!!
	}



	aspect = MAX(MAX(range[0],range[1]),range[2])/ MIN(MIN(range[0],range[1]),range[2]);
	//stl_modelScale =  0.1*MIN(MIN(range[0],range[1]),range[2]) + 0.9*MAX(MAX(range[0],range[1]),range[2]);

    stl_modelScale =1.6*MAX(MAX(range[0],range[1]),range[2]);
    if (stl_verbose)  writeMessageToSTLLog("STLGeom: stl_modelScale = %e, aspect = %.3f",stl_modelScale,aspect);
    writeMessageToSTLLog("STLGeom: stl_modelScale = %e, aspect = %.3f",stl_modelScale,aspect);
	for (i=0; i < 3;i++) {
		stl_XYZMin[i] += 100*TOL*stl_modelScale;
		stl_XYZMax[i] -= 100*TOL*stl_modelScale;
	}
    if (stl_verbose)writeMessageToSTLLog("STLGeom: XYZ Min = (%.3e, %.3e, %.3e) ",stl_XYZMin[0],stl_XYZMin[1],stl_XYZMin[2]);
    if (stl_verbose)writeMessageToSTLLog("STLGeom: XYZ Max = (%.3e, %.3e, %.3e) ",stl_XYZMax[0],stl_XYZMax[1],stl_XYZMax[2]);

	return;
}
//*********************************************
int STLGeom::getParentPartOfTriangle(int tri)
{
	return triPart[tri];
}
//*********************************************
bool STLGeom::readSTLGeom(const char *fileName, int useGPU, bool assemblyMode,bool (*callbackFunction)(double fractionCompleted),bool eliminateDuplicateNodes)
{
	
    int i,nTrianglesPrevious;
	bool success;
	double area;

	if (assemblyMode)
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Assembly mode " );
	else
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Part mode " );

    if (stl_verbose) writeMessageToSTLLog("STLGeom: #Parts read: %d ",stl_nParts );
    nTrianglesPrevious = 0;
	if ( (!assemblyMode) || (stl_nParts == 0) ) {//reset
		stl_nTriangles = 0;
		stl_ntriPoints = 0;
		stl_nTriGroups = 1;
		stl_nTriFaces = 1;
		for (i=0; i < 3;i++) {
			stl_XYZMin[i] = HUGE_VAL;
			stl_XYZMax[i] = -HUGE_VAL;
			stl_STLGeomCenter[i] = 0;
		}
	}
    else
        nTrianglesPrevious = stl_nTriangles;

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading file %s", fileName);

    // check for .off, .obj or .stl
	if (strstr(fileName,".off") != NULL) {
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading off format ... ");
		success = readOffFile(fileName,stl_nParts);
		if (!success) 
			return false; 
	}
    else if (strstr(fileName,".obj") != NULL) {
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading obj format ... ");
        success = readObjFile(fileName);
        if (!success)
            return false;
    }
	else{ 
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading stl format ... ");
		success = readSTLGeomBinary(fileName,stl_nParts,callbackFunction,eliminateDuplicateNodes);//first try Binary
		if (!success) 
			success = readSTLGeomAscii(fileName,stl_nParts,callbackFunction,eliminateDuplicateNodes);//else try Ascii
		if (!success) 
			return false; //both failed!
	}

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Read %d triangles ",stl_nTriangles);
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Read %d points ",stl_ntriPoints);


    if (stl_nTriangles > MAX_TRIANGLES/5.0)
        eliminateDuplicateNodes = false;// too slow

	if (eliminateDuplicateNodes){
        writeMessageToSTLLog( "Eliminating duplicate nodes ");
        if (!findTriNeighbors(useGPU)){
            writeMessageToSTLLog("STLGeom: Possibly non-manifold geometry  ");
            for(int t1 = nTrianglesPrevious; t1 < stl_nTriangles; t1++)
                triPart[t1] = stl_nParts;
            stl_nParts = 1;// added for handling nonmanifold
            precomputePartProperties();
            stl_nSharpEdges = 0;//
        }
        else {
            autoAssignPartNumberToTriangles();
            precomputePartProperties();
            if (!checkForHollowBodies())
                writeMessageToSTLLog("STLGeom:  Could not resolve hollow bodies ");
            stl_nSharpEdges = 0;
            if (stl_nTriangles < CUTOFF_TRIANGLE_FOR_EDGE_MARKING)
                findSharpEdgesAndCorners(20.0);// use a cutoff angle to find crease edges
        }
	}
	else {// if eliminateDuplicateNodes is false, the user must assign part number to triangles 
        writeMessageToSTLLog("STLGeom: Warning, part numbers not assigned to triangles ");
        stl_nParts = 1;
        for(int t1 =0; t1 < stl_nTriangles; t1++)
            triPart[t1] = 0;
        precomputePartProperties();
	}
	for(int t1 =0; t1 < stl_nTriangles; t1++) {
		area = getAreaOfTriangle(t1);
		if (fabs(area) < TOL)
            writeMessageToSTLLog("STLGeom:  Warning, zero area triangle detected; tri = %d, area = %e ",t1,area);
	}

	
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Min: (%e,%e,%e) ",stl_XYZMin[0],stl_XYZMin[1],stl_XYZMin[2]);
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Max: (%e,%e,%e) ",stl_XYZMax[0],stl_XYZMax[1],stl_XYZMax[2]);


	centerSTLGeomModel();
    if (stl_verbose) writeMessageToSTLLog("STLGeom: STLGeom Center: (%e,%e,%e) ",stl_STLGeomCenter[0],stl_STLGeomCenter[1],stl_STLGeomCenter[2]);
	findStartEndTrianglesOfParts();

    if (stl_verbose) writeMessageToSTLLog("#Triangles = %d ",stl_nTriangles);

	double direction[3];
	if (isGeometrySweepable(direction)) 
        if (stl_verbose) writeMessageToSTLLog("STLGeom: Geometry is sweepable along: (%.1f,%.1f,%.1f) ",direction[0],direction[1],direction[2]);
	
	findSymmetry();
	if (stl_symmetry[0])
        writeMessageToSTLLog("STLGeom: Geometry exhibits symmetry in X direction");
	if (stl_symmetry[1])
        writeMessageToSTLLog("STLGeom: Geometry exhibits symmetry in Y direction");
	if (stl_symmetry[2])
        writeMessageToSTLLog("STLGeom: Geometry exhibits symmetry in Z direction");


    if (stl_nTriangles < CUTOFF_TRIANGLE_FOR_EDGE_MARKING)
        segmentModel();

	return true;
}
//*********************************************
void STLGeom::getSTLSymmetry(bool symmetry[3])
{
	symmetry[0] = stl_symmetry[0];
	symmetry[1] = stl_symmetry[1];
	symmetry[2] = stl_symmetry[2];

}
#ifdef _WIN32
//*********************************************
bool STLGeom::SaveIVBoolToObj(ivbool3d::Object*obj, const char*name)
{
    if (!obj)return false;
    FILE *f = fopen(name, "wt");
    if (f)
    {
        const ivbool3d::Point3f*p32 = NULL;
        const ivbool3d::Point3d*p64 = NULL;

        unsigned count;
        if (!obj->GetPoints(&p32, count))
            obj->GetPointsD(&p64, count);
        unsigned i;
        bool bUV = false, bNormals = false;

        if (p32) {
            for (i = 0; i<count; i++)
            {
                fprintf(f, "v %g %g %g\n", p32->x, p32->y, p32->z);
                p32++;
            }
        }
        else
            if (p64) {
                for (i = 0; i<count; i++)
                {
                    fprintf(f, "v %g %g %g\n", p64->x, p64->y, p64->z);
                    p64++;
                }
            }

        {
            const ivbool3d::p2d*uv = NULL; unsigned count;
            if (obj->GetUV(&uv, count))
            {
                bUV = true;
                for (i = 0; i<count; i++)
                {
                    fprintf(f, "vt %g %g\n", uv->x, uv->y);
                    uv++;
                }
            }
        }
        {
            const ivbool3d::Point3f*n = NULL; unsigned count;
            if (obj->GetNormals(&n, count))
            {
                for (i = 0; i<count; i++)
                {
                    fprintf(f, "vn %g %g %g\n", n->x, n->y, n->z);
                    n++;
                }
                bNormals = true;
            }
        }

        count = obj->GetNumFaces();

        ivbool3d::Face face;
        int lastMtl = -1;
        for (i = 0; i<count; i++)
        {
            obj->GetFace(i, &face);
            if (face.mtlId != lastMtl)
            {
                lastMtl = face.mtlId;
                fprintf(f, "useMtl mtl%d\n", lastMtl);
            }
            fprintf(f, "f");
            for (unsigned j = 0; j<face.size; j++)
            {
                fprintf(f, " %d", face.v[j] + 1);//  +1 obj specific !!!!
                bool _bUV = face.uv0 && bUV;
                if (_bUV)
                    fprintf(f, "/%d", face.uv0[j] + 1);//  +1 obj specific !!!!
                if (face.n && bNormals) {
                    if (!_bUV)fprintf(f, "/");
                    fprintf(f, "/%d", face.n[j] + 1);//  +1 obj specific !!!!
                }

            }
            fprintf(f, "\n");
        }
        fclose(f);
        writeMessageToSTLLog("STLGeom: Saved file %s ", name);
        return true;
    }
    return false;
}
//*********************************************
void STLGeom:: RemoveDuplicatePointsFromIVBool(ivbool3d::Object *master, ivbool3d::Object *slave)
{

    unsigned nVerticesMaster,nVerticesSlave;
    const ivbool3d::Point3d*p64Master = NULL,*p64Slave = NULL,*p64Temp;
    double ptSlave[3],ptMaster[3];
    point3dd *vertices;


    master->GetPointsD(&p64Temp, nVerticesMaster);
    slave->GetPointsD(&p64Slave, nVerticesSlave);
    unsigned i,j;


    vertices = (point3dd *)malloc(nVerticesSlave * sizeof(point3dd));

    for (i = 0; i < nVerticesSlave; i++) {
        ptSlave[0] = p64Slave->x;
        ptSlave[1] = p64Slave->y;
        ptSlave[2] = p64Slave->z;

        vertices[i].x = p64Slave->x;
        vertices[i].y = p64Slave->y;
        vertices[i].z = p64Slave->z;
        p64Master = p64Temp;//start from beginning
        for (j = 0;j < nVerticesMaster; j++) {
            ptMaster[0] = p64Master->x;
            ptMaster[1] = p64Master->y;
            ptMaster[2] = p64Master->z;

            if (DIST3D(ptMaster,ptSlave) < 1e-6){
                vertices[i].x = p64Master->x;
                vertices[i].y = p64Master->y;
                vertices[i].z = p64Master->z;
            }
             p64Master++;
        }
       p64Slave++;
    }

    slave->SetPointsD((ivbool3d::Point3d*)vertices,nVerticesSlave);
}

//*********************************************
bool STLGeom::SaveIVBoolToOffFile(ivbool3d::Object*obj, const char*offFile)
{
    if (!obj)return false;
    FILE *f = fopen(offFile, "wt");
    if (f)
    {
        const ivbool3d::Point3f*p32 = NULL;
        const ivbool3d::Point3d*p64 = NULL;

        fprintf(f,"OFF\n");
        unsigned nVertices,nTriangles;

        if (!obj->GetPoints(&p32, nVertices))
            obj->GetPointsD(&p64, nVertices);
        unsigned i;
        nTriangles = obj->GetNumFaces();
        fprintf(f,"%d %d 0 \n",nVertices,nTriangles);

        if (p32) {
            for (i = 0; i<nVertices; i++)  {
                fprintf(f, "%g %g %g\n", p32->x, p32->y, p32->z);
                p32++;
            }
        }
        else if (p64) {
            for (i = 0; i<nVertices; i++) {
                fprintf(f, "%g %g %g\n", p64->x, p64->y, p64->z);
                p64++;
            }
        }
        ivbool3d::Face face;
        for (i = 0; i<nTriangles; i++) {
            obj->GetFace(i, &face);
            fprintf(f, "3 %d %d %d \n", face.v[0],face.v[1],face.v[2]);
        }

        fclose(f);
        writeMessageToSTLLog("STLGeom: Saved off file %s", offFile);
        return true;
    }
    return false;
}
//*********************************************
bool STLGeom::readAndLoadOffFileToIVBool(const char *fileName, ivbool3d::Object*obj)
{
// Read Off file file (ascii) and load into Object
/*
OFF
14 24 0
0.100000 1.000000 0.000000
0.062349 1.000000 0.078183
-0.022252 1.000000 0.097493
-0.090097 1.000000 0.043388
...
3 0 2 1
3 0 3 2
3 0 4 3
3 0 5 4
3 0 6 5
3 0 1 7
3 1 2 8
...
*/
    double	pt[3];
    int nTriPoints, nTriangles;
    int  tempInt, node1, node2, node3;
    FILE *filein;



    filein = fopen(fileName, "r");
    if (filein == 0) {
        writeMessageToSTLLog("STLGeom: Could not find %s", fileName);
        return false;
    }

    if (find_word(filein, "OFF") == 0) {
        writeMessageToSTLLog("STLGeom:  OFF keyword missing ");
        return false;
    }

    fscanf(filein, "%d %d %d", &(nTriPoints), &(nTriangles), &(tempInt));
    if ((nTriPoints == 0) || (nTriangles == 0)) {
        writeMessageToSTLLog("  Incorrect OFF format ");
        return false;
    }

    point3dd *vertices;

    vertices = (point3dd *)malloc(nTriPoints * sizeof(point3dd));

    for (int node = 0; node < nTriPoints; node++) {
        fscanf(filein, "%lf %lf %lf", &(pt[0]), &(pt[1]), &(pt[2]));
        vertices[node].x = pt[0];
        vertices[node].y = pt[1];
        vertices[node].z = pt[2];
    }

    obj->SetPointsD((ivbool3d::Point3d*)vertices, nTriPoints);

    obj->SetNumFaces(nTriangles);
    unsigned *faces = (unsigned *)malloc(nTriangles * 3 * sizeof(unsigned));


    for (int face = 0; face < nTriangles; face++) {
        fscanf(filein, "%d %d %d %d", &(tempInt), &node1, &node2, &node3);
        if (tempInt != 3) {
            writeMessageToSTLLog("STLGeom: face %d in OFF file has more than 3 vertices ... not allowed ",face);
            continue;
        }
        faces[3 * face] = (unsigned) node1;
        faces[3 * face + 1] = (unsigned)node2;
        faces[3 * face + 2] = (unsigned) node3;
    }

    unsigned size = 3;
    for (unsigned i = 0; i<nTriangles; i++)
    {
        unsigned uvFcae = obj->NewFace(size, faces);
        faces += size;
    }

    fclose(filein);

    writeMessageToSTLLog("Read file %s", fileName);
    return true;
}
//*********************************************
bool  STLGeom::Boolean(const char *offFileA,const char *offFileB,ivbool3d::boolean_mode booleanOperation,const char *resultOffFile)
{
    // Works only on Windows platform since it uses an external .dll
    // given two triangle meshes (in .off format), carry out Boolean operations
    // and dump result in an off file
    // Typical options of booleanOperations are
    // M_OR,M_AND,M_SUB_AB,M_SUB_BA,

    IVBOOLCREATEPROC iv3dCreate = NULL;
    IV3DINIT iv3dInit = NULL;
    IV3DDONE iv3dDone = NULL;
#ifdef _WIN32
    HINSTANCE ivModule = 0;

    HINSTANCE hi = LoadLibraryExA("ivmodule.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (hi)
    {
        iv3dCreate = (IVBOOLCREATEPROC)GetProcAddress(hi, "ivCreateBoolean");
        iv3dInit = (IV3DINIT)GetProcAddress(hi, "iv3dInit");
        iv3dDone = (IV3DDONE)GetProcAddress(hi, "iv3dDone");
        ivModule = hi;
        if (iv3dCreate && iv3dInit && iv3dDone)
        {
            iv3dInit();
        }
        else{
             writeMessageToSTLLog("STLGeom: Could not initialize ivmodule.dll");
             return false;
        }
    }
    else{
        writeMessageToSTLLog("STLGeom: Could not load ivmodule.dll");
        return false;
    }


    ivbool3d::Processor*proc = iv3dCreate();
    ivbool3d::Object*objA = proc->AddSource(0);//A channel
    ivbool3d::Object*objB = proc->AddSource(1);//B channel

    if (!readAndLoadOffFileToIVBool(offFileA, objA)) {
        writeMessageToSTLLog("STLGeom: Could not read file %s ", offFileA);
        return false;
    }
    if (!readAndLoadOffFileToIVBool(offFileB, objB)) {
        writeMessageToSTLLog("STLGeom: Could not read file ", offFileB);
        return false;
    }
    proc->SetReduceProps(1e-6*stl_modelScale, 3.141592653 / 180.0 * 0.01, 1e-6*stl_modelScale);

    if (!proc->Process(booleanOperation, ivbool3d::REDUCE)) {
        writeMessageToSTLLog( "STLGeom: Could not perform Boolean ");
        return false;
    }

    SaveIVBoolToOffFile(proc->GetResult(), resultOffFile);

    proc->Destroy();
    iv3dDone();

    return true;

#else
    writeMessageToSTLLog("STLGeom: Boolean requires a windows specific dll");
    return false;
#endif
}
#endif
/**********************************************************************/
bool STLGeom::saveAsSTL(const char *fileName)
{
    // Write as binary file
    double xTri[3], yTri[3], zTri[3];
    unsigned int nTriangles;
    if (stl_nTriangles == 0) return false;

    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL){
        writeMessageToSTLLog("Could not open fileName %s for writing ",fileName);
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 100;; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    for (int tri = 0; tri < stl_nTriangles; tri++) {
        getTriPts(tri,xTri,yTri,zTri);
        dumpTriangle(xTri,yTri,zTri,fptr);
    }
    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&stl_nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);
    return true;
}
//*********************************************
bool STLGeom::saveAsOff(const char *fileName)
// Save as Off file file (ascii)
/*
OFF
14 24 0
0.100000 1.000000 0.000000
0.062349 1.000000 0.078183
-0.022252 1.000000 0.097493
-0.090097 1.000000 0.043388
...
3 0 2 1
3 0 3 2
3 0 4 3
3 0 5 4
3 0 6 5
3 0 1 7
3 1 2 8
...
   */
{
    double pt[3];
    int nodes[3];

    FILE *fptr = fopen(fileName,"w");

    if (!fptr) return false;
    fprintf(fptr,"OFF \n");
    fprintf(fptr,"%d %d 0 \n",getNumTriPoints(),getNumTriangles());

    for (int i = 0; i < getNumTriPoints(); i++){
        getTriNode(i,pt);
        fprintf(fptr,"%.8e %.8e %.8e \n",pt[0],pt[1],pt[2]);
    }
    for (int i = 0; i < getNumTriangles(); i++){
        getTriNodes(i,nodes);
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);
    }
    fclose(fptr);

    return true;
}
//*********************************************
bool STLGeom::savePartsAsOff(const char *fileName,std::vector<int> parts)
// Save as Off file file (ascii)
/*
OFF
14 24 0
0.100000 1.000000 0.000000
0.062349 1.000000 0.078183
-0.022252 1.000000 0.097493
-0.090097 1.000000 0.043388
...
3 0 2 1
3 0 3 2
3 0 4 3
3 0 5 4
3 0 6 5
3 0 1 7
3 1 2 8
...
   */
{
    double pt[3];
    int nodes[3];

    if (parts.size() == 0) return false;

    if (parts.size() == 1) return saveAsOff(fileName);

    FILE *fptr = fopen(fileName,"w");

    if (!fptr) return false;
    // find all triangles that belong to parts selected

    fprintf(fptr,"OFF \n");
    fprintf(fptr,"%d %d 0 \n",getNumTriPoints(),getNumTriangles());

    for (int i = 0; i < getNumTriPoints(); i++){
        getTriNode(i,pt);
        fprintf(fptr,"%.8e %.8e %.8e \n",pt[0],pt[1],pt[2]);
    }
    for (int i = 0; i < getNumTriangles(); i++){
        getTriNodes(i,nodes);
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);
    }
    fclose(fptr);

    return true;
}
//*********************************************
bool STLGeom::readOffFile(const char *fileName, int partNum)
// Read Off file file (ascii)
/*
OFF
14 24 0
0.100000 1.000000 0.000000
0.062349 1.000000 0.078183
-0.022252 1.000000 0.097493
-0.090097 1.000000 0.043388
...
3 0 2 1 [color data optional]
3 0 3 2
3 0 4 3
3 0 5 4
3 0 6 5
3 0 1 7
3 1 2 8
...
   */
{
	double	pt[3];
	int  tempInt,node1,node2,node3;
	FILE *filein;

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading OFF file ");

    writeMessageToSTLLog("Reading off file %s",fileName);
	filein = fopen(fileName,"r");
    if (filein == 0) {
        writeMessageToSTLLog("STLGeom: Could not find %s",fileName);
        return false;
    }
	
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Opened OFF file ...");
	if (find_word(filein,"OFF") == 0){
        writeMessageToSTLLog("STLGeom:  OFF keyword missing ");
		return false;
	}

    int nTriangles, nTriPoints,nTriPointsPrevious;
    nTriPointsPrevious = stl_ntriPoints;
    fscanf(filein, "%d %d %d", &(nTriPoints),&(nTriangles),&(tempInt) );
    if ((nTriPoints == 0) || (nTriangles == 0)) {
        writeMessageToSTLLog("STLGeom:  Incorrect OFF format ");
		return false;
	}

    if (nTriPoints > MAX_TRI_PTS){
        writeMessageToSTLLog("STLGeom: #Nodes %d exceeds allowable %d",nTriPoints,MAX_TRI_PTS);
        return false;
    }

    if (nTriangles > MAX_TRIANGLES){
        writeMessageToSTLLog("STLGeom: #Triangles %d exceeds allowable %d",nTriangles,MAX_TRIANGLES);
        return false;
    }
    for (int node = 0; node < nTriPoints; node++) {
        fscanf(filein, "%lf %lf %lf\n", &(pt[0]),&(pt[1]),&(pt[2]) );
        triXYZ[3*stl_ntriPoints] = pt[0];
        triXYZ[3*stl_ntriPoints+1] = pt[1];
        triXYZ[3*stl_ntriPoints+2] = pt[2];
		stl_XYZMin[0] = MIN(stl_XYZMin[0],pt[0]);
		stl_XYZMin[1] = MIN(stl_XYZMin[1],pt[1]);
		stl_XYZMin[2] = MIN(stl_XYZMin[2],pt[2]);

		stl_XYZMax[0] = MAX(stl_XYZMax[0],pt[0]);
		stl_XYZMax[1] = MAX(stl_XYZMax[1],pt[1]);
		stl_XYZMax[2] = MAX(stl_XYZMax[2],pt[2]);


        stl_ntriPoints++;
	}

    char str[512];
    for (int face = 0; face < nTriangles; face++) {
        fgets(str,512,filein);
        sscanf(str,"%d %d %d %d",&(tempInt),&node1,&node2,&node3 );
        if (tempInt != 3) {
            writeMessageToSTLLog("STLGeom: face %d in OFF file has more than %d vertices ... not allowed ",face,tempInt);
            continue;
        }
        triNodes[3*stl_nTriangles] =  nTriPointsPrevious + node1;
        triNodes[3*stl_nTriangles+1] = nTriPointsPrevious + node2;
        triNodes[3*stl_nTriangles+2] = nTriPointsPrevious + node3;
        stl_nTriangles++;
	}
	fclose(filein);

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Finished reading off file ");
	return true;
}
//*********************************************
bool STLGeom::readObjFile(const char *fileName)
// Read obj file file (ascii)

{
    int nTriPointsPrevious,nodes[3];
    double pt[3];
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading .obj file ");

    writeMessageToSTLLog("Reading file %s",fileName);

    // Initialize Loader
    objl::Loader Loader;

    // Load .obj File
    bool loadout = Loader.LoadFile(fileName);
    if (!loadout) {
        writeMessageToSTLLog("STLGeom: Could not find %s",fileName);
        return false;
    }
    nTriPointsPrevious = stl_ntriPoints;
    // Go through each loaded mesh and out its contents
     writeMessageToSTLLog("STLGeom: Could not find %s",fileName);
    for (int i = 0; i < Loader.LoadedMeshes.size(); i++) {
        // Copy one of the loaded meshes to be our current mesh
        objl::Mesh curMesh = Loader.LoadedMeshes[i];
        for (int j = 0; j < curMesh.Vertices.size(); j++){
            pt[0] = curMesh.Vertices[j].Position.X;
            pt[1] = curMesh.Vertices[j].Position.Y;
            pt[2] = curMesh.Vertices[j].Position.Z;

            triXYZ[3*stl_ntriPoints] = pt[0];
            triXYZ[3*stl_ntriPoints+1] = pt[1];
            triXYZ[3*stl_ntriPoints+2] = pt[2];
            stl_XYZMin[0] = MIN(stl_XYZMin[0],pt[0]);
            stl_XYZMin[1] = MIN(stl_XYZMin[1],pt[1]);
            stl_XYZMin[2] = MIN(stl_XYZMin[2],pt[2]);

            stl_XYZMax[0] = MAX(stl_XYZMax[0],pt[0]);
            stl_XYZMax[1] = MAX(stl_XYZMax[1],pt[1]);
            stl_XYZMax[2] = MAX(stl_XYZMax[2],pt[2]);


            stl_ntriPoints++;
            if (stl_ntriPoints > MAX_TRI_PTS-1){
                writeMessageToSTLLog("STLGeom: #Nodes %d exceeds allowable %d",stl_ntriPoints,MAX_TRI_PTS);
                return false;
            }

        }
        for (int j = 0; j < curMesh.Indices.size(); j += 3) {
            nodes[0] = curMesh.Indices[j];
            nodes[1] = curMesh.Indices[j+1];
            nodes[2] = curMesh.Indices[j+2];

            triNodes[3*stl_nTriangles] =  nTriPointsPrevious + nodes[0];
            triNodes[3*stl_nTriangles+1] = nTriPointsPrevious + nodes[1];
            triNodes[3*stl_nTriangles+2] = nTriPointsPrevious + nodes[2];
            stl_nTriangles++;

            if (stl_nTriangles > MAX_TRIANGLES-1){
                writeMessageToSTLLog("STLGeom: #Triangles %d exceeds allowable %d",stl_nTriangles,MAX_TRIANGLES);
                return false;
            }
        }
    }
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Finished reading .obj file ");
    return true;
}

//*********************************************
bool STLGeom::readSTLGeomAscii(const char *fileName, int partNum,bool (*callbackFunction)(double fractionCompleted),bool eliminateDuplicateNodes)
// Read STLGeom file (ascii)
/*
solid Beam
   facet normal 0.000000e+000 -1.000000e+000 -3.491483e-015
      outer loop
         vertex 3.000000e-001 3.491483e-017 0.000000e+000
         vertex 3.000000e-001 0.000000e+000 1.000000e-002
         vertex 0.000000e+000 3.491483e-017 0.000000e+000
      endloop
   endfacet
   */
{
	double	pt[3];
	int  node;
	FILE *filein;

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Attempting ASCII Mode ");

	filein = fopen(fileName,"r");
    if (filein == 0) {
        writeMessageToSTLLog("STLGeom: Could not find %s",fileName);
        return false;
    }
	
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Opened ASCII mode ... ");
	double fractionCompleted;

	while (find_word(filein,"facet")) {
        //if (stl_verbose) writeMessageToSTLLog("STLGeom: Reading triangles %d ",stl_nTriangles);
		fractionCompleted = 1.0*stl_nTriangles/MAX_TRIANGLES; // in ASCII mode, we don't know how many triangles there are
        if ((stl_nTriangles%5000 == 0) && (callbackFunction != NULL)) callbackFunction(fractionCompleted);
		for (int i =0; i < 3; i++) {
			find_word(filein,"vertex");
			fscanf(filein, "%lf %lf %lf", &(pt[0]),&(pt[1]),&(pt[2]) );
			if (eliminateDuplicateNodes)
				node = findDuplicatePt(pt,partNum);// we will need to remove duplicate vertices in order to naviagte through triangles neighbors and identify copnnectred parts
			else
				node = -1;
			if (node < 0) {// no duplicate
				triXYZ[3*stl_ntriPoints] = pt[0];
				triXYZ[3*stl_ntriPoints+1] = pt[1];
				triXYZ[3*stl_ntriPoints+2] = pt[2];
				node = stl_ntriPoints;
				
				stl_ntriPoints++;
				if (stl_ntriPoints > MAX_TRI_PTS-3) {
                    writeMessageToSTLLog("STLGeom is too large; MAX points allowed = %d ",MAX_TRI_PTS);
					fclose(filein);
					return false;
				}
				stl_XYZMin[0] = MIN(stl_XYZMin[0],pt[0]);
				stl_XYZMin[1] = MIN(stl_XYZMin[1],pt[1]);
				stl_XYZMin[2] = MIN(stl_XYZMin[2],pt[2]);
		
				stl_XYZMax[0] = MAX(stl_XYZMax[0],pt[0]);
				stl_XYZMax[1] = MAX(stl_XYZMax[1],pt[1]);
				stl_XYZMax[2] = MAX(stl_XYZMax[2],pt[2]);
			}
			triNodes[3*stl_nTriangles+i] = node;
		}
		//check if the area is finite
		//if (getAreaOfTriangle(stl_nTriangles) != 0)
        stl_nTriangles++;
        if (stl_nTriangles%2000 == 0) writeMessageToSTLLog("STLGeom: Read %d triangles",stl_nTriangles);
		if (stl_nTriangles > MAX_TRIANGLES-3) {
            writeMessageToSTLLog("STLGeom: STLGeom is too large; MAX triangles allowed = %d ",MAX_TRIANGLES);
			fclose(filein);
			return false;
		}
	}
	fclose(filein);
    if (stl_verbose) writeMessageToSTLLog("STLGeom: Closing ASCII Mode ");
	if (stl_nTriangles == 0)
		return false;

	return true;
}
//*********************************************
bool STLGeom::readSTLGeomBinary(const char *fileName,int partNum,bool (*callbackFunction)(double fractionCompleted),bool eliminateDuplicateNodes)
// Read STLGeom file (binary)
/*
UINT8[80] � Header
UINT32 Number of triangles

foreach triangle
REAL32[3] Normal vector
REAL32[3] Vertex 1
REAL32[3] Vertex 2
REAL32[3] Vertex 3
UINT16 Attribute byte count
end*/
{
	int  node;
	FILE *filein;

    if (stl_verbose) writeMessageToSTLLog("STLGeom: Attempting Binary Mode ");

	filein = fopen(fileName,"rb");// binary file
    if (filein == 0) {
        writeMessageToSTLLog("STLGeom: Could not find %s ",fileName);
        return false;
    }
	char header[80];
	unsigned int tempNTriangles;
	unsigned short attribute;
	float pt[3],normal[3];
	double ptDouble[3];

	fread(&header,80*sizeof(byte),1,filein);
	fread(&tempNTriangles,sizeof(unsigned int),1,filein);
	if ((tempNTriangles <= 0) || (tempNTriangles >10* MAX_TRIANGLES)){
        //writeMessageToSTLLog("STLGeom: stl_nTriangles = %d ... binary format incorrect ",tempNTriangles);
		fclose(filein);
		return false;
	}
	
    writeMessageToSTLLog("STLGeom: #triangles = %d", tempNTriangles);
	if (tempNTriangles > MAX_TRIANGLES) {
        writeMessageToSTLLog("STLGeom:  MAX triangles allowed = %d ", MAX_TRIANGLES);
		fclose(filein);
		return false;
	}
	
    if (tempNTriangles > MAX_TRIANGLES/5.0)
        eliminateDuplicateNodes = false;// too slow

	double fractionCompleted;
	for (unsigned int tri = 0; tri < tempNTriangles; tri++) {
		fractionCompleted = 1.0*stl_nTriangles/tempNTriangles;
        if ((tri%5000 == 0) && (callbackFunction != NULL)) callbackFunction(fractionCompleted);
		// read the normal 
		fread(&normal[0],sizeof(float),1,filein);
		fread(&normal[1],sizeof(float),1,filein);
		fread(&normal[2],sizeof(float),1,filein);

		for (int i =0; i < 3; i++) {// read three points
			fread(&pt[0],sizeof(float),1,filein);
			fread(&pt[1],sizeof(float),1,filein);
			fread(&pt[2],sizeof(float),1,filein);
			ptDouble[0] = pt[0]; ptDouble[1] = pt[1]; ptDouble[2] = pt[2];
			if (eliminateDuplicateNodes)
				node = findDuplicatePt(ptDouble,partNum);// we will need to remove duplicate vertices in order to naviagte through triangles neighbors and identify copnnectred parts
			else
				node = -1;
			if (node < 0) {// no duplicate
				triXYZ[3*stl_ntriPoints] = pt[0];
				triXYZ[3*stl_ntriPoints+1] = pt[1];
				triXYZ[3*stl_ntriPoints+2] = pt[2];
				node = stl_ntriPoints;
				stl_ntriPoints++;
				if (stl_ntriPoints > MAX_TRI_PTS-3) {
                    writeMessageToSTLLog("STLGeom is too large; MAX points allowed = %d ",MAX_TRI_PTS);
					fclose(filein);
					return false;
				}
				stl_XYZMin[0] = MIN(stl_XYZMin[0],pt[0]);
				stl_XYZMin[1] = MIN(stl_XYZMin[1],pt[1]);
				stl_XYZMin[2] = MIN(stl_XYZMin[2],pt[2]);
		
				stl_XYZMax[0] = MAX(stl_XYZMax[0],pt[0]);
				stl_XYZMax[1] = MAX(stl_XYZMax[1],pt[1]);
				stl_XYZMax[2] = MAX(stl_XYZMax[2],pt[2]);
			}
			triNodes[3*stl_nTriangles+i] = node;
		}
		fread(&attribute,sizeof(attribute),1,filein);
		//check if the area is finite
		//if (getAreaOfTriangle(stl_nTriangles)!= 0)
		stl_nTriangles++;
        if (stl_nTriangles%MAX(5000,(int(0.05*tempNTriangles))) == 0) writeMessageToSTLLog("STLGeom: Read triangles %d", stl_nTriangles);
	}
	fclose(filein);
    if (stl_verbose) writeMessageToSTLLog("STLGeom: #Triangles = %d,#Nodes = %d ",stl_nTriangles,stl_ntriPoints);

	return true;
}
//*********************************************
void	STLGeom::scaleSTLGeomAboutOrigin(double scale)
{
	// x *= scale;
	// y *= scale;
	// z *= scale;
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
		for (j = 0; j < 3;j++)
			triXYZ[3*i+j] *= scale;
	}

	recomputeSTLGeomLimits();
	precomputePartProperties();
}
//*********************************************
void	STLGeom::scaleSTLGeomAboutOriginAlongX(double scale)
{
	// x *= scale;

    if (fabs(scale - 1) < TOL) return;
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
        triXYZ[3*i] *= scale;// only scale along x
	}

	recomputeSTLGeomLimits();
	precomputePartProperties();
}
//*********************************************
void	STLGeom::scaleSTLGeomAboutOriginAlongY(double scale)
{
	// y *= scale;
    if (fabs(scale - 1) < TOL) return;
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
		triXYZ[3*i+1] *= scale;
	}

	recomputeSTLGeomLimits();
	precomputePartProperties();
}
//*********************************************
void	STLGeom::scaleSTLGeomAboutOriginAlongZ(double scale)
{
    if (fabs(scale - 1) < TOL) return;
	// z *= scale;
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
		triXYZ[3*i+2] *= scale;
	}

	recomputeSTLGeomLimits();
	precomputePartProperties();
}
//*********************************************
void	STLGeom::rotateSTLGeomAboutX(double angleInDeg)
{
	//y = y*cos(theta) - z*sin(theta);
	//z = y*sin(theta) + z*cos(theta);
	int i,j;
	double theta,y,z;

	theta = angleInDeg*3.1415/180.0;
	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
        y = triXYZ[3*i+1];
        z = triXYZ[3*i+2];
		triXYZ[3*i+1] = y*cos(theta) - z*sin(theta);
		triXYZ[3*i+2] = y*sin(theta) + z*cos(theta);
	}
	recomputeSTLGeomLimits();
}
//*********************************************
void	STLGeom::rotateSTLGeomAboutY(double angleInDeg)
{
	//x = x*cos(theta) + z*sin(theta);
	//z = -x*sin(theta) + z*cos(theta);
	int i,j;
	double theta,x,z;

	theta = angleInDeg*3.1415/180.0;
	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
        x = triXYZ[3*i];
        z = triXYZ[3*i+2];
		triXYZ[3*i] = x*cos(theta) + z*sin(theta);
		triXYZ[3*i+2] = -x*sin(theta) + z*cos(theta);
	}
	recomputeSTLGeomLimits();
}
//*********************************************
void	STLGeom::rotateSTLGeomAboutZ(double angleInDeg)
{
	//x = x*cos(theta) - y*sin(theta);
	//y = x*sin(theta) + y*cos(theta);
	int i,j;
	double theta,x,y;

	theta = angleInDeg*3.1415/180.0;
	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) {
        x = triXYZ[3*i];
        y = triXYZ[3*i+1];
		triXYZ[3*i] = x*cos(theta) - y*sin(theta);
		triXYZ[3*i+1] = x*sin(theta) + y*cos(theta);
	}
	recomputeSTLGeomLimits();
}
//*********************************************
void	STLGeom::applyRotationMatrix(double R[3][3])
{
    int i;
    double x,y,z;

    for (i=0; i < 3;i++) {
        stl_XYZMin[i] = 1e12;
        stl_XYZMax[i] =-1e12;
    }
    for (i = 0; i <stl_ntriPoints; i++) {
        x = triXYZ[3*i];
        y = triXYZ[3*i+1];
        z = triXYZ[3*i+2];

        triXYZ[3*i]   = x*R[0][0] +  y*R[0][1] +  z*R[0][2];
        triXYZ[3*i+1] = x*R[1][0] +  y*R[1][1] +  z*R[1][2];
        triXYZ[3*i+2] = x*R[2][0] +  y*R[2][1] +  z*R[2][2];
    }
    recomputeSTLGeomLimits();
}
//*********************************************
int	STLGeom::createLocallyConformingQuadMesh(double pt[3], double meshSize,int numQuadsPerSide,char *fileName)
{
	// project the point on STL
	// Find the local plane and create a quad grid
	// Project quad grid points on STL
	// Create hex mesh from deformed quad grid
	double ptClosest[3],d,xDir[3],yDir[3],zDir[3],nrm,x,y,origin[3],quadPt[3];
	int i,j,triClosest,N;

	d = distanceOfPointToSTLGeom(pt,ptClosest,&triClosest);
	N = numQuadsPerSide;

	origin[0] = ptClosest[0];
	origin[1] = ptClosest[1];
	origin[2] = ptClosest[2];
	getNormalOfTriangle(triClosest,zDir);//outward normal of triangle at origin
	
	nrm = NORM3D(zDir);
	for (i=0;i < 3;i++)
		zDir[i] /= nrm;

	//pick an arbitrary x direction
	xDir[0] = 0.3213;xDir[1] = 0.45323; xDir[2] = -0.2434;
	//remove z component;
	nrm = DOTPROD3D(xDir,zDir);
	for (i=0;i < 3;i++)
		xDir[i] -= nrm*zDir[i];
	
	nrm = NORM3D(xDir);
	for (i=0;i < 3;i++)
		xDir[i] /= nrm;

	// create a y direction
	yDir[0] = zDir[1]*xDir[2] - zDir[2]*xDir[1];
	yDir[1] = -(zDir[0]*xDir[2] - zDir[2]*xDir[0]);
	yDir[2] = zDir[0]*xDir[1] - zDir[1]*xDir[0];
	
    //writeMessageToSTLLog("origin = %f %f %f \n",origin[0],origin[1],origin[2]);
    //writeMessageToSTLLog("xDir = %f %f %f \n",xDir[0],xDir[1],xDir[2]);
    //writeMessageToSTLLog("yDir = %f %f %f \n",yDir[0],yDir[1],yDir[2]);
    //writeMessageToSTLLog("zDir = %f %f %f \n",zDir[0],zDir[1],zDir[2]);
	// Verify the orthonormality of vectors

	// Now create a quad grid
	// total num of points = (2N+1)*(2N+1)
	// total num of quads = 2N*2N

	//  points need to be projected onto the STL, and only the 'good points' are retained
	bool *pointOK = (bool *)malloc((2*N+1)*(2*N+1)*sizeof(bool));
	memset(pointOK,0,(2*N+1)*(2*N+1)*sizeof(bool));
	double triNormal[3];
	int allPointsCounter;
	FILE *fptr = fopen(fileName,"w");
	fprintf(fptr,"                                   \n");
	allPointsCounter = 0;
	for (j = -N; j <= N; j++) {
		y = j*meshSize;//local y
		for (i = -N; i <= N; i++) {
			x = i*meshSize;//local x
			quadPt[0] = origin[0] + x*xDir[0] + y*yDir[0];
			quadPt[1] = origin[1] + x*xDir[1] + y*yDir[1];
			quadPt[2] = origin[2] + x*xDir[2] + y*yDir[2];
			d = distanceOfPointToSTLGeom(quadPt,ptClosest,&triClosest);
			getNormalOfTriangle(triClosest,triNormal);
			nrm = DOTPROD3D(triNormal,zDir);// both vectors are normalized
			if ((d < 30*meshSize) && (nrm > 0.1)) //point can be used
				pointOK[allPointsCounter] = true;
			fprintf(fptr,"%lf %lf %lf \n",ptClosest[0],ptClosest[1],ptClosest[2]);
			allPointsCounter++;
		}
	}
	int nodeOffset,nodes[4],numGoodQuads;
	numGoodQuads = 0;
	for (j = 0; j < 2*N; j++) {
		nodeOffset = j*(2*N+1);
		for (i = 0; i < 2*N; i++) {
			nodes[0] = nodeOffset + i;
			nodes[1] = nodeOffset + i+1;
			nodes[2] = nodeOffset + i+1+(2*N+1);
			nodes[3] = nodeOffset + i+(2*N+1);
			if ( (pointOK[nodes[0]]) && (pointOK[nodes[1]]) &&
				(pointOK[nodes[2]]) && (pointOK[nodes[3]]) ){
				fprintf(fptr,"%d %d %d %d \n",nodes[0],nodes[1],nodes[2],nodes[3]);
				numGoodQuads++;
			}
		}
	}
	rewind(fptr);
	fprintf(fptr,"%d \n",allPointsCounter);
	fprintf(fptr,"%d \n",numGoodQuads);
	fclose(fptr);
	return 1;
}

//*********************************************
void STLGeom::translateSTLGeom(double offset[3])
{
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) 
		for (j = 0; j < 3; j++)
			triXYZ[3*i+j] += offset[j];

	recomputeSTLGeomLimits();
}
//*********************************************
void STLGeom::translateSTLGeombyZ(double distance)
{
	int i,j;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] = 1e12;
		stl_XYZMax[j] =-1e12;
	}
	for (i = 0; i <stl_ntriPoints; i++) 
		triXYZ[3*i+2] += distance;
	recomputeSTLGeomLimits();
}
//*********************************************
void STLGeom::recomputeSTLGeomLimits(void)
{
	int i,j;
	double range[3];

	stl_modelScale =0;
	for (i = 0; i <stl_ntriPoints; i++) {
		stl_XYZMin[0] = MIN(stl_XYZMin[0],triXYZ[3*i]);
		stl_XYZMax[0] = MAX(stl_XYZMax[0],triXYZ[3*i]);

		stl_XYZMin[1] = MIN(stl_XYZMin[1],triXYZ[3*i+1]);
		stl_XYZMax[1] = MAX(stl_XYZMax[1],triXYZ[3*i+1]);

		stl_XYZMin[2] = MIN(stl_XYZMin[2],triXYZ[3*i+2]);
		stl_XYZMax[2] = MAX(stl_XYZMax[2],triXYZ[3*i+2]);
	}

	for (i=0; i < 3;i++) { 
		range[i] = stl_XYZMax[i] - stl_XYZMin[i];
		stl_modelScale = MAX(stl_modelScale,range[i]);
		stl_STLGeomCenter[i] = (stl_XYZMax[i] + stl_XYZMin[i])/2;
	}
	//stl_modelScale /=3.0;

	for (j=0; j < 3;j++) {
		stl_XYZMin[j] += 100*TOL*stl_modelScale;
		stl_XYZMax[j] -= 100*TOL*stl_modelScale;
	}
    if (stl_verbose)writeMessageToSTLLog("STLGeom: XYZ Min = (%.3e, %.3e, %.3e) ",stl_XYZMin[0],stl_XYZMin[1],stl_XYZMin[2]);
    if (stl_verbose)writeMessageToSTLLog("STLGeom: XYZ Max = (%.3e, %.3e, %.3e) ",stl_XYZMax[0],stl_XYZMax[1],stl_XYZMax[2]);
}
//*********************************************
int	STLGeom::setTriangleData(int nPoints, int nTri, int nGroups, double *xyz,int *nodes)
{
	int i,j;

	stl_nTriangles = 0;
	stl_ntriPoints = 0;
	stl_nTriGroups = 1;
	stl_nTriFaces = 1;
	for (i=0; i < 3;i++) {
		stl_XYZMin[i] = 1e20;
		stl_XYZMax[i] = -1e20;
		stl_STLGeomCenter[i] = 0;
	}

	stl_ntriPoints  = nPoints;
	stl_nTriangles = nTri;
	stl_nTriGroups = nGroups;
	for ( i = 0; i < stl_ntriPoints; i++)
		for (j = 0; j < 3; j++)
			triXYZ[3*i+j] = xyz[3*i+j];

	for ( i = 0; i < stl_nTriangles; i++) {
		for (j = 0; j < 3; j++)
			triNodes[3*i+j] = nodes[3*i+j];
	}

	for (i=0; i < 3;i++) {
		stl_XYZMin[i] = 1e20;
		stl_XYZMax[i] = -1e20;
	}
	for (i = 0; i < stl_ntriPoints; i++) {
		for (j = 0; j < 3;j++) {
			stl_XYZMin[j] = MIN(stl_XYZMin[j],triXYZ[3*i+j]);
			stl_XYZMax[j] = MAX(stl_XYZMax[j],triXYZ[3*i+j]);
		}
	}
    if (stl_verbose)  writeMessageToSTLLog("XMin = %e XMax = %e ",stl_XYZMin[0],stl_XYZMax[0] );

	centerSTLGeomModel();
	findSymmetry();
	return 1;
}
/**********************************************************************/
void STLGeom::saveAsObj(char *objFileName)
{
	double pt[3];
	int nodes[3];

	FILE *fptr = fopen(objFileName,"w");

	for (int i = 0; i < getNumTriPoints(); i++){
		getTriNode(i,pt);
		fprintf(fptr,"v %.3e %.3e %.3e \n",pt[0],pt[1],pt[2]);
	}
	for (int i = 0; i < getNumTriangles(); i++){
		getTriNodes(i,nodes);
		fprintf(fptr,"f %d %d %d \n",nodes[0]+1,nodes[1]+1,nodes[2]+1);// starts with 1
	}
	fclose(fptr);
}
/**********************************************************************/
STLGeom::~STLGeom()
{
    if (stl_verbose) writeMessageToSTLLog("STLGeom: deleting object");
}
/**********************************************************************/
bool STLGeom::doBothNodesBelongToTriangle(int nodes[2],int tri )
{
	int *triangleNodes,count;

	triangleNodes = &(triNodes[3*tri]);
	count = 0;

	if ((nodes[0] == triangleNodes[0]) || (nodes[0] == triangleNodes[1]) || (nodes[0] == triangleNodes[2]) )
		count++;

	if ((nodes[1] == triangleNodes[0]) || (nodes[1] == triangleNodes[1]) || (nodes[1] == triangleNodes[2]) )
		count++;
	
	if (count == 2) return true;
	else return false;
}
/**********************************************************************/
bool STLGeom::isGeometrySweepable(double dir[3])
{// check if the geometry is linearly sweepable, if so return the direction (x, y or z)

	double normal[3],dotprod,center[3];
	bool found;

	for (int i = 0; i < 3; i++) {
		if (i == 0) {dir[0] = 1; dir[1] = 0; dir[2] = 0;}
		else if (i == 1) {dir[0] = 0; dir[1] = 1; dir[2] = 0;}
		else {dir[0] = 0; dir[1] = 0; dir[2] = 1;}

		// for x direction:
		// (1) every triangle must either be perpendicular to x or along +/- x
		// (2) the triangles along +x must be on a single plane, and 
		/// (3) the triangles along -x must be on a single plane
		found = true;
		for (int tri = 0; tri < stl_nTriangles; tri++) {
			getNormalOfTriangle(tri,normal);
			dotprod = DOTPROD3D(normal,dir);
			
			if ( (fabs(dotprod) > 0.001) && (fabs(dotprod) < 0.99) ){
				found = false;// triangle is neither perpendicular nor along direction
				break;
			}
			getCenterOfTriangle(tri,center);
			if (dotprod > 0.99) { // along direction
				if (fabs(center[i] - stl_XYZMax[i]) > 0.01*stl_modelScale){
					found = false;// a  triangle other than extreme has same direction
					break;
				}
			}
			else if (dotprod < -0.99){// along -ve direction
				if (fabs(center[i] - stl_XYZMin[i]) > 0.01*stl_modelScale){
					found = false;// a  triangle other than extreme has same direction
					break;
				}
			}
		}
		if (found) return true;
	}
	return false;
}
/**********************************************************************/
int STLGeom::getRecommendedNumberOfElements(void)
{
	double minThickness,volume,areaOfPart,estimatedThickness,volumeOfPart,criticalRadius;
	int nElementsRecommended;

	volume = 0;
	for (int part = 0; part < stl_nParts; part++) {// consider each part separately, find the recommended size
		minThickness = getMinimumThicknessOfPart(part);//based on bounding box
        if (stl_verbose)  writeMessageToSTLLog("STLGeom: part %d, minThickness= %.2e",part,minThickness);
		volumeOfPart = getVolumeOfSTLPart(part);
		areaOfPart = getAreaOfSTLPart(part);
		estimatedThickness = volumeOfPart/areaOfPart;/// based on surface area and volume
		estimatedThickness = MIN(estimatedThickness,minThickness);
		//also consider radius of selected cylindrical faces, if any
		criticalRadius = stl_smallestCylinderRadius;
		estimatedThickness = MIN(estimatedThickness,criticalRadius/2.0);
		volume += volumeOfPart;
	}
	nElementsRecommended = (int) MAX(10000,ceil(volume/pow(estimatedThickness,3.0)));

	
	nElementsRecommended = (int) MIN(nElementsRecommended,50000000);
	return nElementsRecommended;

}
/**********************************************************************/
void STLGeom::dumpTriangle(double xTri[3],double yTri[3],double zTri[3],FILE *fptr)
{
	float fbuf[12];
	unsigned short att = 0;
	double triNormal[3],p01[3],p02[3],nrm;

	p01[0] = xTri[1]-xTri[0];
	p01[1] = yTri[1]-yTri[0];
	p01[2] = zTri[1]-zTri[0];

	p02[0] = xTri[2]-xTri[0];
	p02[1] = yTri[2]-yTri[0];
	p02[2] = zTri[2]-zTri[0];
	CROSS3D(triNormal,p01,p02);

	nrm = NORM3D(triNormal);
	fbuf[0] = (float) (triNormal[0]/nrm) ; 
	fbuf[1] = (float) (triNormal[1]/nrm) ; 
	fbuf[2] = (float) (triNormal[2]/nrm) ;

	fbuf[3]  = (float) (xTri[0]);
	fbuf[4]  = (float) (yTri[0]);
	fbuf[5]  = (float) (zTri[0]);

	fbuf[6]  = (float) (xTri[1]);
	fbuf[7]  = (float) (yTri[1]);
	fbuf[8]  = (float) (zTri[1]);

	fbuf[9]   = (float) (xTri[2]);
	fbuf[10]  = (float) (yTri[2]);
	fbuf[11]  = (float) (zTri[2]);

	fwrite(fbuf, 12*sizeof(float), 1, fptr);
	fwrite(&att,sizeof(unsigned short),1,fptr);
	stl_nTesselatedTriangles++;

}
/**********************************************************************/
void STLGeom::dumpTriangleTesselationRecursive(double xTri[3],double yTri[3],double zTri[3],FILE *fptr,int maxDepth,double edgeLength)
{
	double x1[3],y1[3],z1[3],x2[3],y2[3],z2[3],*x,*y,*z;
	double LMax = -HUGE_VAL,L;
	int nodes[]= {0,1,2,0},n1,n2,index;


	// if sufficient dept, dump
	if (maxDepth <=0) {
		dumpTriangle(xTri,yTri,zTri,fptr);
		return;
	}
	// next find the longest edge
	for (int i = 0; i < 3;i++) {
		n1 = nodes[i];
		n2 = nodes[i+1];
		L = sqrt ( pow((xTri[n2]-xTri[n1]),2.0) + pow((yTri[n2]-yTri[n1]),2.0) +pow((zTri[n2]-zTri[n1]),2.0) );
		if (L > LMax) {
			index = i;
			LMax = L;
		}
	}
	// if sufficiently small, dump
	if (LMax < edgeLength) {// we are done splitting the triangle
		dumpTriangle(xTri,yTri,zTri,fptr);
		return;
	}
	// Split this edge and call self twice, with depth reduced
	x = xTri; y = yTri; z = zTri;
	switch(index) {
		case 0:
			x1[0] = x[0];         y1[0] = y[0];          z1[0] = z[0];
			x1[1] = (x[0]+x[1])/2;y1[1] = (y[0]+y[1])/2; z1[1] = (z[0]+z[1])/2;
			x1[2] = x[2];         y1[2] = y[2];          z1[2] = z[2];

			x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
			x2[1] = x[1]; y2[1] = y[1]; z2[1] = z[1];
			x2[2] = x[2]; y2[2] = y[2]; z2[2] = z[2];

			break;
		case 1:
			x1[0] = x[1];         y1[0] = y[1];         z1[0] = z[1];
			x1[1] = (x[1]+x[2])/2;y1[1] = (y[1]+y[2])/2;z1[1] = (z[1]+z[2])/2;
			x1[2] = x[0];         y1[2] = y[0];         z1[2] = z[0];

			x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
			x2[1] = x[2]; y2[1] = y[2]; z2[1] = z[2];
			x2[2] = x[0]; y2[2] = y[0]; z2[2] = z[0];

			break;
		case 2:
			x1[0] = x[2];         y1[0] = y[2];         z1[0] = z[2];
			x1[1] = (x[2]+x[0])/2;y1[1] = (y[2]+y[0])/2;z1[1] = (z[2]+z[0])/2;
			x1[2] = x[1];         y1[2] = y[1];         z1[2] = z[1];

			x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
			x2[1] = x[0]; y2[1] = y[0]; z2[1] = z[0];
			x2[2] = x[1]; y2[2] = y[1]; z2[2] = z[1];

			break;
	}
	dumpTriangleTesselationRecursive(x1,y1,z1,fptr,maxDepth-1,edgeLength);
	dumpTriangleTesselationRecursive(x2,y2,z2,fptr,maxDepth-1,edgeLength);
}
/**********************************************************************/
bool STLGeom::generatePointsOnSharpEdges(double spacing,vector<Point3d> &points)
{
    double u;
    double deltaU;
    int nodes[2],triParents[2],node;
    double *pt1,*pt2;
    Point3d pt;

    for (int corner =0; corner < stl_nSharpCorners; corner++){
        node = triCorners[corner];
        pt1 = &(triXYZ[3*node]);
        pt.x = pt1[0];
        pt.y = pt1[1];
        pt.z = pt1[2];
        points.push_back(pt);
    }

    for (int edge = 0; edge < stl_nSharpEdges; edge++){
        for (int j = 0; j < 2; j++){
            nodes[j] = triEdges[2*edge+j];
            triParents[j] = triEdgesParents[2*edge+j];
        }
        pt1 = &(triXYZ[3*nodes[0]]);
        pt2 = &(triXYZ[3*nodes[1]]);
        deltaU = spacing/DIST3D(pt1,pt2);

        for (u = deltaU; u < 1.0; u += deltaU){
            pt.x = u*pt1[0] + (1-u)*pt2[0];
            pt.y = u*pt1[1] + (1-u)*pt2[1];
            pt.z = u*pt1[2] + (1-u)*pt2[2];
            points.push_back(pt);
        }
    }
    return true;
}
/**********************************************************************/
bool STLGeom::dumpPointCloudOnSTL(const char *fileName,double edgeLength,int nPointsDesired)
{
    /*
1. Construct a sorted list of normalized area weights for each triangle: (AreaOfTriangle / TotalSurfaceArea).
2. For each point
     - Generate a random number 'r' in [0,1]
     - Walk through the list of weights, accumulating weight until the total is <= r
    - Generate a random point on the surface of the selected triangle. (Via randomized barycentric coordinates).
*/

    vector<pair<double,int>> areasOfTriangle;
    double area,totalArea,rndArea,cumulativeArea,normal[3],xTri[3],yTri[3],zTri[3],pt[3],u,v,w;
    double deltaU,dist,normal1[3],normal2[3];
    double lineVec[3],  normlineVector[3],offsetNormal1[3], offsetNormal2[3];
    double nrm,ptOffset1[3], ptOffset2[3],offsetDist;
    int tri;

    FILE *fptr = fopen(fileName,"w");
    if (fptr == NULL){
        writeMessageToSTLLog("Could not open fileName %s for writing ",fileName);
        return false;
    }
    totalArea = 0;
    for (int i = 0; i < stl_nTriangles; i++){
        area = getAreaOfTriangle(i);
        areasOfTriangle.push_back(make_pair(area,i));
        totalArea += area;
    }
    std::sort (areasOfTriangle.begin(), areasOfTriangle.end());

    fprintf(fptr,"%d\n",nPointsDesired);
    int nodes[2],nEdgePts,triParents[2];
    double *pt1,*pt2;
    nEdgePts = 0;
    offsetDist = stl_modelScale/1e5;
    bool onEdge;
    onEdge = true;
    for (int en = 0; en < stl_nSharpEdges; en++){
        for (int j = 0; j < 2; j++){
            nodes[j] = triEdges[2*en+j];
            triParents[j] = triEdgesParents[2*en+j];
        }
        pt1 = &(triXYZ[3*nodes[0]]);
        pt2 = &(triXYZ[3*nodes[1]]);
        dist = DIST3D(pt1,pt2);
        deltaU = edgeLength/dist;
        getNormalOfTriangle(triParents[0],normal1);
        getNormalOfTriangle(triParents[1],normal2);
        //finding the normalized vector along the length of the line connecting the two points

        for (int i = 0; i <3 ; i++)
            lineVec[i] = pt2[i] - pt1[i];

        nrm = NORM3D(lineVec);
        for (int i = 0; i <3 ; i++)
            normlineVector[i] = lineVec[i]/nrm;

        // cross product between normal of the triangle and the normalized line Vector
        CROSS3D(offsetNormal1, normlineVector, normal1);   //normal along offset direction 1

        nrm = NORM3D(offsetNormal1);
        for (int i = 0; i <3 ; i++)
            offsetNormal1[i] /= nrm;

        // cross product between normal of the triangle and the normalized line Vector
        CROSS3D(offsetNormal2,  normal2,normlineVector);   //normal along offset direction 2
        nrm = NORM3D(offsetNormal2);
        for (int i = 0; i <3 ; i++)
            offsetNormal2[i] /= nrm;

        //offset distance for the points
        for (u = deltaU; u < 1.0; u += deltaU){
            for (int i = 0; i <3 ; i++){
                pt[i] = u*pt1[i] + (1-u)*pt2[i];
                ptOffset1[i] = pt[i] + offsetNormal1[i]*offsetDist;
                ptOffset2[i] = pt[i] + offsetNormal2[i]*offsetDist;
            }
            if (onEdge){
                fprintf(fptr,"%.8e %.8e %.8e %.8e %.8e %.8e\n",pt[0],pt[1],pt[2],normal2[0],normal2[1],normal2[2]);
                nEdgePts++;
            }
            else {
                fprintf(fptr,"%.8e %.8e %.8e %.8e %.8e %.8e\n",ptOffset1[0],ptOffset1[1],ptOffset1[2],normal1[0],normal1[1],normal1[2]);
                nEdgePts++;
                fprintf(fptr,"%.8e %.8e %.8e %.8e %.8e %.8e\n",ptOffset2[0],ptOffset2[1],ptOffset2[2],normal2[0],normal2[1],normal2[2]);
                nEdgePts++;
            }
        }
    }
    for (int i = nEdgePts; i < nPointsDesired; i++){
        rndArea = totalArea*rand()/RAND_MAX;
        cumulativeArea = 0;
        tri = stl_nTriangles-1;
        // find a triangle
        for (int j = 0; j < stl_nTriangles; j++){
            cumulativeArea += areasOfTriangle[j].first;
            if (cumulativeArea >= rndArea){
                tri = areasOfTriangle[j].second;
                break;
            }
        }
        getTriPts(tri,xTri,yTri,zTri);
        // Find random Barycentrix

        // internal sampling
        u = 1.0*rand()/RAND_MAX;
        v = 1.0*rand()/RAND_MAX;
        while (u+v > 1){
            u = 1.0*rand()/RAND_MAX;
            v = 1.0*rand()/RAND_MAX;
        }
        w = (1-u-v);
        pt[0] = u*xTri[0] + v*xTri[1] + w*xTri[2];
        pt[1] = u*yTri[0] + v*yTri[1] + w*yTri[2];
        pt[2] = u*zTri[0] + v*zTri[1] + w*zTri[2];
        getNormalOfTriangle(tri,normal);
        fprintf(fptr,"%.8e %.8e %.8e %.8e %.8e %.8e\n",pt[0],pt[1],pt[2],normal[0],normal[1],normal[2]);
    }


    fclose(fptr);
    return true;
}
/**********************************************************************/
bool STLGeom::dumpSTLTesselation(const char *fileName,int maxDepth,double edgeLength)
{
	// split an STL into triangles smaller triangles for display purposes
	// Write as binary file 
	double xTri[3], yTri[3], zTri[3];
	unsigned int nTriangles;
	if (stl_nTriangles == 0) return false;

    if (stl_verbose)  writeMessageToSTLLog("In dumpSTLTesselation maxDepth= %d, edgeLength = %e",maxDepth,edgeLength);
	stl_nTesselatedTriangles = 0;
	FILE *fptr = fopen(fileName,"wb");
	if (fptr == NULL){
        writeMessageToSTLLog("Could not open fileName %s for writing ",fileName);
		return false;
	}
	char header[80];
	memset(header,'0', 80*sizeof( char));// dummy header
	fwrite(header,80,1, fptr);
	nTriangles = 100;; // dummy, will overwrite later on
	fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		getTriPts(tri,xTri,yTri,zTri);
		dumpTriangleTesselationRecursive(xTri,yTri,zTri,fptr,maxDepth,edgeLength);
	}
	fseek(fptr,80,SEEK_SET);
	// write the number of triangles
	fwrite(&stl_nTesselatedTriangles, sizeof(unsigned int), 1, fptr);
	fclose(fptr);
	return true;
}
/**********************************************************************/
void STLGeom::extractCrossSection(char *fileName,double hSize)
{
	// Extracts the outer loop for the lowest Z value surface of STLGeom
	int tript,tri,*nodes,*neighbors,i,currentNode,startNode,previousNode,tempNode,nEdges;
	int nPointsOnXYPlane,nTrianglesOnXYPlane,**nodesConnected,commonNodes[2],node1,node2,*nodeList;
	double zCoord;
	bool *isPointOnXYPlane, *isTriangleOnXYPlane;

	isPointOnXYPlane = (bool *)malloc(stl_ntriPoints*sizeof(bool));

	// walk thru all nodes and mark the ones of XY plane
	nPointsOnXYPlane = 0;
	for (tript = 0; tript < stl_ntriPoints; tript++) {
		isPointOnXYPlane[tript] = false;
		zCoord = triXYZ[3*tript+2];
		if (fabs(zCoord-stl_XYZMin[2]) < 1000*TOL*stl_modelScale){
			nPointsOnXYPlane++;
			isPointOnXYPlane[tript] = true;
		}
	}

    if (stl_verbose)  writeMessageToSTLLog("nPointsOnXYPlane = %d ",nPointsOnXYPlane);
	isTriangleOnXYPlane = (bool *)malloc(stl_nTriangles*sizeof(bool));
	// walk through all triangles and mark the ones on XY plane
	nTrianglesOnXYPlane = 0;
	for (tri =0; tri < stl_nTriangles; tri++) {
		isTriangleOnXYPlane[tri] = false;
		nodes = &(triNodes[3*tri]);
		if ( (isPointOnXYPlane[nodes[0]]) &&(isPointOnXYPlane[nodes[1]]) &&(isPointOnXYPlane[nodes[2]])){
			isTriangleOnXYPlane[tri] = true;
			nTrianglesOnXYPlane++;
		}
	}
    if (stl_verbose)  writeMessageToSTLLog("nTrianglesOnXYPlane = %d ",nTrianglesOnXYPlane);
	// find all edges of polygon
	// find all triangles on XY plane that share an edge with boundary of polygon on xy plane
	nodesConnected = (int **)malloc(stl_ntriPoints*sizeof(int *));
	for (i = 0; i < stl_ntriPoints;i++){
		nodesConnected[i] = (int *)malloc(2*sizeof(int));
		nodesConnected[i][0] = -1;
		nodesConnected[i][1] = -1;
	}

	// for every node on edge find the two nodes connected to it ... no particular order
	startNode = -1;
	
	for (tri =0; tri < stl_nTriangles; tri++) {
		if (isTriangleOnXYPlane[tri]) {//if triangle is on XY plane
			neighbors = &(triNeighbors[3*tri]);
			for (i = 0; i < 3; i++) {
				if (!isTriangleOnXYPlane[neighbors[i]]) {// but one of its neighbors is not
					findCommonElements(3,&(triNodes[3*tri]), &(triNodes[3*neighbors[i]]),commonNodes);
					// we want to maintain the cyclic ordering
					if ( ((commonNodes[0] == triNodes[3*tri]) && (commonNodes[1] == triNodes[3*tri+1])) || 
						((commonNodes[0] == triNodes[3*tri+1]) && (commonNodes[1] == triNodes[3*tri+2])) ||
						((commonNodes[0] == triNodes[3*tri+2]) && (commonNodes[1] == triNodes[3*tri])) ){
						node1 = commonNodes[0];
						node2 = commonNodes[1];
					}
					else {// reverse
						node2 = commonNodes[0];
						node1 = commonNodes[1];
					}
					nodesConnected[node1][0] = node2;
					nodesConnected[node2][1] = node1;
					startNode = node1;// some start node
				}
			}
		}
	}
    writeMessageToSTLLog("Created nodesConnected");
	nodeList = (int *)malloc(stl_ntriPoints*sizeof(int));
	nEdges = 0;
	
	previousNode = startNode;
	nodeList[nEdges++] = startNode;
	currentNode = nodesConnected[previousNode][1]; 
	nodeList[nEdges++] = currentNode;
	while (1) {
		node1 = nodesConnected[currentNode][0];
		node2 = nodesConnected[currentNode][1];
		tempNode = currentNode;
		if (node1 != previousNode) currentNode = node1;
		else currentNode = node2;
		previousNode = tempNode;
		if (currentNode == startNode) break;
		nodeList[nEdges++] = currentNode;
	}
    writeMessageToSTLLog("Created nodeList");
    writeMessageToSTLLog(" Writing to file: %s ",fileName);
	//write to a file
	FILE *fout;
	fout = fopen(fileName,"w");
	fprintf(fout,"1\n");
	fprintf(fout,"%d \n",nEdges);
	fprintf(fout,"%d \n",nEdges);
	node1 = nodeList[0];
	fprintf(fout,"%e %e %e \n", triXYZ[3*node1],triXYZ[3*node1+1],hSize);
	for (i =1; i < nEdges;i++){
		node1 = nodeList[i];
		fprintf(fout,"%e %e %e \n", triXYZ[3*node1],triXYZ[3*node1+1],hSize);
		fprintf(fout,"lin \n");
	}
	node1 = nodeList[0];
	fprintf(fout,"%e %e %e \n", triXYZ[3*node1],triXYZ[3*node1+1],hSize);
	fprintf(fout,"lin \n");
	fprintf(fout,"lin \n");
	fclose(fout);

    writeMessageToSTLLog("Finished writing to file");
	free(isPointOnXYPlane);
	free(isTriangleOnXYPlane);
	for (i = 0; i < stl_ntriPoints;i++)
		free(nodesConnected[i]);
	free(nodesConnected);
	free(nodeList);
	
}
