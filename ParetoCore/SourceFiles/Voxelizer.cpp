#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> 
#include <fstream> 
#include <iostream> 
#include "CommonDefines.h"
#include "Voxelizer.h"
#ifndef __APPLE__
#include <omp.h>
#endif
#include <Timer.h>
#include <IntegerSet.h>
#include <algorithm>    // std::sort
#include "isosurface.h"
#include <queue.h>
using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)


void findTriNeighborsGPU(int stl_nTriangles,int *triNodes,int *triNeighbors);
extern void writeMessageToSTLLog(const char format[],...);
/**********************************************************************/
Voxelizer::Voxelizer()
{
	vox_verbose = false;

    vox_includeNodesCloseToBoundary = true;
    vox_meshThinStructures = false;
	nElemsConnectedToNode = NULL;
	vox_closestTriangleToGridNode = NULL;
	vox_elemMapping = NULL;
	vox_nodeMapping = NULL;
    vox_nodeValues = NULL;
	vox_maxVoxelsAllowed = 55*80+122;
	vox_partIdOfGridElem = NULL;
    vox_userDefinedGrid = false;
    vox_forceEqualSizeGrid = false;
}
/**********************************************************************/
void Voxelizer::enableVoxVerbose(void)
{
    writeMessageToSTLLog("Mesher: Enabling Mesher Verbose");
	vox_verbose = true;
}
/**********************************************************************/
void Voxelizer::disableVoxVerbose(void)
{
    writeMessageToSTLLog("Mesher: Disabling Mesher Verbose");
	vox_verbose = false;
}
/**********************************************************************/
void Voxelizer::setVoxelGrid(int nelx, int nely, int nelz)
{
    vox_userDefinedGrid = true;
    vox_nels[0] =  nelx;
    vox_nels[1] =  nely;
    vox_nels[2] =  nelz;
}
/**********************************************************************/
void Voxelizer::unsetVoxelGrid(void)
{
    vox_userDefinedGrid = false;

}
/**********************************************************************/
void Voxelizer::setEqualGridSize(void)
{
    vox_forceEqualSizeGrid = true;
}
/**********************************************************************/
void Voxelizer::unsetEqualGridSize(void)
{
    vox_forceEqualSizeGrid = false;

}
/**********************************************************************/
void Voxelizer::copySTLGeometry(Voxelizer &voxIn)
{
	int i,j;

	stl_ntriPoints = voxIn.stl_ntriPoints;
	stl_nTriangles = voxIn.stl_nTriangles;
	stl_nTriGroups = voxIn.stl_nTriGroups;

    writeMessageToSTLLog("Mesher: Copying %d points, %d triangles and %d groups ",stl_ntriPoints,stl_nTriangles,stl_nTriGroups);
	for ( i = 0; i < stl_ntriPoints; i++)
		for (j = 0; j < 3; j++)
			triXYZ[3*i+j] = voxIn.triXYZ[3*i+j];

	for ( i = 0; i < stl_nTriangles; i++)
		for (j = 0; j < 3; j++)
			triNodes[3*i+j] = voxIn.triNodes[3*i+j];

	for ( i = 0; i < stl_nTriangles; i++)
		triGroup[i] = voxIn.triGroup[i];

}
/**********************************************************************/
void Voxelizer::transferClosestTriGroups(Voxelizer &voxRef)
{
	// for every triangle of this object, find the closest triangle of voxRef
	int i,group;
	double centerPt[3],ptClosest[3],dist;

	stl_nTriGroups =  voxRef.stl_nTriGroups;

	for ( i = 0; i < stl_nTriangles; i++){
		getCenterOfTriangle(i,centerPt);
		for (group = 1; group <stl_nTriGroups; group++) {
			dist = voxRef.distanceOfPointToSTLGeomTriGroup(centerPt,group,ptClosest);
			if (dist <0.001*stl_modelScale) {
				triGroup[i] = group;
				break;
			}
		}
	}

    // verify if each group has been transferred to at least one triangle!?
    //verify if the transfer is meaningful (compare area?)
}
/**********************************************************************/
void Voxelizer::setMaxVoxels(int maxVoxels)
{
	vox_maxVoxelsAllowed = maxVoxels;
}
/**********************************************************************/
int Voxelizer::getMaxVoxels(void)
{
	return vox_maxVoxelsAllowed;
}
/**********************************************************************/
double	Voxelizer::closestVoxelElementToPoint(double x,double y,double z,int *voxelElement,double xietamu[3])
{
	int Nx,Ny,Nz,gridElem,nx,ny,nz;
	double xElem,yElem,zElem,d,dMin;
	int nTotalElems = (vox_nels[0])*(vox_nels[1])*(vox_nels[2]);

	// Find the closest voxel element to point
	Nx = (int) NEARESTINT((x-stl_XYZMin[0]-vox_dsize[0]/2)/vox_dsize[0]);
	Ny = (int) NEARESTINT((y-stl_XYZMin[1]-vox_dsize[1]/2)/vox_dsize[1]);
	Nz = (int) NEARESTINT((z-stl_XYZMin[2]-vox_dsize[2]/2)/vox_dsize[2]);
	// This element may not exist in the mesh.
	// We have to search amongst all nearby elements
	// This element may not exist in the mesh.
	// We have to search amongst all nearby elements

	// we have to search amongst nearby elems
	*voxelElement = -1;
	dMin = 10*vox_dsize[0];

	for (nx = MAX(0,Nx-1);nx <= MIN(vox_nels[0]-1,Nx+1);nx++) {
		for (ny = MAX(0,Ny-1);ny <= MIN(vox_nels[1]-1,Ny+1);ny++) {
			for (nz = MAX(0,Nz-1);nz <= MIN(vox_nels[2]-1,Nz+1);nz++) {
				gridElem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
				if (gridElem < 0) continue;
				if (gridElem >= nTotalElems) continue;
				if (vox_elemMapping[gridElem] < 0) continue; 
				if (vox_partIdOfGridElem[gridElem] < 0) continue;
				xElem = stl_XYZMin[0] + nx*vox_dsize[0] + vox_dsize[0]/2;//mid point
				yElem = stl_XYZMin[1] + ny*vox_dsize[1] + vox_dsize[1]/2;
				zElem = stl_XYZMin[2] + nz*vox_dsize[2] + vox_dsize[2]/2;
				d = sqrt( pow(xElem-x,2.) + pow(yElem-y,2.) + pow(zElem-z,2.));
				if (d < dMin) {
					dMin = d;
					*voxelElement = vox_elemMapping[gridElem];
					xietamu[0] = 2*(x-xElem+vox_dsize[0]/2)/vox_dsize[0] - 1;
					xietamu[1] = 2*(y-yElem+vox_dsize[1]/2)/vox_dsize[1] - 1;
					xietamu[2] = 2*(z-zElem+vox_dsize[2]/2)/vox_dsize[2] - 1;
					if (dMin < 0.5*vox_dsize[0]) return dMin;
				}
			}
		}
	}
	return (dMin);
}
/**********************************************************************/
void	Voxelizer::closestGridNodeToPoint(double x,double y,double z,double dMin,int *closestGridNode)
{
	int nx,ny,nz,gridNode;
	double xNode,yNode,zNode,d;
	int nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z

	*closestGridNode = -1;

	nx = (int) NEARESTINT((x-stl_XYZMin[0])/vox_dsize[0]);
	ny = (int) NEARESTINT((y-stl_XYZMin[1])/vox_dsize[1]);
	nz = (int) NEARESTINT((z-stl_XYZMin[2])/vox_dsize[2]);

	gridNode = nx + ny*yOffset + nz*zOffset; 
	if (gridNode < 0) return;
	if (gridNode >= nTotalGridNodes) return;
	xNode = stl_XYZMin[0] + nx*vox_dsize[0];
	yNode = stl_XYZMin[1] + ny*vox_dsize[1];
	zNode = stl_XYZMin[2] + nz*vox_dsize[2];
	d = sqrt( pow(xNode-x,2.) + pow(yNode-y,2.) + pow(zNode-z,2.));
	if (d < dMin) {
		dMin = d;
		*closestGridNode = gridNode;
	}
	return;
}
/**********************************************************************/
bool	Voxelizer::getClosestTriangleToPt(double *pt,int *tri)
{
	int Nx,Ny,Nz,nx,ny,nz,gridNode,triTemp;
	double dMin,dist;

	int nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
	// Find the closest voxel element to point
	Nx = (int) NEARESTINT((pt[0]-stl_XYZMin[0])/vox_dsize[0]);
	Ny = (int) NEARESTINT((pt[1]-stl_XYZMin[1])/vox_dsize[1]);
	Nz = (int) NEARESTINT((pt[2]-stl_XYZMin[2])/vox_dsize[2]);

	// We have to search amongst all nearby elements
	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z
	dMin = HUGE_VAL;
	// search amongst nearby nodes
	*tri = -1;//invalid
	for (nx = Nx-1;nx <= Nx+1;nx++) {
		for (ny = Ny-1;ny <= Ny+1;ny++) {
			for (nz = Nz-2;nz <= Nz+1;nz++) {
				gridNode = nx + ny*yOffset + nz*zOffset; // start gridnode number for a given hex
				if (gridNode < 0) continue;
				if (gridNode >= nTotalGridNodes) continue;
				if (vox_nodeMapping[gridNode] < 0) continue;// not a valid grid node
				triTemp = vox_closestTriangleToGridNode[gridNode];
				if (triTemp >= 0) {
					dist = distanceOfPointToTri(pt,triTemp);
					if (dist < dMin) {
						*tri = triTemp;
						dMin = dist;
					}

				}
			}
		}
	}
	if (*tri >= 0)
		return true;
	else
		return false;
}

