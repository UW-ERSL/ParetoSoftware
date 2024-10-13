#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CommonGUI.h"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "Timer.h"
#include <ctime>
#include "isosurface.h"
#include "taubinSmooth.h"



#pragma warning(disable:4996)
extern ParetoApplication topopt;
extern Voxelizer vox;
extern Timer globalTimer;
extern Timer timer;
extern struct LiveVariables liveVar;
extern char userDefinedTitle[256];

#define ISOSURFDivisions 2
double NShape[ISOSURFDivisions+1][ISOSURFDivisions+1][ISOSURFDivisions+1][8];

static bool NShapeComputed = false;


/**********************************************************************/

void isoSurfaceGenerationStandard(FILE *fptr, double *offset)
{
	// Used  both for plotting isosurfaces (if fptr is NULL) and for saving isosurfaces (not NULL)
	int elem,ntri;
	double x[8],y[8],z[8],TS[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3];
	GRIDCELL grid;
	TRIANGLE triangles[10];
	int savedCounter;
	double currentVolFraction,TSIsoCutoff,desiredVolFraction,timeInSec;
	int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
	double nrm,triArea;
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;
	

	if (!topopt.doesTSExist()) return;
	desiredVolFraction = liveVar.displayVolFraction;
	savedCounter = topopt.getClosestCounterAtVolFraction(desiredVolFraction,&currentVolFraction,&TSIsoCutoff,&timeInSec);
	if (savedCounter < 0) 	return;
	if (fptr) {// writing to binary file
		char header[80];
		memset(header,'0', 80*sizeof( char));// dummy header
		fwrite(header,80,1, fptr);
		nTriangles = 100;; // dummy, will overwrite later on
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}
	nTriangles = 0;
	
	for (elem = 0; elem < topopt.getNumElems(); elem++) {
		topopt.getTopSensAtElemNodesAtCounter(elem,savedCounter,TS);
		topopt.getElemNodeLocation(elem,x,y,z);
		for (int m = 0;m < 8;m++) {
			grid.p[m].x = x[m];
			grid.p[m].y = y[m];
			grid.p[m].z = z[m];
			grid.val[m] = TS[m];
		}
		ntri = PolygoniseCube(grid,TSIsoCutoff,triangles);

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

			triArea = vox.getAreaOfTriangle(p0,p1,p2);
			if ( (_isnan(triArea)))
				continue;
			nTriangles++;
			SUBTRACT3D(p01,p1,p0);
			SUBTRACT3D(p02,p2,p0);
			CROSS3D(normal,p01,p02);

			if (fptr == NULL) {
				/*
#if (COMPILE_MODE != BATCH_MODE) 
				GLfloat color[4];
				color[0] = 0.7; color[1] = 0.3; color[2] = 0; color[3] = 1.0;
				glNormal3f(normal[0],normal[1],normal[2]);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
				glBegin(GL_TRIANGLES);
				glNormal3f(normal[0],normal[1],normal[2]);
				glVertex3d(triangles[m].p[0].x, triangles[m].p[0].y,  triangles[m].p[0].z);
				glVertex3d(triangles[m].p[1].x, triangles[m].p[1].y,  triangles[m].p[1].z);
				glVertex3d(triangles[m].p[2].x, triangles[m].p[2].y,  triangles[m].p[2].z);
				glEnd();
#endif
*/
			}
			else {
				nrm = NORM3D(normal);

				fbuf[0] = (float) (normal[0]/nrm) ; 
				fbuf[1] = (float) (normal[1]/nrm) ; 
				fbuf[2] = (float) (normal[2]/nrm) ;

				fbuf[3]  = (float) (p0[0]+offset[0]);
				fbuf[4]  = (float) (p0[1]+offset[1]);
				fbuf[5]  = (float) (p0[2]+offset[2]);

				fbuf[6]  = (float) (p1[0]+offset[0]);
				fbuf[7]  = (float) (p1[1]+offset[1]);
				fbuf[8]  = (float) (p1[2]+offset[2]);

				fbuf[9]   = (float) (p2[0]+offset[0]);
				fbuf[10]  = (float) (p2[1]+offset[1]);
				fbuf[11]  = (float) (p2[2]+offset[2]);

				fwrite(fbuf, 12*sizeof(float), 1, fptr);
				fwrite(&att,sizeof(unsigned short),1,fptr);

			}
		}
	}
	if (fptr) {
		fseek(fptr,80,SEEK_SET);
		// write the number of triangles
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}		
}