/**********************************************************************/
double	Voxelizer::closestVoxelNodeToPoint(double x,double y,double z,int *voxelNode)
{
	int Nx,Ny,Nz,nx,ny,nz,gridNode;
	double dMin,xNode,yNode,zNode,d;
	int nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
	// Find the closest voxel element to point
	Nx = (int) NEARESTINT((x-stl_XYZMin[0])/vox_dsize[0]);
	Ny = (int) NEARESTINT((y-stl_XYZMin[1])/vox_dsize[1]);
	Nz = (int) NEARESTINT((z-stl_XYZMin[2])/vox_dsize[2]);

	// We have to search amongst all nearby elements
	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z
	int nodeIndex[8] = {0,1,yOffset+1,yOffset,zOffset,zOffset+1,zOffset+yOffset+1,zOffset+yOffset};

	// we have to search amongst nearby nodes
	*voxelNode = -1;
	dMin = getModelScale();

	for (nx = Nx-1;nx <= Nx+1;nx++) {
		for (ny = Ny-1;ny <= Ny+1;ny++) {
			for (nz = Nz-2;nz <= Nz+1;nz++) {
				gridNode = nx + ny*yOffset + nz*zOffset; // start gridnode number for a given hex
				if (gridNode < 0) continue;
				if (gridNode >= nTotalGridNodes) continue;
				if (vox_nodeMapping[gridNode] < 0) continue;
				xNode = stl_XYZMin[0] + nx*vox_dsize[0];
				yNode = stl_XYZMin[1] + ny*vox_dsize[1];
				zNode = stl_XYZMin[2] + nz*vox_dsize[2];
				d = sqrt( pow(xNode-x,2.) + pow(yNode-y,2.) + pow(zNode-z,2.));
				if (d < dMin) {
					dMin = d;
					*voxelNode = vox_nodeMapping[gridNode];
					if (dMin < 0.8*vox_dsize[0]) return dMin;
				}
			}
		}
	}
	return (dMin);
}
/**********************************************************************/
int Voxelizer::getElemFromGridElem(int gridElem)
{
	if ((gridElem < 0) || (gridElem >= vox_nels[0]*vox_nels[1]*vox_nels[2])){
        writeMessageToSTLLog("Mesher: Error in getElemFromGridElem; gridElem = %d ",gridElem);
		return -1;
	}
	return vox_elemMapping[gridElem];
}
/**********************************************************************/
void Voxelizer::getNumGridElems(int grid[3])
{
	grid[0] = vox_nels[0];
	grid[1] = vox_nels[1];
	grid[2] = vox_nels[2];
}
/**********************************************************************/
void Voxelizer::getVoxelSize(double size[3])
{
	size[0] = vox_dsize[0];
	size[1] = vox_dsize[1];
	size[2] = vox_dsize[2];
}
/**********************************************************************/
int Voxelizer::find_word(FILE *filein,char *word)
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
double Voxelizer::distanceOfPointFromLine3D(double *Pt,double *Pt1,double *Pt2,double *PtClosest)
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
void Voxelizer::allocateMemory(int nTotalGridNodes, int nTotalGridElems)
{
	if (nElemsConnectedToNode!= NULL) free(nElemsConnectedToNode);
	nElemsConnectedToNode = (int *)malloc(nTotalGridNodes*sizeof(int));
	memset(nElemsConnectedToNode,0,nTotalGridNodes*sizeof(int));

	if (vox_closestTriangleToGridNode != NULL) free(vox_closestTriangleToGridNode);
	vox_closestTriangleToGridNode = (int *)malloc(nTotalGridNodes*sizeof(int));
	memset(vox_closestTriangleToGridNode,-1,nTotalGridNodes*sizeof(int));

	if (vox_elemMapping != NULL) free(vox_elemMapping);
	vox_elemMapping = (int *)malloc(nTotalGridElems*sizeof(int));
	memset(vox_elemMapping,-1,nTotalGridElems*sizeof(int)); // invalid by default

	if (vox_nodeMapping != NULL) free(vox_nodeMapping);
	vox_nodeMapping = (int *)malloc(nTotalGridNodes*sizeof(int));
	memset(vox_nodeMapping,-1,nTotalGridNodes*sizeof(int));//  grid node is invalid (lies outside)

    if (vox_nodeValues != NULL) free(vox_nodeValues);
    vox_nodeValues = (double *)malloc(nTotalGridNodes*sizeof(double));
    memset(vox_nodeValues,0,nTotalGridNodes*sizeof(double));

	if (vox_partIdOfGridElem!= NULL) free(vox_partIdOfGridElem);
	vox_partIdOfGridElem = (int *)malloc(nTotalGridElems*sizeof(int));
	for (int i =0; i <nTotalGridElems; i++){
		vox_partIdOfGridElem[i] = -1;
	}
}
/**********************************************************************/
int Voxelizer::findAllNonAccessibleTriangles(int dir)
{
	// given a direction, say z direction, find all triangles that cannot be accessed for machining.
	int n1,n2,n1Max,n2Max,ntri,*intersectionTris,i,*sortIndex;
	double origin[3],rayDir[3],**intersectionPts,*alphaIntersects;


	if (dir == 0) return 0;

	intersectionTris = (int *)malloc(MAXINTERSECTIONS*sizeof(int));
	intersectionPts = (double **)malloc(MAXINTERSECTIONS*sizeof(double *));
	for (i = 0;i < MAXINTERSECTIONS; i++) 
		intersectionPts[i] = (double *)malloc(3*sizeof(double));

	alphaIntersects = (double *)malloc(MAXINTERSECTIONS*sizeof(double));
	sortIndex = (int *)malloc(MAXINTERSECTIONS*sizeof(int));


	origin[0] =  stl_XYZMin[0] - (stl_XYZMax[0]-stl_XYZMin[0]); 
	origin[1] =  stl_XYZMin[1] - (stl_XYZMax[1]-stl_XYZMin[1]); 
	origin[2] =  stl_XYZMin[2] - (stl_XYZMax[2]-stl_XYZMin[2]); 
	rayDir[0] = 0; rayDir[1] = 0; rayDir[2] = 0;

	int part = 0;
	if (dir == 1) { // x direction
		n1Max = vox_nels[1];
		n2Max = vox_nels[2];
		rayDir[0] = 1;
	}
	else if (dir == 2) {
		n1Max = vox_nels[0];
		n2Max = vox_nels[2];
		rayDir[1] = 1;
	}
	else if (dir == 3) {
		n1Max = vox_nels[0];
		n2Max = vox_nels[1];
		rayDir[2] = 1;
	}
	for (n1 = 0; n1 <= n1Max; n1++) {
		for (n2 = 0; n2 <= n2Max; n2++) {
			if (dir == 1) {
				origin[1] = n1*vox_dsize[1] + stl_XYZMin[1];
				origin[2] = n2*vox_dsize[2] + stl_XYZMin[2];
			}
			else if (dir == 2) {
				origin[0] = n1*vox_dsize[0] + stl_XYZMin[0];
				origin[2] = n2*vox_dsize[2] + stl_XYZMin[2];
			}
			if (dir == 3) {
				origin[0] = n1*vox_dsize[0] + stl_XYZMin[0];
				origin[1] = n2*vox_dsize[1] + stl_XYZMin[1];
			}
			ntri = findIntersectionPointsWithSTLGeom(part,origin,rayDir,intersectionPts,intersectionTris);

			if (ntri == 0) continue;
			// copy over the Z values
			for (i =0; i < ntri; i++) 
				alphaIntersects[i] = intersectionPts[i][dir-1];

			// and sort 
			bubbleSortIncreasing(alphaIntersects,sortIndex,ntri);
			// skip the first and last triangles
			for (i = 1; i < ntri-1;i++)
				triHighlight[intersectionTris[sortIndex[i]]] = true;
		}
	}

	for (i = 0;i < MAXINTERSECTIONS; i++) 
		free(intersectionPts[i]);
	free(intersectionPts);
	free(intersectionTris);
	free(sortIndex);
	free(alphaIntersects);
	return 1;
}


/**********************************************************************/
void Voxelizer::MarkNodesRecursive(double x[3],double y[3],double z[3],double dMin,double maxEdgeLength,int currentDepth, int *gridNodesInsidePart)
	/// makr nodes of the voxel

{
	int i,gridNode;
	double x1[3],y1[3],z1[3],x2[3],y2[3],z2[3],center[3],LMax = -1e8,L;;
	int nodes[]= {0,1,2,0},n1,n2,index;

	//  center
	center[0] = (x[0]+x[1]+x[2])/3;
	center[1] = (y[0]+y[1]+y[2])/3;
	center[2] = (z[0]+z[1]+z[2])/3;
	closestGridNodeToPoint(center[0],center[1],center[2],dMin,&(gridNode));
	if ((gridNode != -1)) gridNodesInsidePart[gridNode] = 1;

	if (currentDepth == 0) return;
	// find the longest edge
	for (i = 0; i < 3;i++) {
		n1 = nodes[i];
		n2 = nodes[i+1];
		L = sqrt ( pow((x[n2]-x[n1]),2.0) + pow((y[n2]-y[n1]),2.0) +pow((z[n2]-z[n1]),2.0) );
		if (L > LMax) {
			index = i;
			LMax = L;
		}
	}
	if (LMax < maxEdgeLength)  return;

	// Split this edge and call self twice, with depth reduced
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
	closestGridNodeToPoint(x1[1],y1[1],z1[1],dMin,&(gridNode));
	if ((gridNode != -1)) gridNodesInsidePart[gridNode] = 1;

	MarkNodesRecursive(x1,y1,z1,dMin,maxEdgeLength,currentDepth-1,gridNodesInsidePart);
	MarkNodesRecursive(x2,y2,z2,dMin,maxEdgeLength,currentDepth-1,gridNodesInsidePart);
}
/**********************************************************************/
void Voxelizer::imposeSymmetry(int symmetryIndex,int *gridNodesInsidePart)
{
	/*symmetryIndex =0, means no symmetry
	1 = x-symmetry
	2 = y-symmetry
	3 = x & y symmetry
	4 = z-symmetry
	5 = x & z symmetry
	6 = y & z symmetry
	7 = x, y & z symmetry*/
	int xSym,ySym,zSym,nx,ny,nz,gridnode,otherGridNode,value,nxBar,nyBar,nzBar;

	if (symmetryIndex == 0) return;
	zSym = symmetryIndex/4;
	ySym = (symmetryIndex - zSym*4)/2;
	xSym = (symmetryIndex - zSym*4 -ySym*2);

	if (vox_verbose) {
        if (xSym) writeMessageToSTLLog("Mesher: Imposing X symmetry ");
        if (ySym) writeMessageToSTLLog("Mesher: Imposing Y symmetry ");
        if (zSym) writeMessageToSTLLog("Mesher: Imposing Z symmetry ");
	}
	for (nx = 0; nx <= vox_nels[0]; nx++) {
		for (ny = 0; ny <= vox_nels[1]; ny++) {
			for (nz = 0; nz <= vox_nels[2]; nz++) {
				gridnode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nx;
				if (xSym) {
					nxBar = vox_nels[0] - nx;
					otherGridNode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nxBar;
					value = max(gridNodesInsidePart[gridnode],gridNodesInsidePart[otherGridNode]);
					gridNodesInsidePart[gridnode] = value;
					gridNodesInsidePart[otherGridNode] = value;
				}
				if (ySym) {
					nyBar = vox_nels[1] - ny;
					otherGridNode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + nyBar*(vox_nels[0]+1) + nx;
					value = max(gridNodesInsidePart[gridnode],gridNodesInsidePart[otherGridNode]);
					gridNodesInsidePart[gridnode] = value;
					gridNodesInsidePart[otherGridNode] = value;
				}
				if (zSym) {
					nzBar = vox_nels[2] - nz;
					otherGridNode = nzBar*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nx;
					value = max(gridNodesInsidePart[gridnode],gridNodesInsidePart[otherGridNode]);
					gridNodesInsidePart[gridnode] = value;
					gridNodesInsidePart[otherGridNode] = value;
				}
			}
		}
	}
}
/**********************************************************************/
bool Voxelizer::getGridNodeIndices(int gridNode,int gridNodeIndices[3])
{
    int nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
    if (gridNode < 0) return false;
    if (gridNode >= nTotalGridNodes) return false;

    int nx,ny,nz,tempInt;
    //gridnode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nx;
    // we have to backwards
    nz = int(gridNode/((vox_nels[1]+1)*(vox_nels[0]+1)));
    tempInt = gridNode - nz*(vox_nels[1]+1)*(vox_nels[0]+1);
    ny = int(tempInt/(vox_nels[0]+1));
    nx = tempInt - ny*(vox_nels[0]+1);
    gridNodeIndices[0] = nx;
    gridNodeIndices[1] = ny;
    gridNodeIndices[2] = nz;
    return true;
}
/**********************************************************************/
bool Voxelizer::getLocationOfGridNode(int gridNode,double gridPt[3])
{
    int nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
    if (gridNode < 0) return false;
    if (gridNode >= nTotalGridNodes) return false;
    int nx,ny,nz,tempInt;
    //gridnode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nx;
    // we have to backwards
    nz = int(1.0*gridNode/((vox_nels[1]+1)*(vox_nels[0]+1)));
    tempInt = gridNode - nz*(vox_nels[1]+1)*(vox_nels[0]+1);
    ny = int(1.0*tempInt/(vox_nels[0]+1));
    nx = tempInt - ny*(vox_nels[0]+1);
    gridPt[0] = nx*vox_dsize[0] + stl_XYZMin[0];//x coord of grid point
    gridPt[1] = ny*vox_dsize[1] + stl_XYZMin[1];//y coord of grid point
    gridPt[2] = nz*vox_dsize[2] + stl_XYZMin[2];//z coord of grid point
    return true;

}
/**********************************************************************/
void Voxelizer::includeNodesCloseToTriangles(int part,double dMin, int *gridNodesInside)
{
    int centerNode,otherNode;
    double triCenter[3],otherPt[3],dist;
    int nTotalGridNodes,currentNode;
    bool *nodeVisited;
    Queue Q;
    int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
    int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z

    int gridNodeOffsets[] = {0,1,-1,-yOffset,-yOffset+1,-yOffset-1,+yOffset,+yOffset+1,+yOffset-1,
                         zOffset,1+zOffset,-1+zOffset,-yOffset+zOffset,-yOffset+1+zOffset,-yOffset-1+zOffset,+yOffset+zOffset,+yOffset+1+zOffset,+yOffset-1+zOffset,
                        -zOffset,1-zOffset,-1-zOffset,-yOffset-zOffset,-yOffset+1-zOffset,-yOffset-1-zOffset,+yOffset-zOffset,+yOffset+1-zOffset,+yOffset-1-zOffset};


    // process triangles
    nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
    nodeVisited = (bool *)malloc(nTotalGridNodes*sizeof(bool));
    Q = CreateQueue(nTotalGridNodes+10);
    for (int tri =triStartEnd[part][0]; tri <= triStartEnd[part][1]; tri++) {
        if (triPart[tri] != part) continue;
        getCenterOfTriangle(tri,triCenter);
        closestGridNodeToPoint(triCenter[0],triCenter[1],triCenter[2],dMin,&centerNode);
        if (centerNode < 0) continue;
        gridNodesInside[centerNode] = 1;
        MakeEmpty(Q);
        Enqueue(centerNode, Q); // add node
        memset(nodeVisited,false,nTotalGridNodes*sizeof(bool));
        while (!IsEmpty(Q)) {
            currentNode = Front(Q);
            Dequeue(Q);
            nodeVisited[currentNode] = true;
            for (int i = 0; i < 27; i++){
                otherNode = currentNode + gridNodeOffsets[i];
                if (otherNode < 0) continue;
                if (otherNode >= nTotalGridNodes) continue;
                if (nodeVisited[otherNode]) continue;
                if (gridNodesInside[otherNode]==1) continue;
                nodeVisited[otherNode] = true;
                getLocationOfGridNode(otherNode,otherPt);
                dist = distanceOfPointToTri(otherPt,tri);
                if (dist > dMin) continue;
                gridNodesInside[otherNode] = 1;
                Enqueue(otherNode, Q); // add node

            }
        }
    }
    free(nodeVisited);
    DisposeQueue(Q);

}
/**********************************************************************/
void Voxelizer::markGridNodeCloseToBoundary(int part, double maxEdgeLength,int *gridNodesInside,bool considerAllTriangles)
{
	int i,tri,trinode,maxDepth,gridNode;
	double x[3],y[3],z[3],dMin;

    maxDepth = 10;
	dMin = 0.7*vox_dsize[0];
	for (tri =triStartEnd[part][0]; tri <= triStartEnd[part][1]; tri++) {
		if ((!considerAllTriangles) && (triGroup[tri] == 0)) continue;//only fixed and force triangles are critical if not all triangles need to be considered
		if (triPart[tri] != part) continue;
		for (i =0; i <3; i++) {
			trinode = triNodes[3*tri+i];
			x[i] = triXYZ[3*trinode];
			y[i] = triXYZ[3*trinode+1];
			z[i] = triXYZ[3*trinode+2];
		}
		for (i = 0; i < 3; i++) {//mark the three corners
			closestGridNodeToPoint(x[i],y[i],z[i],dMin,&(gridNode));
			if ((gridNode != -1)) gridNodesInside[gridNode] = 1;
		}
		MarkNodesRecursive(x,y,z,dMin,maxEdgeLength,maxDepth,gridNodesInside);
	}
}
/**********************************************************************/
double Voxelizer::distanceOfGridNodeToSTL(int nx,int ny,int nz,double ptClosest[3])
{
	double pt[3];
    int tri;

	pt[0] = nx*vox_dsize[0] + stl_XYZMin[0];//x coord of grid point
	pt[1] = ny*vox_dsize[1] + stl_XYZMin[1];//y coord of grid point
	pt[2] = nz*vox_dsize[2] + stl_XYZMin[2];//z coord of grid point
    return (distanceOfPointToSTL(pt,ptClosest,&tri));

}
/**********************************************************************/
double Voxelizer::distanceOfPointToSTL(double pt[3], double ptClosest[3], int *closestTri)
{
	double x[3],y[3],z[3],pt1[3],pt2[3],pt3[3],d,stu[3],tempPt[3],dMin;

	dMin = 1e8;
    *closestTri = 0;
	for (int tri = 0; tri < stl_nTriangles; tri++) {
		getTriPts(tri,x,y,z);
		pt1[0] = x[0];pt1[1] = y[0];pt1[2] = z[0];
		pt2[0] = x[1];pt2[1] = y[1];pt2[2] = z[1];
		pt3[0] = x[2];pt3[1] = y[2];pt3[2] = z[2];
		d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3,tempPt,stu);
		if (d < dMin) {
			dMin = d;
            *closestTri = tri;
			for (int i = 0; i < 3; i++)
				ptClosest[i] = tempPt[i];
		}
	}
	return dMin;
}
/**********************************************************************/
double Voxelizer::distanceOfPointToSTLTriGroup(double pt[3],int group,double ptClosest[3])
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
/**********************************************************************/
void Voxelizer::markBoundaryConditions( double LModel)
{
	double x[3],y[3],z[3],pt1[3],pt2[3],pt3[3],d,stu[3],ptClosest[3],pt[3];
	int i,gridnode,nx,ny,nz,triPrevious,groupPrevious;



	double gridH = 0.75*MAX(vox_dsize[2],MAX(vox_dsize[0],vox_dsize[1]));
	// find all boundary nodes close to triangles with boundary conditions 
	double bBox[3][2];
	int nStart[3],nEnd[3];
	for (int tri = 0; tri < stl_nTriangles; tri++) {
        //if (triGroup[tri] == 0) continue;// don't check distance to these triangles
		getTriPts(tri,x,y,z);
		// find the boundary box of triangle
		bBox[0][0] = MIN(MIN(x[0],x[1]),x[2])-TOL*LModel;
		bBox[1][0] = MIN(MIN(y[0],y[1]),y[2])-TOL*LModel;
		bBox[2][0] = MIN(MIN(z[0],z[1]),z[2])-TOL*LModel;

		bBox[0][1] = MAX(MAX(x[0],x[1]),x[2])+TOL*LModel;
		bBox[1][1] = MAX(MAX(y[0],y[1]),y[2])+TOL*LModel;
		bBox[2][1] = MAX(MAX(z[0],z[1]),z[2])+TOL*LModel;

		pt1[0] = x[0];pt1[1] = y[0];pt1[2] = z[0];
		pt2[0] = x[1];pt2[1] = y[1];pt2[2] = z[1];
		pt3[0] = x[2];pt3[1] = y[2];pt3[2] = z[2];

		// find the start and end node locations in the grid
		for (i = 0; i < 3; i++) {
			nStart[i] = (int) NEARESTINT((bBox[i][0] - stl_XYZMin[i])/vox_dsize[i])-1;
			nStart[i] = MIN(MAX(nStart[i],0),vox_nels[i]);

			nEnd[i] = (int) NEARESTINT((bBox[i][1] - stl_XYZMin[i])/vox_dsize[i])+1;
			nEnd[i] = MIN(MAX(nEnd[i],0),vox_nels[i]);
		}
		
		for (nz = nStart[2]; nz <= nEnd[2]; nz++) {
			pt[2] = nz*vox_dsize[2] + stl_XYZMin[2];//z coord of grid point
			for (ny = nStart[1]; ny <= nEnd[1]; ny++) {
				pt[1] = ny*vox_dsize[1] + stl_XYZMin[1];
				for (nx = nStart[0]; nx <= nEnd[0]; nx++) {
					pt[0] = nx*vox_dsize[0] + stl_XYZMin[0];
					gridnode = nz*(vox_nels[1]+1)*(vox_nels[0]+1) + ny*(vox_nels[0]+1) + nx;
					triPrevious = vox_closestTriangleToGridNode[gridnode];
					groupPrevious = -1;
					if (triPrevious >=0) 
						groupPrevious = triGroup[triPrevious];
					if (groupPrevious > 0) continue; // already assigned to a relevant BC
					if (nElemsConnectedToNode[gridnode] < 8){//Boundary node
						d = distanceOfPointFromTriangle3D(pt,pt1,pt2,pt3,ptClosest,stu);
						if (d < gridH) {
							vox_closestTriangleToGridNode[gridnode] = tri;
						}
					}
				}
			}
		}
	}
}