/************************************************************/
void computeNShape(void)
{
	double xi,eta,mu,delta,temp;

	int i,j,k;
	xi = -1;
	eta = -1; 
	mu = -1;
	delta = 2.0/(ISOSURFDivisions);
	// The following is used for plotting triangles on a finer grid
	for (k = 0; k < ISOSURFDivisions+1; k++) {
		for (j = 0; j < ISOSURFDivisions+1; j++) {
			for (i = 0; i < ISOSURFDivisions+1; i++) {
				topopt.computeN(xi,eta,mu,NShape[i][j][k]);
				temp = 1.0;
				for (int r = 0; r < 8; r++)
					temp *= NShape[i][j][k][r];
				xi += delta;
			}
			xi = -1;
			eta += delta;
		}
		xi = -1;
		eta = -1;
		mu += delta;
	}
	NShapeComputed = true;
}
/**********************************************************************/
void isoSurfaceGenerationSubdivide(FILE *fptr, double *offset)
{
	// Used  both for plotting isosurfaces (if fptr is NULL) and for saving isosurfaces (not NULL)
	int elem,m,ntri,nodes[8];
	double x[8],y[8],z[8],TS[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3],ts_TSMax,ts_TSMin;
	double xgrid[8],ygrid[8],zgrid[8],TSgrid[8];
	int xIndex[8],yIndex[8],zIndex[8];
	GRIDCELL grid;
	TRIANGLE triangles[10];
	int savedCounter;
	double currentVolFraction,TSIsoCutoff,desiredVolFraction,timeInSec,temp,temp2;
	int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
	double size[3],dVoxel,nrm,triArea;
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;

	if (!topopt.doesTSExist()) return;
	vox.getVoxelSize(size);
	dVoxel = 0.8*MAX(size[0],MAX(size[1],size[2]));
	currentVolFraction = 1;

	desiredVolFraction = liveVar.displayVolFraction;
	savedCounter = topopt.getClosestCounterAtVolFraction(desiredVolFraction,&currentVolFraction,&TSIsoCutoff,&timeInSec);
	if (savedCounter < 0) 	return;

	if (!NShapeComputed) computeNShape();
	if (fptr) {
		char header[80];
		memset(header,'0', 80*sizeof(unsigned char));// dummy header
		fwrite(header,80,1, fptr);
		nTriangles = 100;; // dummy, will overwrite later on
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}
	nTriangles = 0;
	for (elem = 0; elem < topopt.getNumElems(); elem++) {
		topopt.getTopSensAtElemNodesAtCounter(elem,savedCounter,TS);
		ts_TSMax = -HUGE_VAL;
		ts_TSMin = HUGE_VAL;
		for (m = 0; m < 8; m++) {	 
			ts_TSMax = MAX(ts_TSMax,TS[m]);
			ts_TSMin = MIN(ts_TSMin,TS[m]);
		}
		//if ((ts_TSMax < TSIsoCutoff) || (ts_TSMin > TSIsoCutoff) ) continue;// element is completely outside or inside	
		topopt.getElemNodes(elem,nodes);
		topopt.getElemNodeLocation(elem,x,y,z);
		// divide each element into sub elements to avoid pathological problems
		for (int xDiv = 0; xDiv < ISOSURFDivisions; xDiv++) {
			xIndex[0] = xDiv; xIndex[1] = xDiv+1; xIndex[2] = xDiv+1; xIndex[3] = xDiv; 
			xIndex[4] = xDiv; xIndex[5] = xDiv+1; xIndex[6] = xDiv+1; xIndex[7] = xDiv;
			for (int yDiv = 0; yDiv <ISOSURFDivisions; yDiv++) {
				yIndex[0] = yDiv; yIndex[1] = yDiv; yIndex[2] = yDiv+1; yIndex[3] = yDiv+1; 
				yIndex[4] = yDiv; yIndex[5] = yDiv; yIndex[6] = yDiv+1; yIndex[7] = yDiv+1; 
				for (int zDiv = 0; zDiv < ISOSURFDivisions; zDiv++) {
					zIndex[0] = zDiv; zIndex[1] = zDiv; zIndex[2] = zDiv; zIndex[3] = zDiv; 
					zIndex[4] = zDiv+1; zIndex[5] = zDiv+1; zIndex[6] = zDiv+1; zIndex[7] = zDiv+1; 
					for (m = 0; m < 8; m++) {
						xgrid[m] = 0;
						ygrid[m] = 0;
						zgrid[m] = 0;
						TSgrid[m] = 0;
						temp2 = 1;
						for (int kk =0; kk < 8; kk++) {
							temp =  NShape[xIndex[m]][yIndex[m]][zIndex[m]][kk];
							temp2 *= temp;
							xgrid[m] += temp*x[kk];
							ygrid[m] += temp*y[kk];
							zgrid[m] += temp*z[kk];
							TSgrid[m] += temp*TS[kk];
						}
						if ( (topopt.doesElemExistsAtTSCounter(elem,savedCounter)) && (fabs(temp2) > 1e-9)) {//interior node
							TSgrid[m] = MAX(TSgrid[m] ,TSIsoCutoff+0.1);
						}
					}
					for (m = 0;m < 8;m++) {
						grid.p[m].x = xgrid[m];
						grid.p[m].y = ygrid[m];
						grid.p[m].z = zgrid[m];
						grid.val[m] = TSgrid[m];
					}
					ntri = PolygoniseCube(grid,TSIsoCutoff,triangles);
					for (m = 0; m < ntri;m++) {
						p0[0] = triangles[m].p[0].x;
						p0[1] = triangles[m].p[0].y;
						p0[2] = triangles[m].p[0].z;

						p1[0] = triangles[m].p[1].x;
						p1[1] = triangles[m].p[1].y;
						p1[2] = triangles[m].p[1].z;

						p2[0] = triangles[m].p[2].x;
						p2[1] = triangles[m].p[2].y;
						p2[2] = triangles[m].p[2].z;

						triArea = vox.getAreaOfTriangle(p0,p1,p2);
						if ( (_isnan(triArea)))
							continue;
						nTriangles++;
						SUBTRACT3D(p01,p1,p0);
						SUBTRACT3D(p02,p2,p0);
						CROSS3D(normal,p01,p02);

						if (fptr == NULL) {
							/*
#if (COMPILE_MODE != BATCH_MODE) 
							GLfloat color[4] ={0.7, 0.3, 0.0, 0.9};
							glNormal3f(normal[0],normal[1],normal[2]);
							glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
							glBegin(GL_TRIANGLES);
							glNormal3f(normal[0],normal[1],normal[2]);
							glVertex3d(triangles[m].p[0].x, triangles[m].p[0].y,  triangles[m].p[0].z);
							glVertex3d(triangles[m].p[1].x, triangles[m].p[1].y,  triangles[m].p[1].z);
							glVertex3d(triangles[m].p[2].x, triangles[m].p[2].y,  triangles[m].p[2].z);
							glEnd();
#endif
*/
						}
						else {
						nrm = NORM3D(normal);

						fbuf[0] = (float) (normal[0]/nrm) ; 
						fbuf[1] = (float) (normal[1]/nrm) ; 
						fbuf[2] = (float) (normal[2]/nrm) ;

						fbuf[3]  = (float) (p0[0]+offset[0]);
						fbuf[4]  = (float) (p0[1]+offset[1]);
						fbuf[5]  = (float) (p0[2]+offset[2]);

						fbuf[6]  = (float) (p1[0]+offset[0]);
						fbuf[7]  = (float) (p1[1]+offset[1]);
						fbuf[8]  = (float) (p1[2]+offset[2]);

						fbuf[9]   = (float) (p2[0]+offset[0]);
						fbuf[10]  = (float) (p2[1]+offset[1]);
						fbuf[11]  = (float) (p2[2]+offset[2]);

						fwrite(fbuf, 12*sizeof(float), 1, fptr);
						fwrite(&att,sizeof(unsigned short),1,fptr);

						}
					}
				}
			}
		}
	}
	if ( (fptr)) {
		fseek(fptr,80,SEEK_SET);
		// write the number of triangles
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}		
}
/**********************************************************************/
void isoSurfaceGeneration(FILE *fptr, double *offset)
{
	isoSurfaceGenerationStandard(fptr,offset);
	//isoSurfaceGenerationSubdivide(fptr,offset);
}
/**********************************************************************/
void isoSurfaceGenerationInverted(FILE *fptr, double *offset)
{
	// Used  both for plotting negative isosurfaces (if fptr is NULL) and for saving negative isosurfaces  (not NULL)
	int elem,m,ntri,nodes[8];
	double x[8],y[8],z[8],TS[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3];
	GRIDCELL grid;
	TRIANGLE triangles[10];
	int savedCounter;
	double currentVolFraction,TSIsoCutoff,desiredVolFraction,timeInSec;
	int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
	double size[3],dVoxel,nrm,triArea;
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;

	if (!topopt.doesTSExist()) return;
	vox.getVoxelSize(size);
	dVoxel = 0.8*MAX(size[0],MAX(size[1],size[2]));
	currentVolFraction = 1;

	desiredVolFraction = liveVar.displayVolFraction;
	savedCounter = topopt.getClosestCounterAtVolFraction(desiredVolFraction,&currentVolFraction,&TSIsoCutoff,&timeInSec);
	
	if (savedCounter < 0) 	return;

	sprintf(userDefinedTitle,"Vol. Fraction = %0.3f, time = %.2f (secs)",currentVolFraction,timeInSec);
	if ((fptr)) {
		char header[80];
		memset(header,'0', 80*sizeof(unsigned char));// dummy header
		fwrite(header,80,1, fptr);
		nTriangles = 100;; // dummy, will overwrite later on
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}

	nTriangles = 0;
	for (elem = 0; elem < topopt.getNumElems(); elem++) {
		topopt.getTopSensAtElemNodesAtCounter(elem,savedCounter,TS);
		topopt.getElemNodes(elem,nodes);
		topopt.getElemNodeLocation(elem,x,y,z);
		for (m = 0;m < 8;m++) {
			grid.p[m].x = x[m];
			grid.p[m].y = y[m];
			grid.p[m].z = z[m];
			grid.val[m] = TS[m];
		}

		ntri = PolygoniseCube(grid,TSIsoCutoff,triangles);
		for (m = 0; m < ntri;m++) {
			p0[0] = triangles[m].p[0].x;
			p0[1] = triangles[m].p[0].y;
			p0[2] = triangles[m].p[0].z;

			p1[0] = triangles[m].p[1].x;
			p1[1] = triangles[m].p[1].y;
			p1[2] = triangles[m].p[1].z;

			p2[0] = triangles[m].p[2].x;
			p2[1] = triangles[m].p[2].y;
			p2[2] = triangles[m].p[2].z;

			triArea = vox.getAreaOfTriangle(p0,p1,p2);
			if ( (_isnan(triArea)))
				continue;
			nTriangles ++;
			SUBTRACT3D(p01,p1,p0);
			SUBTRACT3D(p02,p2,p0);
			CROSS3D(normal,p01,p02);

			if (fptr == NULL) {
				/**
#if (COMPILE_MODE != BATCH_MODE) 
				GLfloat color[4] ={0.7, 0.3, 0.0, 0.9};
				glNormal3f(normal[0],normal[1],normal[2]);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
				glBegin(GL_TRIANGLES);
				glNormal3f(normal[0],normal[1],normal[2]);
				glVertex3d(triangles[m].p[0].x, triangles[m].p[0].y,  triangles[m].p[0].z);
				glVertex3d(triangles[m].p[1].x, triangles[m].p[1].y,  triangles[m].p[1].z);
				glVertex3d(triangles[m].p[2].x, triangles[m].p[2].y,  triangles[m].p[2].z);
				glEnd();
#endif*/
			}
			else {
				nrm = NORM3D(normal);
				
					fbuf[0] = (float) (normal[0]/nrm) ; 
					fbuf[1] = (float) (normal[1]/nrm) ; 
					fbuf[2] = (float) (normal[2]/nrm) ;

					fbuf[3]  = (float) (p0[0]+offset[0]);
					fbuf[4]  = (float) (p0[1]+offset[1]);
					fbuf[5]  = (float) (p0[2]+offset[2]);

					fbuf[6]  = (float) (p1[0]+offset[0]);
					fbuf[7]  = (float) (p1[1]+offset[1]);
					fbuf[8]  = (float) (p1[2]+offset[2]);

					fbuf[9]   = (float) (p2[0]+offset[0]);
					fbuf[10]  = (float) (p2[1]+offset[1]);
					fbuf[11]  = (float) (p2[2]+offset[2]);

					fwrite(fbuf, 12*sizeof(float), 1, fptr);
					fwrite(&att,sizeof(unsigned short),1,fptr);
	
			}
		}
	}
	if ((fptr)) {
		fseek(fptr,80,SEEK_SET);
		// write the number of triangles
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}		
}
/**********************************************************************/
void isoSurfaceGenerationFromPseudoDensity(FILE *fptr, double *offset,double pseudodensity)
{
	int elem,m,ntri;
	double x[8],y[8],z[8],pseudoDensity[8],p0[3],p1[3],p2[3],p01[3],p02[3], normal[3],nrm;
	GRIDCELL grid;
	bool boundaryElem;
	TRIANGLE triangles[10];
	double IsoCutoff,triArea;
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;


	if (fptr) {// writing to binary file
		char header[80];
		memset(header,'0', 80*sizeof( char));// dummy header
		fwrite(header,80,1, fptr);
		nTriangles = 100;; // dummy, will overwrite later on
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}

	IsoCutoff = pseudodensity;  
	nTriangles = 0;
	for (elem = 0; elem < topopt.getNumElems(); elem++) {
		boundaryElem = topopt.isElemOnEvolvingBoundary(elem);
		//if (!boundaryElem) continue;
		topopt.getElemDensityAtElemNodes(elem,pseudoDensity);
		topopt.getElemNodeLocation(elem,x,y,z);
		for (m = 0;m < 8;m++) {
			grid.p[m].x = x[m];
			grid.p[m].y = y[m];
			grid.p[m].z = z[m];
			grid.val[m] = pseudoDensity[m];
		}
		ntri = PolygoniseCube(grid,IsoCutoff,triangles);
		for (m = 0; m < ntri;m++) {
			p0[0] = triangles[m].p[0].x;
			p0[1] = triangles[m].p[0].y;
			p0[2] = triangles[m].p[0].z;

			p1[0] = triangles[m].p[1].x;
			p1[1] = triangles[m].p[1].y;
			p1[2] = triangles[m].p[1].z;

			p2[0] = triangles[m].p[2].x;
			p2[1] = triangles[m].p[2].y;
			p2[2] = triangles[m].p[2].z;

			triArea = vox.getAreaOfTriangle(p0,p1,p2);
			if ((fabs(triArea) < 1e-10) || (_isnan(triArea)))
				continue;
			nTriangles++;
			SUBTRACT3D(p01,p1,p0);
			SUBTRACT3D(p02,p2,p0);
			CROSS3D(normal,p01,p02);	
			if (fptr == NULL) {
				/*
#if (COMPILE_MODE != BATCH_MODE) 
				GLfloat color[4] ={0.7, 0.3, 0.0, 0.9};
				GLfloat color2[4] ={0.0, 1.0, 0.0, 0.3};
				glNormal3f(normal[0],normal[1],normal[2]);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
				glBegin(GL_TRIANGLES);
				glNormal3f(normal[0],normal[1],normal[2]);
				glVertex3d(triangles[m].p[0].x, triangles[m].p[0].y,  triangles[m].p[0].z);
				glVertex3d(triangles[m].p[1].x, triangles[m].p[1].y,  triangles[m].p[1].z);
				glVertex3d(triangles[m].p[2].x, triangles[m].p[2].y,  triangles[m].p[2].z);
				glEnd();
#endif
*/
			}
			else {
				nrm = NORM3D(normal);

				fbuf[0] = (float) (normal[0]/nrm) ; 
				fbuf[1] = (float) (normal[1]/nrm) ; 
				fbuf[2] = (float) (normal[2]/nrm) ;

				fbuf[3]  = (float) (p0[0]+offset[0]);
				fbuf[4]  = (float) (p0[1]+offset[1]);
				fbuf[5]  = (float) (p0[2]+offset[2]);

				fbuf[6]  = (float) (p1[0]+offset[0]);
				fbuf[7]  = (float) (p1[1]+offset[1]);
				fbuf[8]  = (float) (p1[2]+offset[2]);

				fbuf[9]   = (float) (p2[0]+offset[0]);
				fbuf[10]  = (float) (p2[1]+offset[1]);
				fbuf[11]  = (float) (p2[2]+offset[2]);

				fwrite(fbuf, 12*sizeof(float), 1, fptr);
				fwrite(&att,sizeof(unsigned short),1,fptr);

			}
		}
	}
	if (fptr) {
		fseek(fptr,80,SEEK_SET);
		// write the number of triangles
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}
}
/**********************************************************************/
void meshSurface(FILE *fptr, double *offset)
{
	int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem;
	double x[8],y[8],z[8],p0[3],p1[3],p2[3],p01[3],p02[3], normal[3],nrm,tsCutoff,currentVolFraction,timeInSec;
	int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;
	bool neighboringElemsExists[7];

	if (fptr) {// writing to binary file
		char header[80];
		memset(header,'0', 80*sizeof( char));// dummy header
		fwrite(header,80,1, fptr);
		nTriangles = 100;; // dummy, will overwrite later on
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}

	nTriangles = 0;
	bool deformToIsosurface = true;
	int isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&currentVolFraction,&tsCutoff,&timeInSec);
	for (elem = 0; elem < topopt.getNumElems(); elem++) {
		if (!topopt.doesElemExistsAtTSCounter(elem,isoCounter)) continue;
		topopt.getElemNodeLocation(elem,x,y,z);
		topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);// in the correct order
		// 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
		neighboringElemsExists[0] = true;
		for (int m = 1; m < 7; m++) {// start with 1 for neighbors
			neighboringElemsExists[m] = false;
			neighboringElem = neighboringElemsOfElem[m];
			if (neighboringElem == -1) continue;
			if (!topopt.doesElemExistsAtTSCounter(neighboringElem,isoCounter)) continue;
			neighboringElemsExists[m] = true;
		}
		topopt.getElemNodes(elem,nodes);
		topopt.getElemNodeLocation(elem,x,y,z);

		
		if (deformToIsosurface) {
			int node,nodeNeighbors[27],nNeighbors;
			double nodeTSValue,neighborTSValue,x0,y0,z0,xyzNeighbor[3],wt,neighborWt;
			for (int i = 0; i < 8; i++) {// consider all 8 nodes of elem
				node = nodes[i];
				nodeTSValue = topopt.getTopSensAtNodeAtCounter(node,isoCounter);
				nNeighbors = topopt.getNeighborNodesOfNode(node,nodeNeighbors);
				if (nodeTSValue > 1.1*tsCutoff) continue;
				wt = 1;
				x0 = wt*x[i];
				y0 = wt*y[i];
				z0 = wt*z[i];
				
				for (int j = 0; j < nNeighbors; j++) {
					if (nodeNeighbors[j] < 0) continue;
					neighborTSValue = topopt.getTopSensAtNodeAtCounter(nodeNeighbors[j],isoCounter);
					topopt.getNodeXYZ(nodeNeighbors[j],xyzNeighbor);
					if (neighborTSValue <0.95*tsCutoff) continue;
					neighborWt = 0.33;
					x0 += neighborWt*xyzNeighbor[0];// pull this node towards neighbor
					y0 += neighborWt*xyzNeighbor[1];
					z0 += neighborWt*xyzNeighbor[2];
					wt+= neighborWt;
				}
				x[i] = x0/wt;
				y[i] = y0/wt;
				z[i] = z0/wt;
			}
		}
		for (int m = 0; m < 6;m++) {// consider the six quads
			if (neighboringElemsExists[m+1]) continue;//offset by 1
			for (int tri = 0; tri < 2; tri++) {// 2 triangles per quad
				if (tri == 0) {// vertices 0,1,2
					p0[0] = x[quads[m][0]]; p0[1] = y[quads[m][0]]; p0[2] = z[quads[m][0]];
					p1[0] = x[quads[m][1]]; p1[1] = y[quads[m][1]]; p1[2] = z[quads[m][1]];
					p2[0] = x[quads[m][2]]; p2[1] = y[quads[m][2]]; p2[2] = z[quads[m][2]];
				}
				else {//vertices 0,2,3
					p0[0] = x[quads[m][0]]; p0[1] = y[quads[m][0]]; p0[2] = z[quads[m][0]];
					p1[0] = x[quads[m][2]]; p1[1] = y[quads[m][2]]; p1[2] = z[quads[m][2]];
					p2[0] = x[quads[m][3]]; p2[1] = y[quads[m][3]]; p2[2] = z[quads[m][3]];
				}
				for (int i = 0; i < 3; i++) {
					p01[i] = p1[i] - p0[i];
					p02[i] = p2[i] - p0[i];
				}
				CROSS3D(normal,p01,p02);//normal of triangle

				if (fptr == NULL) {
					/*
#if (COMPILE_MODE != BATCH_MODE) 
					GLfloat color[4] ={0.7, 0.3, 0.0, 0.9};
					glNormal3f(normal[0],normal[1],normal[2]);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
					glBegin(GL_TRIANGLES);
						glNormal3f(normal[0],normal[1],normal[2]);
						glVertex3d(p0[0], p0[1],  p0[2]);
						glVertex3d(p1[0], p1[1],  p1[2]);
						glVertex3d(p2[0], p2[1],  p2[2]);
					glEnd();
#endif
*/
				}
				else {
					nTriangles++;
					nrm = NORM3D(normal);

					fbuf[0] = (float) (normal[0]/nrm) ; 
					fbuf[1] = (float) (normal[1]/nrm) ; 
					fbuf[2] = (float) (normal[2]/nrm) ;

					fbuf[3]  = (float) (p0[0]+offset[0]);
					fbuf[4]  = (float) (p0[1]+offset[1]);
					fbuf[5]  = (float) (p0[2]+offset[2]);

					fbuf[6]  = (float) (p1[0]+offset[0]);
					fbuf[7]  = (float) (p1[1]+offset[1]);
					fbuf[8]  = (float) (p1[2]+offset[2]);

					fbuf[9]   = (float) (p2[0]+offset[0]);
					fbuf[10]  = (float) (p2[1]+offset[1]);
					fbuf[11]  = (float) (p2[2]+offset[2]);

					fwrite(fbuf, 12*sizeof(float), 1, fptr);
					fwrite(&att,sizeof(unsigned short),1,fptr);

				}
			}
		}
	}
	if (fptr) {
		fseek(fptr,80,SEEK_SET);
		// write the number of triangles
		fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	}
}
/**********************************************************************/
bool saveIsoSurface(char *fileName,double *offset,bool snapToGeom,double desiredFractionOfIsoTriangles,bool fromMeshDensity,double pseudodensityCutoff=0.8)
{
	indexed_mesh mesh;

	// First create the isosurface in binary format
	FILE *fptrTemp = fopen(fileName,"wb");
	if (fptrTemp == NULL) {
		printf("Unable to open STL file: %s \n",fileName);
		return false;
	}
	if (fromMeshDensity) isoSurfaceGenerationFromPseudoDensity(fptrTemp,offset,pseudodensityCutoff);
	else isoSurfaceGeneration(fptrTemp,offset);
	fclose(fptrTemp);

	if(!mesh.load_from_binary_stereo_lithography_file(fileName)){
		cout << "Error: Could not properly read file " << fileName<< endl;
		return false;
	}

	mesh.taubin_smooth(0.6307f, -0.6734f,6);
	
	if (snapToGeom) {
		double dVoxel = topopt.getMeshElementSize();
		// Algorithm: smooth, snap, smooth,snap
		double xyz[3];
		int nSmoothingIterations = 3;
		for (int nSmooth =0; nSmooth < nSmoothingIterations; nSmooth++) {
			mesh.taubin_smooth(0.6307f, -0.6734f,1);
			for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
				mesh.getVertex(i,xyz);
				vox.snapPointToGeometry(xyz,1.1*dVoxel);//find the new  xyz location after snapping to geometry
				mesh.setVertex(i,xyz);//make the change only if the vertex normals have not changed much
			}
		}
	}
	if(!mesh.save_to_binary_stereo_lithography_file(fileName)){
		cout << "Error: Could not properly write file " << fileName << endl;
		return false;
	}
	return true;
}

/**********************************************************************/
void smoothMesh_Taubin(double *temp_x,double *temp_y,double *temp_z,int maxIter, bool* fixedNodes)
{
	double XYZ[3],delta[3],size[3],v[3],neighborXYZ[3],neighborWeight,totalWeight;
	int numNodes,numNeighborNodes,numNeighborElems,*neighborNodes,currentNeighbors[27],neighborElems[8];
	bool flag;
	numNodes = topopt.getNumNodes();
	
	vox.getVoxelSize(size);
	double dVoxel = 0.8*MAX(size[0],MAX(size[1],size[2]));

	neighborNodes = (int *)malloc(numNodes*27*sizeof(int));
	memset(neighborNodes,0,numNodes*27*sizeof(int));
	topopt.getNeighborNodes(neighborNodes);
	// Taubin constants
	double kpb = 0.1;
	double lambda = 0.6307;
	double mu = 1.0/(kpb-1.0/lambda);
	double scale = 0.5;
	lambda *= scale; 
	mu *= scale; 
	// backup voxel data
	double *backupXYZ;
	backupXYZ = (double *)malloc(3*numNodes*sizeof(double));
	memset(backupXYZ,0,3*numNodes*sizeof(double));
	for (int node=0;node<numNodes;node++){
		topopt.getNodeXYZ(node,XYZ);
		for (int i=0;i<3;i++){
			backupXYZ[3*node+i] = XYZ[i];
		}
	}
	// initialize
	memset(temp_x,0,numNodes*sizeof(double));
	memset(temp_y,0,numNodes*sizeof(double));
	memset(temp_z,0,numNodes*sizeof(double));
	for (int node=0;node<numNodes;node++){
		topopt.getNodeXYZ(node,XYZ);
			temp_x[node] = XYZ[0];
			temp_y[node] = XYZ[1];
			temp_z[node] = XYZ[2];
	}
	for (int iter=0;iter<maxIter;iter++){

		for (int node=0;node<numNodes;node++){
			if (!(topopt.isNodeOnEvolvingBoundary(node) || topopt.isNodeOnBoundary(node))) continue;
			if (fixedNodes[node]) continue;
			topopt.getNodeXYZ(node,XYZ);
			numNeighborNodes = 0;
			totalWeight = 0.0;
			delta[0] = delta[1] = delta[2] = 0.0;
			// find neighboring nodes
			for (int i=0;i<27;i++){
				currentNeighbors[i] = neighborNodes[numNodes*i+node];
				if ( currentNeighbors[i]<0 || currentNeighbors[i]>numNodes-1) continue;
				if ( currentNeighbors[i] == node) continue;
				if (!(topopt.isNodeOnEvolvingBoundary(currentNeighbors[i]) || topopt.isNodeOnBoundary(currentNeighbors[i]))) continue;
				// remove if belongs to no solid elem 
				numNeighborElems = topopt.getNeighboringElemsOfNode(currentNeighbors[i] ,neighborElems);
				flag = false;
				for (int j = 0;j<numNeighborElems;j++){
					if (topopt.getElemPseudoDensity(neighborElems[j])<0.5) continue;
					flag = true;
					break;
				}
				if (!flag)continue;
				// node exists
				numNeighborNodes++;
				topopt.getNodeXYZ(currentNeighbors[i],neighborXYZ);
				SUBTRACT3D(v,neighborXYZ,XYZ); 
				neighborWeight = 1.0/(NORM3D(v));
				totalWeight = totalWeight+neighborWeight;
				for (int j=0;j<3;j++){
					delta[j] = delta[j] + neighborWeight*v[j];
				}
			}
			if (numNeighborNodes==0) continue;
			for (int j=0;j<3;j++){
				delta[j] /= totalWeight;
			}
			temp_x[node] += lambda*delta[0];
			temp_y[node] += lambda*delta[1];
			temp_z[node] += lambda*delta[2];
		}
		for (int node=0;node<numNodes;node++){
			if (fixedNodes[node]) continue;
			XYZ[0] = temp_x[node];
			XYZ[1] = temp_y[node];
			XYZ[2] = temp_z[node];
			topopt.setMeshNodeLocation(node,XYZ);
		}
		for (int node=0;node<numNodes;node++){
			if (!(topopt.isNodeOnEvolvingBoundary(node) || topopt.isNodeOnBoundary(node))) continue;
			if (fixedNodes[node]) continue;
			topopt.getNodeXYZ(node,XYZ);
			numNeighborNodes = 0;
			totalWeight = 0.0;
			delta[0] = delta[1] = delta[2] = 0.0;
			// find neighboring nodes
			for (int i=0;i<27;i++){
				currentNeighbors[i] = neighborNodes[numNodes*i+node];
				if ( currentNeighbors[i]<0 || currentNeighbors[i]>numNodes-1) continue;
				if ( currentNeighbors[i]== node) continue;
				if (!(topopt.isNodeOnEvolvingBoundary(currentNeighbors[i]) || topopt.isNodeOnBoundary(currentNeighbors[i]))) continue;
				// remove if belongs to no solid elem 
				numNeighborElems = topopt.getNeighboringElemsOfNode(currentNeighbors[i] ,neighborElems);
				flag = false;
				for (int j = 0;j<numNeighborElems;j++){
					if (topopt.getElemPseudoDensity(neighborElems[j])<0.5) continue;
					flag = true;
					break;
				}
				if (!flag)continue;
				// node exists
				numNeighborNodes++;
				topopt.getNodeXYZ(currentNeighbors[i],neighborXYZ);
				SUBTRACT3D(v,neighborXYZ,XYZ); 
				neighborWeight = 1.0/(NORM3D(v));
				totalWeight = totalWeight+neighborWeight;

				for (int j=0;j<3;j++){
					delta[j] = delta[j] + neighborWeight*v[j];
				}
			}
			if (numNeighborNodes==0) continue;
			for (int j=0;j<3;j++){
				delta[j] /= totalWeight;
			}
			temp_x[node] += mu*delta[0];
			temp_y[node] += mu*delta[1];
			temp_z[node] += mu*delta[2];
		}
		for (int node=0;node<numNodes;node++){
			if (fixedNodes[node]) continue;
			XYZ[0] = temp_x[node];
			XYZ[1] = temp_y[node];
			XYZ[2] = temp_z[node];
			topopt.setMeshNodeLocation(node,XYZ);
		}
	}
	// backup voxel data
	for (int node=0;node<numNodes;node++){
		for (int i=0;i<3;i++){
			XYZ[i] = backupXYZ[3*node+i];
		}
		topopt.setMeshNodeLocation(node,XYZ);
	}
	topopt.safeFree(backupXYZ);
	topopt.safeFree(neighborNodes);
}