/**********************************************************************/
bool Voxelizer::setGridSize(int nElemsDesired,double maxAspectRatio)
{
	double Lx,Ly,Lz,volume,h,LModel;
	double area,eta[3],hSize[3],fillRatio;

    volume = getVolumeOfSTLGeom(&area);
    Lx = (stl_XYZMax[0]-stl_XYZMin[0]);
    Ly = (stl_XYZMax[1]-stl_XYZMin[1]);
    Lz = (stl_XYZMax[2]-stl_XYZMin[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: Lx,Ly,Lz (%e,%e,%e)",Lx,Ly,Lz);


    if (!vox_userDefinedGrid) {// user has not forced a grid
        nElemsDesired = min(nElemsDesired,vox_maxVoxelsAllowed);
        fillRatio = volume/Lx*Ly*Lz;
        h = pow(volume/nElemsDesired,1./3.0);// cube-root of ..
        if (vox_verbose) writeMessageToSTLLog("Mesher: h = %e",h);
        LModel = max(max(Lx,Ly),Lz);

        if ((Lx > Ly) && (Lx > Lz)) {
            eta[0] = 1.0;
            eta[1] = MAX(Ly/Lx,1./maxAspectRatio);// between 0 and 1
            eta[2] = MAX(Lz/Lx,1./maxAspectRatio);

            hSize[0] = pow(volume/nElemsDesired/(eta[0]*eta[1]*eta[2]),1./3.0);// cube-root of
            hSize[1] = hSize[0]*eta[1];
            hSize[2] = hSize[0]*eta[2];
        }
        else if ((Ly > Lx) && (Ly > Lz)) {
            eta[1] = 1.0;
            eta[0] = MAX(Lx/Ly,1./maxAspectRatio);// between 0 and 1
            eta[2] = MAX(Lz/Ly,1./maxAspectRatio);

            hSize[1] = pow(volume/nElemsDesired/(eta[0]*eta[1]*eta[2]),1./3.0);// cube-root of
            hSize[0] = hSize[1]*eta[0];
            hSize[2] = hSize[1]*eta[2];
        }
        else if ((Lz > Lx) && (Lz > Lx)) {
            eta[2] = 1.0;
            eta[0] = MAX(Lx/Lz,1./maxAspectRatio);// between 0 and 1
            eta[1] = MAX(Ly/Lz,1./maxAspectRatio);

            hSize[2] = pow(volume/nElemsDesired/(eta[0]*eta[1]*eta[2]),1./3.0);// cube-root of
            hSize[0] = hSize[2]*eta[0];
            hSize[1] = hSize[2]*eta[1];
        }
        else {
            hSize[0] = h;
            hSize[1] = h;
            hSize[2] = h;
        }
        vox_nels[0] = (int) MAX(NEARESTINT(Lx/hSize[0]),2);
        vox_nels[1] = (int) MAX(NEARESTINT(Ly/hSize[1]),2);
        vox_nels[2] = (int) MAX(NEARESTINT(Lz/hSize[2]),2);
    }

	//for (int i = 0; i < 3; i++)	vox_nels[i] = MIN(vox_nels[i],512);
	

	vox_dsize[0] = Lx/vox_nels[0];
	vox_dsize[1] = Ly/vox_nels[1];
	vox_dsize[2] = Lz/vox_nels[2];
    if (vox_forceEqualSizeGrid){
        double maxSize;
        maxSize = MAX(MAX(vox_dsize[0],vox_dsize[1]),vox_dsize[2]);
        vox_dsize[0] = vox_dsize[1] = vox_dsize[2] = maxSize;
    }
	
    writeMessageToSTLLog("Mesher: Background Grid (%d,%d,%d)",vox_nels[0],vox_nels[1],vox_nels[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: Background Grid (%d,%d,%d)",vox_nels[0],vox_nels[1],vox_nels[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: Min (%f,%f,%f)",stl_XYZMin[0],stl_XYZMin[1],stl_XYZMin[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: Max (%f,%f,%f)",stl_XYZMax[0],stl_XYZMax[1],stl_XYZMax[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: #voxels (%d,%d,%d)",vox_nels[0],vox_nels[1],vox_nels[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: voxel size (%f,%f,%f)",vox_dsize[0],vox_dsize[1],vox_dsize[2]);

	return true;
}
/**********************************************************************/
void Voxelizer::swap(double *v, int *index, int i, int j)
{
	double temp;
	int t;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;

	t = index[i];
	index[i] = index[j];
	index[j] = t;
}
/**********************************************************************/
void Voxelizer::quickSort(double *v, int *sortIndex,int s, int n)
{
	// sort in increasing order
	int  p, i;
	double x;

	if (n < 1) return;
	// pick pivot and swap with first element
	x = v[s + n/2];
	swap(v, sortIndex,s, s + n/2);
	// partition slice starting at s+1
	p = s;
	for (i = s+1; i < s+n; i++)
		if (v[i] < x) {
			p++;
			swap(v, sortIndex,i, p);
		}
	// swap pivot into place
	swap(v, sortIndex,s, p);
	// recurse into partition
	quickSort(v, sortIndex,s, p-s);
	quickSort(v, sortIndex,p+1, s+n-p-1);
}
/**********************************************************************/
void    Voxelizer::set_includeNodesCloseToBoundary(bool value)
{
    vox_includeNodesCloseToBoundary = value;
}
/**********************************************************************/
void    Voxelizer::set_ThinStructure(bool value)
{
    vox_meshThinStructures = value;
}
/**********************************************************************/
bool Voxelizer::voxelize( int nElemsDesired,double maxAspectRatio,int symmetryIndex,bool (*callBackFunction)(double fractionCompleted))
{
	// Cast rays to voxelize

	double pt[3],LModel;
	int i,nx,ny,nz,nSegments,seg,startZIndex,endZIndex,gridnode,gridelem;
	int nTotalGridNodes,nTotalGridElems,nIntersectionPts,*sortIndex,*intersectionTris,*nNodesInGridElement;
	int node,startNode;
	double origin[3],rayDir[3],**intersectionPts,*zIntersects;
	Timer timer;
	bool everyPartHasAtLeastOneElement;

	
	if (!setGridSize(nElemsDesired,maxAspectRatio))
		return false;

    vox_userDefinedGrid = false;// return to default for safe behavior
	LModel = stl_modelScale;
	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z
	int nodeIndex[8] = {0,1,yOffset+1,yOffset,zOffset,zOffset+1,zOffset+yOffset+1,zOffset+yOffset};

	nTotalGridNodes = (vox_nels[0]+1)*(vox_nels[1]+1)*(vox_nels[2]+1);
	nTotalGridElems = vox_nels[0]*vox_nels[1]*vox_nels[2];

    writeMessageToSTLLog("Mesher: Grid: (%d, %d, %d) ",vox_nels[0],vox_nels[1],vox_nels[2]);

	nNodesInGridElement = (int *)malloc(nTotalGridElems*sizeof(int));
	memset(nNodesInGridElement,0,nTotalGridElems*sizeof(int)); 


	// Assume that the ray direction is z (not optimal)
	// for each node is (x,y) grid, find the intersection of ray with STL
	// for each triangle trace a ray

	intersectionTris = (int *)malloc(MAXINTERSECTIONS*sizeof(int));
	intersectionPts = (double **)malloc(MAXINTERSECTIONS*sizeof(double *));
	for (i = 0;i < MAXINTERSECTIONS; i++) 
		intersectionPts[i] = (double *)malloc(3*sizeof(double));

	zIntersects = (double *)malloc(MAXINTERSECTIONS*sizeof(double));
	sortIndex = (int *)malloc(MAXINTERSECTIONS*sizeof(int));
	rayDir[0] = 0;
	rayDir[1] = 0;
	rayDir[2] = 1;// z direction
	origin[2] = stl_XYZMin[2] -(stl_XYZMax[2]-stl_XYZMin[2]); // z plane is below the body	
    if (vox_verbose) writeMessageToSTLLog("Mesher: Allocating memory for %d nodes and %d elems ",nTotalGridNodes,nTotalGridElems);
	allocateMemory( nTotalGridNodes, nTotalGridElems);

	int *gridNodeInsidePart = (int *)malloc(nTotalGridNodes*sizeof(int));
	vox_nElemsInside =0;
	int nGridNodesInside = 0;
	int nGridNodesInsidePart = 0;
	int currentWorkDone;
	double fractionCompleted;
	
	
	IntegerSet **gridTriangleIndices;// for each grid point, this contains the set of triangles the grid ray intersects
	gridTriangleIndices = (IntegerSet **)malloc((vox_nels[0]+1)*sizeof(IntegerSet *));
	for (nx = 0; nx <= vox_nels[0]; nx++) {
		gridTriangleIndices[nx] = (IntegerSet *)malloc((vox_nels[1]+1)*sizeof(IntegerSet));
		for (ny = 0; ny <= vox_nels[1]; ny++) 
			gridTriangleIndices[nx][ny].init();
	}

	for (int tri = 0; tri < stl_nTriangles; tri++){
		double xTri[3],yTri[3],zTri[3],xMin,yMin,xMax,yMax;
		int nxStart,nxEnd,nyStart,nyEnd;
		getTriPts(tri,xTri,yTri,zTri);
		xMin = MIN(xTri[0],MIN(xTri[1],xTri[2]));
		yMin = MIN(yTri[0],MIN(yTri[1],yTri[2]));
		xMax = MAX(xTri[0],MAX(xTri[1],xTri[2]));
		yMax = MAX(yTri[0],MAX(yTri[1],yTri[2]));
		
		nxStart = MAX(0,MIN(vox_nels[0],NEARESTINT((xMin-stl_XYZMin[0])/vox_dsize[0])));
		nxEnd = MAX(0,MIN(vox_nels[0],NEARESTINT((xMax-stl_XYZMin[0])/vox_dsize[0])));

		nyStart = MAX(0,MIN(vox_nels[1],NEARESTINT((yMin-stl_XYZMin[1])/vox_dsize[1])));
		nyEnd = MAX(0,MIN(vox_nels[1],NEARESTINT((yMax-stl_XYZMin[1])/vox_dsize[1])));
		for (nx = nxStart; nx <= nxEnd; nx++)
			for (ny = nyStart; ny <= nyEnd; ny++)
				gridTriangleIndices[nx][ny].insertNoCheckForUnique(tri);
	}
	
	timer.start();

	// we will mesh them in the increase order of volume. This will ensure that smaller parts don't get missed out
	double partVolume[MAX_STL_PARTS];
	int partSortIndex[MAX_STL_PARTS];
	for (int part =0; part < getNumParts(); part++) {
		partVolume[part] = getVolumeOfSTLPart(part);
		partSortIndex[part] = part;
	}
	quickSort(partVolume,partSortIndex,0, getNumParts()-1);
	int part;
	for (int index =0; index < getNumParts(); index++) {
		part = partSortIndex[index];
		// for each part, create voxels
		memset(gridNodeInsidePart,0,nTotalGridNodes*sizeof(int));//for every part, initialize
		nGridNodesInsidePart = 0;
        writeMessageToSTLLog("Mesher: Meshing part %d",part);
		for (nx = 0; nx <= vox_nels[0]; nx++) {//grid location in x
			currentWorkDone = vox_nels[0]*part + nx;
			fractionCompleted = 0.5*currentWorkDone/(getNumParts()*vox_nels[0]);
            if (int(fractionCompleted*100)%10 == 0)
                if(!callBackFunction(fractionCompleted)) return false;// helps monitor progress
			for (ny = 0; ny <= vox_nels[1]; ny++) {

				if (gridTriangleIndices[nx][ny].size() == 0) continue;
				origin[0] = nx*vox_dsize[0] + stl_XYZMin[0];
				origin[1] = ny*vox_dsize[1] + stl_XYZMin[1];

                nIntersectionPts = findIntersectionPointsWithTriangleSet_ZDirRay(part,origin[0],origin[1],gridTriangleIndices[nx][ny],intersectionPts,intersectionTris);
                // If in the first attempt, we hit no triangles, then we exit since this is a ray lying outside the geometry
                if (nIntersectionPts == 0) continue;// no intersections

                if (nIntersectionPts%2 != 0) {// odd number of intersections, either missing, overlapping or duplicate triangles
                    //writeMessageToSTLLog("Mesher: Attempting local search to find even intersections");
                    int nAttemptsPerSearchArea,nSearchAreas;
                    nAttemptsPerSearchArea = 5;
                    nSearchAreas = 10;

                    for (int searchArea = 0; searchArea < nSearchAreas; searchArea++) {
                        double innerRadius,theta,radius;
                        innerRadius = searchArea*vox_dsize[0];//search in larger circles
                        for (int attempt = 0;attempt <  nAttemptsPerSearchArea; attempt++){// search multiple times in each circle
                            radius = innerRadius + 1.0*vox_dsize[0]*rand()/RAND_MAX;// between two circles
                            theta = 2*3.1415*rand()/RAND_MAX;
                            origin[0] = nx*vox_dsize[0] + stl_XYZMin[0] + radius*cos(theta);
                            origin[1] = ny*vox_dsize[1] + stl_XYZMin[1] + radius*sin(theta);
                            nIntersectionPts = findIntersectionPointsWithTriangleSet_ZDirRay(part,origin[0],origin[1],gridTriangleIndices[nx][ny],intersectionPts,intersectionTris);
                            if((nIntersectionPts > 0) && (nIntersectionPts%2 == 0))break;// successful even number of intersections
                        }
                        if((nIntersectionPts > 0) && (nIntersectionPts%2 == 0))break;// successful even number of intersections
                    }
                }


                if (nIntersectionPts%2 != 0) { // still unsuccessful despite local searches
                    cout << "Mesher: Warning!! Unable to fix meshing problems" << endl;
                    writeMessageToSTLLog("Mesher: Warning!! Unable to fix meshing problems");
                    continue;
				}

				// copy over the Z values
				for (i =0; i < nIntersectionPts; i++) 
					zIntersects[i] = intersectionPts[i][2];

				// and sort the z values
				bubbleSortIncreasing(zIntersects,sortIndex,nIntersectionPts);
				nSegments = nIntersectionPts/2;
				for (seg =0; seg < nSegments;seg++) {
					startZIndex = (int) NEARESTINT((zIntersects[2*seg] - stl_XYZMin[2])/vox_dsize[2]);
					endZIndex = (int) NEARESTINT((zIntersects[2*seg+1]-stl_XYZMin[2])/vox_dsize[2]);
					startZIndex = MIN(MAX(startZIndex,0),vox_nels[2]);
					endZIndex = MIN(MAX(endZIndex,0),vox_nels[2]);
					for (nz = startZIndex; nz <= endZIndex; nz++){
						gridnode = nx + ny*yOffset + nz*zOffset; // start node number for a given hex
						if (gridNodeInsidePart[gridnode] != 1) {
							gridNodeInsidePart[gridnode] = 1;
							nGridNodesInside++;
							nGridNodesInsidePart++;
						}
					}
				}
			}
		}
        if (vox_verbose) writeMessageToSTLLog("Mesher: Making elements close to boundary ");
		// We might miss intersections. So, use recursive sampling of boundary triangles to insert voxel nodes
        if (vox_includeNodesCloseToBoundary)
            markGridNodeCloseToBoundary(part,0.5*vox_dsize[0],gridNodeInsidePart,true);// consider all triangles of part

        if (vox_meshThinStructures)
            includeNodesCloseToTriangles(part,vox_dsize[0],gridNodeInsidePart);

		// Problem with imposing symmetry for multiple part numbers is that part numbers can be messed up
		if (getNumParts() == 1)
			imposeSymmetry(symmetryIndex,gridNodeInsidePart);

		// next assign a rho value for each elem
		//Also, mark nodes as inside that belong to an inside element 

		everyPartHasAtLeastOneElement = false;
        //writeMessageToSTLLog("Mesher: Node assignment completed; time = %.3f (secs) ",timer.getElapsedTimeInSec());
		for (nz = 0; nz < vox_nels[2]; nz++) {
			currentWorkDone = vox_nels[2]*part + nz;
			fractionCompleted = 0.5 +0.5*currentWorkDone/(getNumParts()*vox_nels[2]);
			if(!callBackFunction(fractionCompleted)) return false;// helps monitor progress
			for (ny = 0; ny < vox_nels[1]; ny++) {
				for (nx = 0; nx < vox_nels[0]; nx++) {
					startNode = nx + ny*yOffset + nz*zOffset; // start node number for a given hex
					gridelem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
					if (vox_partIdOfGridElem[gridelem] >= 0) continue;//already assigned
					for (int m = 0; m < 8; m++) {
						node = startNode + nodeIndex[m];
						if (gridNodeInsidePart[node])// inside  part
							nNodesInGridElement[gridelem]++;// every node contributes
                    }

                    if (nNodesInGridElement[gridelem] > 4) {// means element is deemed inside (barring an optional check below)
                        if (nTotalGridElems < 1000) {// for small number of elements, add an additional check
                            double centerOfGridElement[3];
                            centerOfGridElement[0] = (nx+0.5)*vox_dsize[0] + stl_XYZMin[0];
                            centerOfGridElement[1] = (ny+0.5)*vox_dsize[1] + stl_XYZMin[1];
                            centerOfGridElement[2] = (nz+0.5)*vox_dsize[2] + stl_XYZMin[2];
                            if (!isPointInsidePart(centerOfGridElement,part)) continue;
                        }
                        vox_elemMapping[gridelem] = vox_nElemsInside;
						vox_nElemsInside++;
						vox_partIdOfGridElem[gridelem] = part;
						everyPartHasAtLeastOneElement= true;
						for (int m = 0; m < 8; m++) {
							node = startNode + nodeIndex[m];
							nElemsConnectedToNode[node]++;
						}
					}
				}
			}
		}
        //writeMessageToSTLLog("Mesher: #Elems: %d ",vox_nElemsInside);
		if (!everyPartHasAtLeastOneElement) {
            writeMessageToSTLLog("Mesher: Insufficient elements for part %d ",part);
		}
	}

	for (nx = 0; nx <= vox_nels[0]; nx++) {
		for (ny = 0; ny <= vox_nels[1]; ny++) 
			gridTriangleIndices[nx][ny].clear();
		free(gridTriangleIndices[nx]);
	}
	free(gridTriangleIndices);

	vox_nNodesInside = 0;
	if (vox_nElemsInside == 0) {
		free(nNodesInGridElement);
		return false;
	}
    writeMessageToSTLLog("Mesher: #Elements: %d ",vox_nElemsInside);
	gridnode = 0;
	// walk through all nodes, get node mapping, mark boundary nodes and boundary conditions
	//double gridH = sqrt(vox_dsize[0]*vox_dsize[0]+vox_dsize[1]*vox_dsize[1]+vox_dsize[2]*vox_dsize[2]);

	double gridMin[3],gridMax[3];
	for ( i = 0; i < 3; i++) {
		gridMin[i] = HUGE_VAL;
		gridMax[i] = -HUGE_VAL;
	}
	for (nz = 0; nz <= vox_nels[2]; nz++) {
		pt[2] = nz*vox_dsize[2] + stl_XYZMin[2];
		for (ny = 0; ny <= vox_nels[1]; ny++) {
			pt[1] = ny*vox_dsize[1] + stl_XYZMin[1];
			for (nx = 0; nx <= vox_nels[0]; nx++) {
				pt[0] = nx*vox_dsize[0] + stl_XYZMin[0];
				if (nElemsConnectedToNode[gridnode]>0) {// at least one element is connected to gridnode
					vox_nodeMapping[gridnode] = vox_nNodesInside; //mapping to valid gridnode number
					for ( i = 0; i < 3; i++) {
						gridMin[i] = min(gridMin[i],pt[i]);
						gridMax[i] = max(gridMax[i],pt[i]);
					}
					vox_nNodesInside++;
				}
				gridnode++;
			}
		}
	}
    writeMessageToSTLLog("Mesher: #Nodes: %d ",vox_nNodesInside);

    if (vox_verbose) writeMessageToSTLLog("Mesher: GridMin (%f,%f,%f)",gridMin[0],gridMin[1],gridMin[2]);
    if (vox_verbose) writeMessageToSTLLog("Mesher: GridMax (%f,%f,%f)",gridMax[0],gridMax[1],gridMax[2]);

	markBoundaryConditions(LModel);
    if (vox_verbose) writeMessageToSTLLog("Mesher: voxelization complete; time = %.3f (secs) ",timer.getElapsedTimeInSec());;
	free(sortIndex);
	free(zIntersects);
	for (i = 0;i < MAXINTERSECTIONS; i++) 
		free(intersectionPts[i]);
	free(intersectionPts);
	free(gridNodeInsidePart);
	free(intersectionTris);
	free(nNodesInGridElement);

	return true;

}
//*********************************************
double  Voxelizer::getVoxelVolume(void)
{
	double elemSize;
	elemSize = vox_dsize[0]*vox_dsize[1]*vox_dsize[2];
	return(elemSize*vox_nElemsInside);
}
//*********************************************
bool Voxelizer::isElemInside(int elx,int ely,int elz)
{
    int gridElem;
    gridElem = elz*(vox_nels[1]*vox_nels[0]) + ely*(vox_nels[0]) + elx;
    if (vox_partIdOfGridElem[gridElem] >= 0)
        return true;
    else
        return false;
}

//*********************************************
void Voxelizer::dumpVoxelData(const char *fileName)
{
	/*
	Print the following
	% 1. #Grid
	% 2. nelx nely nelz
	% 3. #Origin
	% 4. X0 Y0 Z0
	% 5. #Step
	% 6. dx dy dz
	% 7. #Nodes
	% 8. h_nNodes
	% 9. (i,j,k,f) of all nodes (i ranges from 0 to nelx, etc), f is the facegroup
	% 10. #Elems
	% 11. h_nElems
	% 12. (n1,n2,...,n8) of all elements
	Optionally ...
	% 13. #PartId
	% 14 (p0,p1,p2,....) for each element, the material id, default is 0 
	Optionally ...
	% 15. #PseudoDensity
	% 16. 1,1,0,1,0,....

	CAVEAT:
	We assume a 0-index, i.e., it is assumed that all gridnode numbers, element numbers, and 
	templateIDs start from 0.
	*/


	int i,gridnode,nx,ny,nz,startNode,nDirichletDOF,nNeumannDOF,nOutputDOF,faceGroup,gridelem,tri;
	int nNodesFaceGroup[STL_MAX_GROUPS];

	for (i =0; i < STL_MAX_GROUPS; i++)
		nNodesFaceGroup[i] = 0;


	FILE *fptr = fopen(fileName,"w");

	if (fptr == NULL){
        writeMessageToSTLLog("Could not open file %s ",fileName);
		return;
	}
	fprintf(fptr,"#Grid\n");
	fprintf(fptr,"%d %d %d\n",vox_nels[0],vox_nels[1],vox_nels[2]);


	fprintf(fptr,"#Origin\n");
	fprintf(fptr,"%0.14e %0.14e %0.14e\n",stl_XYZMin[0],stl_XYZMin[1],stl_XYZMin[2]);

	fprintf(fptr,"#Step\n");
	fprintf(fptr,"%0.14e %0.14e %0.14e\n",vox_dsize[0],vox_dsize[1],vox_dsize[2]);

	fprintf(fptr,"#Nodes\n");
	fprintf(fptr,"%12d\n",vox_nNodesInside);
	gridnode = 0;
	nDirichletDOF = 0;
	nNeumannDOF = 0;
	nOutputDOF = 0;
	for (nz = 0; nz < vox_nels[2]+1; nz++) {
		for (ny = 0; ny < vox_nels[1]+1; ny++) {
			for (nx = 0; nx < vox_nels[0]+1; nx++) {
				if (nElemsConnectedToNode[gridnode]>0) {// at least one element is connected to it
					tri = vox_closestTriangleToGridNode[gridnode];
					faceGroup = triGroup[tri];
					fprintf(fptr,"%12d %12d %12d %12d\n",nx,ny,nz,faceGroup);
					if (nElemsConnectedToNode[gridnode]<8) {//boundary gridnode
						nNodesFaceGroup[faceGroup]++;
					}
				}
				gridnode++;
			}
		}
	}
	fprintf(fptr,"#Elems\n");
	fprintf(fptr,"%12d\n",vox_nElemsInside);

	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z
	int nodeIndex[8] = {0,1,yOffset+1,yOffset,zOffset,zOffset+1,zOffset+yOffset+1,zOffset+yOffset};
	for (int part =0; part < getNumParts(); part++) {
		for (nz = 0; nz < vox_nels[2]; nz++) {
			for (ny = 0; ny < vox_nels[1]; ny++) {
				for (nx = 0; nx < vox_nels[0]; nx++) {
					gridelem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
					if (vox_partIdOfGridElem[gridelem] == part) {
						startNode = nx + ny*yOffset + nz*zOffset; // start gridnode number for a given hex
						for (int m = 0; m < 8; m++) {
							gridnode = startNode + nodeIndex[m];
							if (vox_nodeMapping[gridnode] < 0){
								printf("Mesher: Error in node mapping \n");
								vox_nodeMapping[gridnode] = 0;
							}
							fprintf(fptr,"%12d ",vox_nodeMapping[gridnode]);// use the mapped node index
						}
						fprintf(fptr,"\n");
					}
				}
			}
		}
	}
	if (getNumParts() > 0) {
		fprintf(fptr,"#PartId\n");
		for (int part =0; part < getNumParts(); part++) {
			for (nz = 0; nz < vox_nels[2]; nz++) {
				for (ny = 0; ny < vox_nels[1]; ny++) {
					for (nx = 0; nx < vox_nels[0]; nx++) {
						gridelem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
						if (vox_partIdOfGridElem[gridelem] == part) 
							fprintf(fptr,"%d ",vox_partIdOfGridElem[gridelem]);
					}
				}
			}
			fprintf(fptr,"\n");
		}
	}
	fclose(fptr);
}
//*********************************************
bool Voxelizer::dumpVoxelDataBinary(const char *fileName)
{
	/*
	Print the following in binary format (see dumpVoxel)
	% 1. nelx nely nelz
	% 2. X0 Y0 Z0
	% 3. dx dy dz
	% 4. h_nNodes
	% 5. (i,j,k,f) of all nodes (i ranges from 0 to nelx, etc), f is the facegroup
	% 6. h_nElems
	% 7. (n1,n2,...,n8) of all elements
	% 8 (m0,m1,m2,....) for each element, the material id, default is 0 
	% 9. 1,0.9,0.2,1,0,.... for each element, the pseudo density, default is 1.0

	CAVEAT:
	We assume a 0-index, i.e., it is assumed that all gridnode numbers, element numbers, and 
	templateIDs start from 0.
	*/


	int i,gridnode,nx,ny,nz,startNode,nDirichletDOF,nNeumannDOF,nOutputDOF,faceGroup,gridelem,tri;
	int nNodesFaceGroup[STL_MAX_GROUPS],tempInt[4];

	for (i =0; i < STL_MAX_GROUPS; i++)
		nNodesFaceGroup[i] = 0;


	FILE *fptr = fopen(fileName,"wb");

	if (fptr == NULL){
        cout << "Could not open file %s " << fileName << endl;
		return false;
	}
	fwrite(vox_nels,sizeof(vox_nels[0]),3,fptr);
	fwrite(stl_XYZMin,sizeof(stl_XYZMin[0]),3,fptr);
	fwrite(vox_dsize,sizeof(vox_dsize[0]),3,fptr);
	fwrite(&vox_nNodesInside,sizeof(int),1,fptr);
	
	gridnode = 0;
	nDirichletDOF = 0;
	nNeumannDOF = 0;
	nOutputDOF = 0;
	for (nz = 0; nz < vox_nels[2]+1; nz++) {
		for (ny = 0; ny < vox_nels[1]+1; ny++) {
			for (nx = 0; nx < vox_nels[0]+1; nx++) {
				if (nElemsConnectedToNode[gridnode]>0) {// at least one element is connected to it
					tri = vox_closestTriangleToGridNode[gridnode];
					faceGroup = triGroup[tri];
					tempInt[0] = nx;
					tempInt[1] = ny;
					tempInt[2] = nz;
					tempInt[3] = faceGroup;
					fwrite(tempInt,sizeof(tempInt[0]),4,fptr);
					if (nElemsConnectedToNode[gridnode]<8) {//boundary gridnode
						nNodesFaceGroup[faceGroup]++;
					}
				}
				gridnode++;
			}
		}
	}
	fwrite(&vox_nElemsInside,sizeof(int),1,fptr);
	
	int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
	int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z
	int nodeIndex[8] = {0,1,yOffset+1,yOffset,zOffset,zOffset+1,zOffset+yOffset+1,zOffset+yOffset};
	for (int part =0; part < getNumParts(); part++) {
		for (nz = 0; nz < vox_nels[2]; nz++) {
			for (ny = 0; ny < vox_nels[1]; ny++) {
				for (nx = 0; nx < vox_nels[0]; nx++) {
					gridelem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
					if (vox_partIdOfGridElem[gridelem] == part) {
						startNode = nx + ny*yOffset + nz*zOffset; // start gridnode number for a given hex
						for (int m = 0; m < 8; m++) {
							gridnode = startNode + nodeIndex[m];
							if (vox_nodeMapping[gridnode] < 0){
                                writeMessageToSTLLog("Mesher: Error in node mapping ");
								vox_nodeMapping[gridnode] = 0;
							}
							fwrite(&(vox_nodeMapping[gridnode]),sizeof(int),1,fptr);
						}
					}
				}
			}
		}
	}
	//dump part id
	for (int part =0; part < getNumParts(); part++) {
		for (nz = 0; nz < vox_nels[2]; nz++) {
			for (ny = 0; ny < vox_nels[1]; ny++) {
				for (nx = 0; nx < vox_nels[0]; nx++) {
					gridelem = nz*(vox_nels[1]*vox_nels[0]) + ny*(vox_nels[0]) + nx;
					if (vox_partIdOfGridElem[gridelem] == part) 
						fwrite(&(vox_partIdOfGridElem[gridelem]),sizeof(int),1,fptr);
				}
			}
		}
	}
	//Dump pseudo density
	double *tempDensity = (double *)malloc(vox_nElemsInside*sizeof(double));
#pragma omp parallel for 
	for (int i = 0; i < vox_nElemsInside; i++)
		tempDensity[i] = 1.0;

	fwrite(tempDensity,sizeof(double),vox_nElemsInside,fptr);

	free(tempDensity);
	fclose(fptr);
	return true;
}
/**********************************************************************/
bool Voxelizer::assignValueAtAllVoxelBoundaryNodes(double value)
{
    if (vox_nodeValues == NULL) return false;
    int gridNode,nx,ny,nz;

    for (int i = 0; i < 2; i++) {
        nx = (i == 0)? 0:vox_nels[0];
        for (ny = 0; ny <= vox_nels[1]; ny++){
            for (nz = 0; nz <= vox_nels[2]; nz++){
                gridNode = nx + ny*(vox_nels[0]+1) + nz*(vox_nels[0]+1)*(vox_nels[1]+1);
                vox_nodeValues[gridNode] = value;
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        ny = (i == 0)? 0:vox_nels[1];
        for (nx = 0; nx <= vox_nels[0]; nx++){
            for (nz = 0; nz <= vox_nels[2]; nz++){
                gridNode = nx + ny*(vox_nels[0]+1) + nz*(vox_nels[0]+1)*(vox_nels[1]+1);
                vox_nodeValues[gridNode] = value;
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        nz = (i == 0)? 0:vox_nels[2];
        for (nx = 0; nx <= vox_nels[0]; nx++){
            for (ny = 0; ny <= vox_nels[1]; ny++){
                gridNode = nx + ny*(vox_nels[0]+1) + nz*(vox_nels[0]+1)*(vox_nels[1]+1);
                vox_nodeValues[gridNode] = value;
            }
        }
    }
}
/**********************************************************************/
bool Voxelizer::assignValueAtVoxNode(int nodex,int nodey,int nodez,double value)
{
    if (vox_nodeValues == NULL) return false;
    int gridNode;

    if ((nodex < 0) || (nodex >= (vox_nels[0]+1))) return false;
    if ((nodey < 0) || (nodey >= (vox_nels[1]+1))) return false;
    if ((nodez < 0) || (nodez >= (vox_nels[2]+1))) return false;

    gridNode = nodex + nodey*(vox_nels[0]+1) + nodez*(vox_nels[0]+1)*(vox_nels[1]+1);
    vox_nodeValues[gridNode] = value;
    return true;
}
/**********************************************************************/
bool Voxelizer::assignImplicitFunctionAtGridNodes(double (*function)(double pt[3]))
{
    // Evaluate function at all grid nodes and assign the values to vox_nodeValues
    // This is typically used for extracting isosurfaces
    double pt[3];
    int gridNode;

    if (vox_nodeValues == NULL) return false;
    for (int nx = 0; nx <= vox_nels[0]; nx++){// #nodes = #elements+1
        pt[0] = nx*vox_dsize[0] + stl_XYZMin[0];
        for (int ny = 0; ny <= vox_nels[1]; ny++){
            pt[1] = ny*vox_dsize[1] + stl_XYZMin[1];
            for (int nz = 0; nz <= vox_nels[2]; nz++){
                pt[2] = nz*vox_dsize[2] + stl_XYZMin[2];
                gridNode = nx + ny*(vox_nels[0]+1) + nz*(vox_nels[0]+1)*(vox_nels[1]+1);
                vox_nodeValues[gridNode] = function(pt);
            }
        }
    }
    return true;
}
/******************************************************************/
bool Voxelizer::createAndDumpIsoSurfaceFromNodalValues(const char *fileName,double tau)
{
    int ntri;
    double x[8],y[8],z[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3],nrm;
    GRIDCELL grid;
    TRIANGLE triangles[10];
    int gridNodes[8];
    float fbuf[12];
    unsigned short att = 0;
    unsigned int nTriangles;
    int yOffset = (vox_nels[0]+1); // difference in 2 nodes that are next to each other in y
    int zOffset = (vox_nels[0]+1)*(vox_nels[1]+1); // difference in 2 nodes that are next to each other in z


    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL) {
        writeMessageToSTLLog("Unable to open STL file: %s ",fileName);
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;

    for (int nx = 0; nx < vox_nels[0]; nx++){//elem
        x[0] = x[3] = x[4] = x[7] = nx*vox_dsize[0] + stl_XYZMin[0];
        x[1] = x[2] = x[5] = x[6] = x[0] + vox_dsize[0];
        for (int ny = 0; ny < vox_nels[1]; ny++){
            y[0] = y[1] = y[4] = y[5] = ny*vox_dsize[1] + stl_XYZMin[1];
            y[2] = y[3] = y[6] = y[7] = y[0] + vox_dsize[1];
            for (int nz = 0; nz < vox_nels[2]; nz++){
                z[0] = z[1] = z[2] = z[3] = nz*vox_dsize[2] + stl_XYZMin[2];
                z[4] = z[5] = z[6] = z[7] = z[0] + vox_dsize[2];
                gridNodes[0] = nx + ny*(vox_nels[0]+1) + nz*(vox_nels[0]+1)*(vox_nels[1]+1);
                gridNodes[1] = gridNodes[0] + 1;
                gridNodes[2] = gridNodes[1] + yOffset;
                gridNodes[3] = gridNodes[0] + yOffset;
                gridNodes[4] = gridNodes[0] + zOffset;
                gridNodes[5] = gridNodes[1] + zOffset;
                gridNodes[6] = gridNodes[2] + zOffset;
                gridNodes[7] = gridNodes[3] + zOffset;
                for (int m = 0;m < 8;m++) {
                    grid.p[m].x = x[m];
                    grid.p[m].y = y[m];
                    grid.p[m].z = z[m];
                    grid.val[m] = vox_nodeValues[gridNodes[m]];
                }
                ntri = PolygoniseCube(grid,tau,triangles);
                for (int m = 0; m < ntri;m++) {
                    p0[0] = triangles[m].p[0].x;
                    p0[1] = triangles[m].p[0].y;
                    p0[2] = triangles[m].p[0].z;

                    p1[0] = triangles[m].p[1].x;
                    p1[1] = triangles[m].p[1].y;
                    p1[2] = triangles[m].p[1].z;

                    p2[0] = triangles[m].p[2].x;
                    p2[1] = triangles[m].p[2].y;
                    p2[2] = triangles[m].p[2].z;

                    nTriangles++;
                    SUBTRACT3D(p01,p1,p0);
                    SUBTRACT3D(p02,p2,p0);
                    CROSS3D(normal,p01,p02);

                    nrm = NORM3D(normal);

                    fbuf[0] = (float) (normal[0]/nrm) ;
                    fbuf[1] = (float) (normal[1]/nrm) ;
                    fbuf[2] = (float) (normal[2]/nrm) ;

                    fbuf[3]  = (float) (p0[0]);
                    fbuf[4]  = (float) (p0[1]);
                    fbuf[5]  = (float) (p0[2]);

                    fbuf[6]  = (float) (p1[0]);
                    fbuf[7]  = (float) (p1[1]);
                    fbuf[8]  = (float) (p1[2]);

                    fbuf[9]   = (float) (p2[0]);
                    fbuf[10]  = (float) (p2[1]);
                    fbuf[11]  = (float) (p2[2]);

                    fwrite(fbuf, 12*sizeof(float), 1, fptr);
                    fwrite(&att,sizeof(unsigned short),1,fptr);

                }
            }
        }
    }
    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);
    return true;
}
/**********************************************************************/
Voxelizer::~Voxelizer()
{
	if (vox_verbose)
		printf("Mesher: deleting object\n");

	if (nElemsConnectedToNode != NULL) {
		free(nElemsConnectedToNode);
		nElemsConnectedToNode = NULL;
	}
	if (vox_closestTriangleToGridNode !=NULL) {
		free(vox_closestTriangleToGridNode);
		vox_closestTriangleToGridNode = NULL;
	}

	if (vox_nodeMapping !=NULL) {
		free(vox_nodeMapping);
		vox_nodeMapping = NULL;
	}

    if (vox_nodeValues !=NULL) {
        free(vox_nodeValues);
        vox_nodeValues = NULL;
    }
	if (vox_elemMapping !=NULL) {
		free(vox_elemMapping);
		vox_elemMapping = NULL;
	}
	if (vox_partIdOfGridElem !=NULL) {
		free(vox_partIdOfGridElem);
		vox_partIdOfGridElem = NULL;
	}
}
