#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string> 
#include <fstream> 
#include <iostream> 
#ifndef __APPLE__
#include <omp.h>
#endif
#include "cmatrix.h"
#include "Timer.h"
#include "TopOptIsoSurface.h"
#include "queue.h"
#include "isosurface.h"

using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)


/**********************************************************************/
TopOptIsoSurface::TopOptIsoSurface() 
{
	for (int i = 0; i < MAX_ISO_SAVES;i++) {
		ts_isoNPoints[i]  = 0;
		ts_isoNTriangles[i] = 0;
		ts_isoXYZ[i] = NULL;
		ts_isoVertexNormals[i] = NULL;
		ts_isoNormals[i] = NULL;
		ts_isoPartId[i] = NULL;
		ts_isoTriangles[i] = NULL;
		ts_isoTrianglesNeighbors[i] = NULL;
	}
	ts_isoUVWFinal = NULL;
	ts_isoVonMisesFinal = NULL;
    ts_isoTemperatureFinal = NULL;
    ts_isoModalFinal = NULL;
}
//*********************************************
int TopOptIsoSurface::findDuplicateIsoPt(int isoCounter,double pt[3])
// find duplicate vertices in iso surface
{	
	double temp[3];
	double *ptr;
	int node = -1;

	for (int i = 0; i < ts_isoNPoints[isoCounter];i++) {
		ptr = &(ts_isoXYZ[isoCounter][3*i]);
		SUBTRACT3D(temp,pt,ptr);
		if (ABSSUM3D(temp)< 1000*TOL){
			node = i;
			break;
		}
	}
	return node;
}
//*********************************************
bool TopOptIsoSurface::readIsoBinary(const char *fileName, int isoCounter)
// Read STLGeom file (binary)
/*
UINT8[80] - Header
UINT32 - Number of triangles

foreach triangle
REAL32[3] Normal vector
REAL32[3] Vertex 1
REAL32[3] Vertex 2
REAL32[3] Vertex 3
UINT16 – Attribute byte count
end*/
{
	int  node;

	FILE *filein;


	filein = fopen(fileName,"rb");// binary file
    if (filein == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not find %s ",fileName);
        return false;
    }
	char header[80];
	unsigned int tempNTriangles;
	unsigned short attribute;
	float pt[3],normal[3];
	double ptDouble[3];

	
	if (isoCounter == -1)//default
		isoCounter = MAX(ts_currentIsoCounter, 0);
	if (isoCounter < 0)
		return false;//nothing saved

	ts_isoNPoints[isoCounter]  = 0;
	ts_isoNTriangles[isoCounter] = 0;
	safeFree(ts_isoXYZ[isoCounter]);
	safeFree(ts_isoTriangles[isoCounter]);
	safeFree(ts_isoTrianglesNeighbors[isoCounter]);
	safeFree(ts_isoVertexNormals[isoCounter]);
	safeFree(ts_isoNormals[isoCounter]);
	safeFree(ts_isoPartId[isoCounter]);

	fread(&header,80*sizeof(byte),1,filein);
	fread(&tempNTriangles,sizeof(unsigned int),1,filein);
	ts_isoNTriangles[isoCounter] = 0;// for multi-threading, this may be called so set to 0 until ready with data
	ts_isoTriangles[isoCounter] = (int *)malloc(3*tempNTriangles*sizeof(int));
	ts_isoTrianglesNeighbors[isoCounter] =(int *)malloc(3*tempNTriangles*sizeof(int));
	ts_isoXYZ[isoCounter] = (double *)malloc(9* tempNTriangles *sizeof(double));
	ts_isoVertexNormals[isoCounter] = (double *)malloc(9*tempNTriangles*sizeof(double));
	ts_isoNormals[isoCounter] = (double *)malloc(3* tempNTriangles *sizeof(double));
	ts_isoPartId[isoCounter] = (int *)malloc(tempNTriangles *sizeof(int));

	memset(ts_isoTriangles[isoCounter],0,3* tempNTriangles *sizeof(int));
	memset(ts_isoTrianglesNeighbors[isoCounter],0,3* tempNTriangles *sizeof(int));//this is not available for .stl, see extended off
	memset(ts_isoXYZ[isoCounter],0,9* tempNTriangles *sizeof(double));
	memset(ts_isoVertexNormals[isoCounter],0,9* tempNTriangles *sizeof(double));// this is not available for .stl files
	memset(ts_isoNormals[isoCounter],0,3* tempNTriangles *sizeof(double));
	memset(ts_isoPartId[isoCounter],0, tempNTriangles *sizeof(int));



	for (unsigned int tri = 0; tri < tempNTriangles; tri++) {
		// read the normal 
		fread(&normal[0],sizeof(float),1,filein);
		fread(&normal[1],sizeof(float),1,filein);
		fread(&normal[2],sizeof(float),1,filein);

		ts_isoNormals[isoCounter][3*tri] = normal[0];
		ts_isoNormals[isoCounter][3*tri+1] = normal[1];
		ts_isoNormals[isoCounter][3*tri+2] = normal[2];

		for (int i =0; i < 3; i++) {// read three points
			fread(&pt[0],sizeof(float),1,filein);
			fread(&pt[1],sizeof(float),1,filein);
			fread(&pt[2],sizeof(float),1,filein);
			ptDouble[0] = pt[0]; ptDouble[1] = pt[1]; ptDouble[2] = pt[2];
			node = -1;
			//node = findDuplicateIsoPt(isoCounter,ptDouble);//really slow for large models .. don't worry about duplicates
			if (node < 0) {// no duplicate
				ts_isoXYZ[isoCounter][3*ts_isoNPoints[isoCounter]] = pt[0];
				ts_isoXYZ[isoCounter][3*ts_isoNPoints[isoCounter]+1] = pt[1];
				ts_isoXYZ[isoCounter][3*ts_isoNPoints[isoCounter]+2] = pt[2];
				node = ts_isoNPoints[isoCounter];
				ts_isoNPoints[isoCounter]++;
			}
			ts_isoTriangles[isoCounter][3*tri+i] = node;
		}
		fread(&attribute,sizeof(attribute),1,filein);
	}
	fclose(filein);
	ts_isoNTriangles[isoCounter]  = tempNTriangles;//set this at the very end, when data is ready.
	return true;
}
/************************************************************/
double TopOptIsoSurface::computeIsoSurfaceVolume(int isoCounter)
{
	double xTri[3],yTri[3],zTri[3],triNormal[3],vertexNormals0[3], vertexNormals1[3], vertexNormals2[3],triVol,triArea,triCenter[3];
	double totalVol;

	totalVol  = 0;
	for (int tri = 0; tri < getNumIsoTriangles(isoCounter); tri++) {
		getIsoTriPts(isoCounter,tri,xTri,yTri,zTri,triNormal,vertexNormals0, vertexNormals1, vertexNormals2);
		computeContributionOfTriangle(xTri,yTri,zTri,&triVol,&triArea,triCenter);
		totalVol += triVol;
	}
	return totalVol;
}
/**********************************************************************/
int TopOptIsoSurface::dumpMeshWithTSfield(const char *fileName)
{
    /*
    Print the following (after discarding zero density voxels)
    % 1. #Grid
    % 2. nelx nely nelz
    % 3. #Origin
    % 4. X0 Y0 Z0
    % 5. #Step
    % 6. dx dy dz
    % 7. #Nodes
    % 8. h_nNodes
    % 9. (i,j,k,l) of all nodes (i ranges from 0 to nelx, etc), l is the label of node
    % 10. #Elems
    % 11. h_nElems
    % 12. (n1,n2,...,n8) of all elements
    Optionally ...
    % 13. #ElemId
    % 14 id0,id1,id2,.... for each element, the id (typically part #), default is 0
    Optionally ...
    % 15. #PseudoDensity
    % 16. 1,1,0,1,0,....

    CAVEAT:
    We assume a 0-index, i.e., it is assumed that all node numbers, element numbers, and
    templateIDs start from 0.
    Very limited error check
    */
    FILE *fptr;
    int elem, nElems, node, m,nNodes,nx,ny,nz;


    fptr = fopen(fileName,"w");

    fprintf(fptr,"#Grid\n");
    fprintf(fptr,"%d %d %d \n\n",h_nelx,h_nely,h_nelz);

    int yOffset = (h_nelx+1); // difference in 2 nodes that are next to each other in y
    int zOffset = (h_nelx+1)*(h_nely+1); // difference in 2 nodes that are next to each other in z
    int nodeIndex[8] = {0,1,yOffset+1,yOffset,zOffset,zOffset+1,zOffset+yOffset+1,zOffset+yOffset};

    fprintf(fptr,"#Origin\n");
    fprintf(fptr,"%e %e %e \n\n",h_xMin,h_yMin,h_zMin);

    fprintf(fptr,"#Step\n");
    fprintf(fptr,"%e %e %e \n\n",h_dx,h_dy,h_dz);

    nNodes = h_nNodes;

    fprintf(fptr,"#Nodes\n");
    fprintf(fptr,"%d\n\n",nNodes);

    for (node = 0; node < nNodes; node++) {
        nx = (int) NEARESTINT((h_x[node]-h_xMin)/h_dx);
        ny = (int) NEARESTINT((h_y[node]-h_yMin)/h_dy);
        nz = (int) NEARESTINT((h_z[node]-h_zMin)/h_dz);
        fprintf(fptr,"%d %d %d %d\n",nx,ny,nz,h_nodeLabels[node]);
    }

    fprintf(fptr,"#Elems\n");
    fprintf(fptr,"%d\n\n",nElems);
    for (elem = 0; elem < h_nElems;elem++) {
        for (m = 0; m < 8; m++) {
            node = h_elems[(h_nElems)*m + elem];
            fprintf(fptr,"%d ",node);
        }
        fprintf(fptr,"\n");
    }
    fprintf(fptr,"#TSField\n");
    double scaledTSField;
    for (elem = 0; elem < h_nElems;elem++) {
        scaledTSField = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax - ts_TSMin);
        fprintf(fptr,"%.3f \n",scaledTSField);
    }
    fprintf(fptr,"\n");

    fclose(fptr);
    return 1;
}
//*********************************************
bool TopOptIsoSurface::dumpExtendedOffFile(const char *fileName, int isoCounter)
{
	if (isoCounter == -1) {//default
		if (ts_currentIsoCounter == -1)
			ts_currentIsoCounter = 0;
		isoCounter = MAX(ts_currentIsoCounter, 0);
	}
	if (isoCounter < 0)
		return false;//nothing saved

	FILE *filein;
	filein = fopen(fileName, "w");
	if (filein == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not find %s ", fileName);
		return false;
	}
	
    fprintf(filein, "OFF\n");

    fprintf(filein, "%d %d 0\n", ts_isoNPoints[isoCounter], ts_isoNTriangles[isoCounter]);
	
	for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) 
        fprintf(filein, "%.3e %.3e %.3e \n", ts_isoXYZ[isoCounter][3 * i], ts_isoXYZ[isoCounter][3 * i + 1], ts_isoXYZ[isoCounter][3 * i + 2]);


	for (int i = 0; i < ts_isoNTriangles[isoCounter]; i++) 
        fprintf(filein, "3 %d %d %d \n",  ts_isoTriangles[isoCounter][3 * i], ts_isoTriangles[isoCounter][3 * i+1], ts_isoTriangles[isoCounter][3 * i + 2]);


    fprintf(filein, "VERTEX_NORMALS\n");

	for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) 
        fprintf(filein, "%.3e %.3e %.3e \n", ts_isoVertexNormals[isoCounter][3 * i], ts_isoVertexNormals[isoCounter][3 * i+1], ts_isoVertexNormals[isoCounter][3 * i+2]);
	
    fprintf(filein, "TRIANGLE_NEIGHBORS\n");

	for (int i = 0; i < ts_isoNTriangles[isoCounter]; i++)
        fprintf(filein, "%d %d %d \n", ts_isoTrianglesNeighbors[isoCounter][3 * i], ts_isoTrianglesNeighbors[isoCounter][3 * i + 1], ts_isoTrianglesNeighbors[isoCounter][3 * i + 2]);

	fclose(filein);

	return true;
}
//*********************************************
bool TopOptIsoSurface::readExtendedOffFile(const char *fileName, int isoCounter)
// Read extended .off file, where at the end, the vertex normals are also read (if present). Useful for smooth plotting
/*
OFF
#Vertices #Triangles 0
x0 y0 z0
x1 y1 z1
...

...
3 v0 v1 v2
3 ...

...
VERTEX_NORMALS
n0x n0y n0z
...


TRIANGLE_NEIGHBORS
t0 t1 t2
...
*/
{
	FILE *filein;
	double normal[3],nrm;
	float tempFloat[3];
	int n0,n1,n2;

	if (isoCounter == -1) {//default
		if (ts_currentIsoCounter == -1)
			ts_currentIsoCounter = 0;
		isoCounter = MAX(ts_currentIsoCounter, 0);
	}
	if (isoCounter < 0) 	
		return false;//nothing saved

	filein = fopen(fileName,"r");
    if (filein == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not find %s ",fileName);
        return false;
    }
	if (!find_word(filein,"OFF")){
        writeMessageToParetoLog("TopOptIsoSurface: File %s is missing keyword OFF ",fileName);
		fclose(filein);
        return false;
	}
	int nVertices,nTriangles,tempInt;
	
	
	ts_isoNPoints[isoCounter]  = 0;
	ts_isoNTriangles[isoCounter] = 0;
	safeFree(ts_isoXYZ[isoCounter]);
	safeFree(ts_isoTriangles[isoCounter]);
	safeFree(ts_isoTrianglesNeighbors[isoCounter]);
	safeFree(ts_isoVertexNormals[isoCounter]);
	safeFree(ts_isoNormals[isoCounter]);
	safeFree(ts_isoPartId[isoCounter]);
	fscanf(filein,"%d %d %d ",&nVertices,&nTriangles,&tempInt);


	ts_isoXYZ[isoCounter] = (double *)malloc(3*nVertices*sizeof(double));
	ts_isoVertexNormals[isoCounter] = (double *)malloc(3*nVertices*sizeof(double));
	ts_isoTriangles[isoCounter] = (int *)malloc(3*nTriangles*sizeof(int));
	ts_isoTrianglesNeighbors[isoCounter] = (int *)malloc(3* nTriangles *sizeof(int));
	ts_isoNormals[isoCounter] = (double *)malloc(3* nTriangles *sizeof(double));//calculated
	ts_isoPartId[isoCounter] = (int *)malloc(nTriangles *sizeof(int));

	memset(ts_isoXYZ[isoCounter],0,3*nVertices*sizeof(double));
	memset(ts_isoVertexNormals[isoCounter],0,3*nVertices*sizeof(double));
	memset(ts_isoTriangles[isoCounter],0,3*nTriangles*sizeof(int));
	memset(ts_isoTrianglesNeighbors[isoCounter],0,3* nTriangles *sizeof(int));
	memset(ts_isoNormals[isoCounter],0,3*nTriangles*sizeof(double));
	memset(ts_isoPartId[isoCounter],0,nTriangles*sizeof(int));

	for (int i = 0; i < nVertices; i++) {
		fscanf(filein, "%e %e %e ", &(tempFloat[0]), &(tempFloat[1]), &(tempFloat[2]));
		ts_isoXYZ[isoCounter][3 * i] = tempFloat[0];
		ts_isoXYZ[isoCounter][3 * i + 1] = tempFloat[1];
		ts_isoXYZ[isoCounter][3 * i + 2] = tempFloat[2];
	}
	for (int i = 0; i < nTriangles; i++) {
		fscanf(filein, "%d %d %d %d ", &tempInt, &n0, &n1, &n2);
		if ((n0 >= nVertices) || (n0 >= nVertices) || (n0 >= nVertices)) {
			cout << "Incorrect vertex in Off file " << endl;
			continue;
		}
		ts_isoTriangles[isoCounter][3 * i] = n0;
		ts_isoTriangles[isoCounter][3 * i + 1] = n1;
		ts_isoTriangles[isoCounter][3 * i + 2] = n2;
	}

	if (find_word(filein,"VERTEX_NORMALS")){// extended off format
		for (int i = 0; i < nVertices; i++) {
			fscanf(filein, "%e %e %e ", &(tempFloat[0]), &(tempFloat[1]), &(tempFloat[2]));
			ts_isoVertexNormals[isoCounter][3 * i] = tempFloat[0];
			ts_isoVertexNormals[isoCounter][3 * i + 1] = tempFloat[1];
			ts_isoVertexNormals[isoCounter][3 * i + 2] = tempFloat[2];
		}
		for ( int i = 0; i < nTriangles; i++) {
			n0 = ts_isoTriangles[isoCounter][3*i];
			n1 = ts_isoTriangles[isoCounter][3*i+1];
			n2 = ts_isoTriangles[isoCounter][3*i+2];
			if ((n0 >= nVertices) || (n0 >= nVertices) || (n0 >= nVertices)) {
				cout << "Incorrect vertex in Off file " << endl;
				continue;
			}
			if ((n0 < 0) || (n1 < 0) || (n2 < 0)) {
				cout << "Incorrect vertex in Off file " << endl;
				continue;
			}
			if ((n0 == n1) || (n0 == n2) || (n1 == n2)) {
				cout << "Incorrect vertex in Off file " << endl;
				continue;
			}

			// compute the triangle normal as average of vertex normals
			normal[0] = (ts_isoVertexNormals[isoCounter][3*n0] + ts_isoVertexNormals[isoCounter][3*n1] + ts_isoVertexNormals[isoCounter][3*n2])/3.0;
			normal[1] = (ts_isoVertexNormals[isoCounter][3*n0+1] + ts_isoVertexNormals[isoCounter][3*n1+1] + ts_isoVertexNormals[isoCounter][3*n2+1])/3.0;
			normal[2] = (ts_isoVertexNormals[isoCounter][3*n0+2] + ts_isoVertexNormals[isoCounter][3*n1+2] + ts_isoVertexNormals[isoCounter][3*n2+2])/3.0;
			nrm = NORM3D(normal);

			ts_isoNormals[isoCounter][3*i] = normal[0]/nrm;
			ts_isoNormals[isoCounter][3*i+1] = normal[1]/nrm;
			ts_isoNormals[isoCounter][3*i+2] = normal[2]/nrm;
		}
	}
	if (find_word(filein,"TRIANGLE_NEIGHBORS")){// extended off format
		for ( int i = 0; i < nTriangles; i++) 
			fscanf(filein,"%d %d %d ",&(ts_isoTrianglesNeighbors[isoCounter][3*i]),&(ts_isoTrianglesNeighbors[isoCounter][3*i+1]),&(ts_isoTrianglesNeighbors[isoCounter][3*i+2]));
		ts_isoNTriangles[isoCounter] = nTriangles;// this is used in  assignPartNumberIsoSurfaceTriangles
        assignPartNumberIsoSurfaceTriangles(isoCounter);
	}
	
	fclose(filein);
	ts_isoNPoints[isoCounter] = nVertices;
	ts_isoNTriangles[isoCounter] = nTriangles;// this is used in  assignPartNumberIsoSurfaceTriangles
	return true;
}
//*********************************************
bool TopOptIsoSurface::readAndAppendFromExtendedOffFile(const char *fileName, int isoCounter)
// Read extended .off file, where at the end, the vertex normals are also read (if present). Useful for smooth plotting
// append to current isocounter
/*
OFF
#Vertices #Triangles 0
x0 y0 z0
x1 y1 z1
...

...
3 v0 v1 v2
3 ...

...
VERTEX_NORMALS
n0x n0y n0z
...


TRIANGLE_NEIGHBORS
t0 t1 t2
...
*/
{
    FILE *filein;
    double normal[3],nrm;
    float tempFloat[3];
    int n0,n1,n2;

    isoCounter = MAX(ts_currentIsoCounter, 0) ;

    cout << "Append isoCounter = " << isoCounter << endl;
    filein = fopen(fileName,"r");
    if (filein == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not find %s ",fileName);
        return false;
    }
    if (!find_word(filein,"OFF")){
        writeMessageToParetoLog("TopOptIsoSurface: File %s is missing keyword OFF ",fileName);
        fclose(filein);
        return false;
    }
    int nVertices,nTriangles,tempInt;


    // we do NOT reset any of the vertex counts, etc to zero
    int nPriorPoints = ts_isoNPoints[isoCounter];
    int nPriorTriangles = ts_isoNTriangles[isoCounter];



    if ((nPriorPoints == 0) || (nPriorTriangles = 0))
        return false; // cannot append to an empty data structure

    fscanf(filein,"%d %d %d ",&nVertices,&nTriangles,&tempInt);//current

    if ((nVertices == 0) || (nTriangles = 0)){
         fclose(filein);
        return false; // cannot append to an empty data structure
    }

    int totalVertices = nVertices + nPriorPoints;
    int totalTriangles = nTriangles + nPriorTriangles;

    ts_isoXYZ[isoCounter] = (double *)realloc(ts_isoXYZ[isoCounter],3*totalVertices*sizeof(double));
    ts_isoVertexNormals[isoCounter] = (double *)realloc(ts_isoVertexNormals[isoCounter],3*totalVertices*sizeof(double));
    ts_isoTriangles[isoCounter] = (int *)realloc(ts_isoTriangles[isoCounter],3*totalTriangles*sizeof(int));
    ts_isoTrianglesNeighbors[isoCounter] = (int *)realloc(ts_isoTrianglesNeighbors[isoCounter],3* totalTriangles *sizeof(int));
    ts_isoNormals[isoCounter] = (double *)realloc(ts_isoNormals[isoCounter],3* totalTriangles *sizeof(double));//calculated
    ts_isoPartId[isoCounter] = (int *)realloc(ts_isoPartId[isoCounter], totalTriangles *sizeof(int));

    for (int i = 0; i < nVertices; i++) {
        fscanf(filein, "%e %e %e ", &(tempFloat[0]), &(tempFloat[1]), &(tempFloat[2]));
        ts_isoXYZ[isoCounter][3 * (nPriorPoints+i)] = tempFloat[0];
        ts_isoXYZ[isoCounter][3 * (nPriorPoints+i) + 1] = tempFloat[1];
        ts_isoXYZ[isoCounter][3 * (nPriorPoints+i) + 2] = tempFloat[2];
    }

    for (int i = 0; i < nTriangles; i++) {
        fscanf(filein, "%d %d %d %d ", &tempInt, &n0, &n1, &n2);
        if ((n0 >= nVertices) || (n0 >= nVertices) || (n0 >= nVertices)) {
            cout << "Incorrect vertex in Off file " << endl;
            continue;
        }
        ts_isoTriangles[isoCounter][3 * (nPriorTriangles + i)] = n0+nPriorPoints;
        ts_isoTriangles[isoCounter][3 * (nPriorTriangles + i) + 1] = n1+nPriorPoints;
        ts_isoTriangles[isoCounter][3 * (nPriorTriangles + i) + 2] = n2+nPriorPoints;
    }

    if (find_word(filein,"VERTEX_NORMALS")){// extended off format
        for (int i = 0; i < nVertices; i++) {
            fscanf(filein, "%e %e %e ", &(tempFloat[0]), &(tempFloat[1]), &(tempFloat[2]));
            ts_isoVertexNormals[isoCounter][3 *  (nPriorPoints+i)] = tempFloat[0];
            ts_isoVertexNormals[isoCounter][3 *  (nPriorPoints+i) + 1] = tempFloat[1];
            ts_isoVertexNormals[isoCounter][3 *  (nPriorPoints+i) + 2] = tempFloat[2];
        }
        for ( int i = 0; i < nTriangles; i++) {
            n0 = ts_isoTriangles[isoCounter][3*(nPriorTriangles + i)];// n0 is already offset by nPriorPoints
            n1 = ts_isoTriangles[isoCounter][3*(nPriorTriangles + i)+1];
            n2 = ts_isoTriangles[isoCounter][3*(nPriorTriangles + i)+2];


            // compute the triangle normal as average of vertex normals
            normal[0] = (ts_isoVertexNormals[isoCounter][3*n0] + ts_isoVertexNormals[isoCounter][3*n1] + ts_isoVertexNormals[isoCounter][3*n2])/3.0;
            normal[1] = (ts_isoVertexNormals[isoCounter][3*n0+1] + ts_isoVertexNormals[isoCounter][3*n1+1] + ts_isoVertexNormals[isoCounter][3*n2+1])/3.0;
            normal[2] = (ts_isoVertexNormals[isoCounter][3*n0+2] + ts_isoVertexNormals[isoCounter][3*n1+2] + ts_isoVertexNormals[isoCounter][3*n2+2])/3.0;
            nrm = NORM3D(normal);

            ts_isoNormals[isoCounter][3*(nPriorTriangles + i)] = normal[0]/nrm;
            ts_isoNormals[isoCounter][3*(nPriorTriangles + i)+1] = normal[1]/nrm;
            ts_isoNormals[isoCounter][3*(nPriorTriangles + i)+2] = normal[2]/nrm;
        }
    }

    if (find_word(filein,"TRIANGLE_NEIGHBORS")){// extended off format
        for ( int i = 0; i < nTriangles; i++){
            fscanf(filein,"%d %d %d ",&n0,&n1,&n2);
            ts_isoTrianglesNeighbors[isoCounter][3*(nPriorTriangles + i)] = n0;
            ts_isoTrianglesNeighbors[isoCounter][3*(nPriorTriangles + i)+1] = n1;
            ts_isoTrianglesNeighbors[isoCounter][3*(nPriorTriangles + i)+2] = n2;
        }

        ts_isoNTriangles[isoCounter] = nPriorTriangles + nTriangles;// this is used in  assignPartNumberIsoSurfaceTriangles
        assignPartNumberIsoSurfaceTriangles(isoCounter);
    }

    fclose(filein);
    cout << "Done reading file " << endl;
    ts_isoNPoints[isoCounter] = nVertices + nPriorPoints;
    ts_isoNTriangles[isoCounter] = nTriangles + nPriorTriangles;// this is used in  assignPartNumberIsoSurfaceTriangles
    return true;
}
/**********************************************************************/
void TopOptIsoSurface::assignPartNumberIsoSurfaceTriangles(int isoCounter)
{
    if ((getNumberOfParts() == 1) && (getNumMaterials() == 1)) return;// all triangles belong to zero

	int t1, nParts,currentTri,neighborTri,i;
	bool *inQueue;
	Queue Q;
	
	inQueue = (bool *)malloc((ts_isoNTriangles[isoCounter])*sizeof(bool));
	memset(inQueue,false,ts_isoNTriangles[isoCounter]*sizeof(bool));
	
	for( t1 =0; t1 < ts_isoNTriangles[isoCounter]; t1++) 
		ts_isoPartId[isoCounter][t1] = -1;

	Q = CreateQueue(ts_isoNTriangles[isoCounter]);
	MakeEmpty(Q);
	nParts = 0;

	for( t1 =0; t1 < ts_isoNTriangles[isoCounter]; t1++) {
		if (ts_isoPartId[isoCounter][t1] == -1) {	
			Enqueue(t1, Q);
			while (!IsEmpty(Q)) {
				currentTri = Front(Q);
				Dequeue(Q);
				ts_isoPartId[isoCounter][currentTri] = nParts;
				for (i =0; i < 3; i++) {
					neighborTri = ts_isoTrianglesNeighbors[isoCounter][3*currentTri + i];
					if ((!inQueue[neighborTri]) && (ts_isoPartId[isoCounter][neighborTri] == -1)) {
						Enqueue(neighborTri, Q);
						inQueue[neighborTri] = true;
					}
				}
			}
			nParts++;
		}
	}

	DisposeQueue(Q);
	free(inQueue);
    cout << "\nassignPartNumberIsoSurfaceTriangles: NParts assigned = " << nParts << endl;
}
/**********************************************************************/
bool TopOptIsoSurface::pushFinalStaticSolutionToIsoSurface(void)
{
	double pt[3],uvw[3],stresses[6],vonMises;
	int isoCounter;

	isoCounter = getCurrentIsoCounter();

	if (isoCounter < 0) return false;
	if (ts_isoTriangles[isoCounter] == 0) return false;

	safeFree(ts_isoUVWFinal);
	ts_isoUVWFinal = (double *)malloc(9*ts_isoNTriangles[isoCounter]*sizeof(double));

	safeFree(ts_isoVonMisesFinal);
	ts_isoVonMisesFinal = (double *)malloc(3*ts_isoNTriangles[isoCounter]*sizeof(double));



	for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) {
		pt[0] =ts_isoXYZ[isoCounter][3*i];
		pt[1] =ts_isoXYZ[isoCounter][3*i+1];
		pt[2] =ts_isoXYZ[isoCounter][3*i+2];
		getElasticitySolutionAt(pt,uvw,stresses,&vonMises);
		for (int j = 0; j < 3;j++)
			ts_isoUVWFinal[3*i+j] = uvw[j];

		ts_isoVonMisesFinal[i] = vonMises;
	}
	
	return true;
}
/**********************************************************************/
bool TopOptIsoSurface::pushFinalTemperatureToIsoSurface(void)
{
    double pt[3],temperature;
    int isoCounter;

    isoCounter = getCurrentIsoCounter();

    if (isoCounter < 0) return false;
    if (ts_isoTriangles[isoCounter] == 0) return false;

    safeFree(ts_isoTemperatureFinal);
    ts_isoTemperatureFinal = (double *)malloc(3*ts_isoNTriangles[isoCounter]*sizeof(double));

    for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) {
        pt[0] = ts_isoXYZ[isoCounter][3*i];
        pt[1] = ts_isoXYZ[isoCounter][3*i+1];
        pt[2] = ts_isoXYZ[isoCounter][3*i+2];
        getTemperatureAt(pt,&temperature);
        ts_isoTemperatureFinal[i] = temperature;
    }

    return true;
}
/**********************************************************************/
bool TopOptIsoSurface::pushFinalModalSolutionToIsoSurface(void)
{
    double pt[3],uvw[3],stresses[6],vonMises;
    int isoCounter;

    isoCounter = getCurrentIsoCounter();

    if (isoCounter < 0) return false;
    if (ts_isoTriangles[isoCounter] == 0) return false;

    safeFree(ts_isoModalFinal);
    ts_isoModalFinal = (double *)malloc(9*ts_isoNTriangles[isoCounter]*sizeof(double));

    int eigenMode;
    eigenMode = 0;
    for (int i = 0; i < ts_isoNPoints[isoCounter]; i++) {
        pt[0] = ts_isoXYZ[isoCounter][3*i];
        pt[1] = ts_isoXYZ[isoCounter][3*i+1];
        pt[2] = ts_isoXYZ[isoCounter][3*i+2];
        getModalSolutionAt(pt,eigenMode,uvw);
        for (int j = 0; j < 3;j++)
            ts_isoModalFinal[3*i+j] = uvw[j];
    }

    return true;
}
//*********************************************
bool TopOptIsoSurface::getFinalIsoElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3],double vonMises[3])
{
	int nodes[3],isoCounter;

	isoCounter = getCurrentIsoCounter();
	if (isoCounter < 0) return false;
	if (ts_isoTriangles[isoCounter] == 0) return false;

	for (int i = 0; i < 3; i++)
		nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

	for (int i = 0; i < 3; i++){
		u[i] = ts_isoUVWFinal[3*nodes[i]];
		v[i] = ts_isoUVWFinal[3*nodes[i]+1];
		w[i] = ts_isoUVWFinal[3*nodes[i]+2];
		vonMises[i] = ts_isoVonMisesFinal[nodes[i]];
	}
	return true;
}
//*********************************************
bool TopOptIsoSurface::getFinalIsoTemperatureOfTriangle(int tri,double temperature[3])
{
    int nodes[3],isoCounter;

    isoCounter = getCurrentIsoCounter();
    if (isoCounter < 0) return false;
    if (ts_isoTriangles[isoCounter] == 0) return false;

    for (int i = 0; i < 3; i++)
        nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

    for (int i = 0; i < 3; i++)
        temperature[i] = ts_isoTemperatureFinal[nodes[i]];
    return true;
}
//*********************************************
bool TopOptIsoSurface::getFinalModalSolutionOfTriangle(int tri,double u[3],double v[3],double w[3])
{
    int nodes[3],isoCounter;

    isoCounter = getCurrentIsoCounter();
    if (isoCounter < 0) return false;
    if (ts_isoTriangles[isoCounter] == 0) return false;

    for (int i = 0; i < 3; i++)
        nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

    for (int i = 0; i < 3; i++){
        u[i] = ts_isoModalFinal[3*nodes[i]];
        v[i] = ts_isoModalFinal[3*nodes[i]+1];
        w[i] = ts_isoModalFinal[3*nodes[i]+2];
    }
    return true;
}
//*********************************************
bool TopOptIsoSurface::dumpIsoOffFormat(int isoCounter,const char *fileName,double scale)
// Dump sosurface in off format

{
	FILE *fptr = fopen(fileName,"w");
    fprintf(fptr,"OFF\n");
	int nVertices = ts_isoNPoints[isoCounter];
	int nTriangles = ts_isoNTriangles[isoCounter];
    fprintf(fptr,"%d %d 0\n",nVertices,nTriangles);
	for(int i = 0; i < nVertices; i++)
        fprintf(fptr,"%.3e %.3e %.3e \n",scale*ts_isoXYZ[isoCounter][3*i],scale*ts_isoXYZ[isoCounter][3*i+1],scale*ts_isoXYZ[isoCounter][3*i+2]);

	for(int i = 0; i < nTriangles; i++) 
        fprintf(fptr,"3 %d %d %d \n",ts_isoTriangles[isoCounter][3*i],ts_isoTriangles[isoCounter][3*i+1],ts_isoTriangles[isoCounter][3*i+2]);
	
	fclose(fptr);
	return true;
}
//*********************************************
bool TopOptIsoSurface::dumpIsoBinary(int isoCounter, const char *fileName,double scale)
// Dump sosurface as .stl file (binary)
/*
UINT8[80] – Header
UINT32 – Number of triangles

foreach triangle
REAL32[3] Normal vector
REAL32[3] Vertex 1
REAL32[3] Vertex 2
REAL32[3] Vertex 3
UINT16 – Attribute byte count
end*/
{
	int  nodes[3];
	char header[80];
	unsigned int nTriangles;
	double pt0[3],pt1[3],pt2[3],normal[3],pt01[3],pt02[3],nrm;
	float fbuf[12];
	FILE *fptr;
	unsigned short att = 0;

    if (ts_verbose) writeMessageToParetoLog("TopOptIsoSurface: Writing Isosurface ... ");
	fptr = fopen(fileName,"wb");// binary file
    if (fptr == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not create %s for writing ",fileName);
        return false;
    }
	memset(header,'0', 80*sizeof(unsigned char));// dummy header
	fwrite(header,80,1, fptr);
	nTriangles = ts_isoNTriangles[isoCounter];
	fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

	for (unsigned int tri = 0; tri < nTriangles; tri++) {
		for (int i = 0; i < 3; i++)
			nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

		for (int i = 0; i < 3; i++) {
			pt0[i] = scale*ts_isoXYZ[isoCounter][3*nodes[0]+i];
			pt1[i] = scale*ts_isoXYZ[isoCounter][3*nodes[1]+i];
			pt2[i] = scale*ts_isoXYZ[isoCounter][3*nodes[2]+i];
		}
		SUBTRACT3D(pt01,pt1,pt0);
		SUBTRACT3D(pt02,pt2,pt0);
		CROSS3D(normal,pt01,pt02);

		nrm = NORM3D(normal);

		fbuf[0] = (float) (normal[0]/nrm) ; 
		fbuf[1] = (float) (normal[1]/nrm) ; 
		fbuf[2] = (float) (normal[2]/nrm) ;

		fbuf[3]  = (float) (pt0[0]);
		fbuf[4]  = (float) (pt0[1]);
		fbuf[5]  = (float) (pt0[2]);

		fbuf[6]  = (float) (pt1[0]);
		fbuf[7]  = (float) (pt1[1]);
		fbuf[8]  = (float) (pt1[2]);

		fbuf[9]   = (float) (pt2[0]);
		fbuf[10]  = (float) (pt2[1]);
		fbuf[11]  = (float) (pt2[2]);

		fwrite(fbuf, 12*sizeof(float), 1, fptr);
		fwrite(&att,sizeof(unsigned short),1,fptr);
	}
	fclose(fptr);
	return true;
}
//*********************************************
bool TopOptIsoSurface::dumpSolidWorksVRML(int isoCounter,const char *fileName,double scale)
// Dump topopt iso surface as an VRML (.wrl) as file (ascii)
/*
#VRML V1.0 ascii
Separator {
MaterialBinding {
value OVERALL
}
Material {
ambientColor [
 0.792157 0.819608 0.933333
]
diffuseColor [
 0.792157 0.819608 0.933333
]
emissiveColor [
 0.000000 0.000000 0.000000
]
specularColor [
 0.396078 0.409804 0.466667
]
shininess [
 0.400000
]
transparency [
 0.000000
]
}
Coordinate3 {
point [
x0 y0 z0, x1 y1 z1,...
]
}
IndexedFaceSet {
coordIndex [
 t0_0,t0_1,t0_2, -1, t1_0, ...
]
}
}

*/
{
	int  nodes[3];
	unsigned int nTriangles,nNodes;
	double x,y,z;
	FILE *fptr;
	unsigned short att = 0;

	if (ts_verbose) cout << "TopOptIsoSurface: Writing VRML ..." << endl;
	fptr = fopen(fileName,"w");// ascii file
    if (fptr == 0) {
        writeMessageToParetoLog("TopOptIsoSurface: Could not create %s for writing ",fileName);
        return false;
    }
    //fprintf(fptr,"#VRML V1.0 ascii\nSeparator {\nMaterialBinding {\nvalue OVERALL\n}\nMaterial {\nambientColor [ \n0.792157 0.819608 0.933333\n]\ndiffuseColor [\n 0.792157 0.819608 0.933333\n]\nemissiveColor [\n 0.000000 0.000000 0.000000\n]\nspecularColor [\n 0.396078 0.409804 0.466667\n]\nshininess [\n 0.400000\n]\ntransparency [\n 0.000000\n]\n}\n");

    fprintf(fptr,"#VRML V2.0 utf8\nTransform {\nchildren [\nShape {\nappearance Appearance {\nmaterial Material {\nambientIntensity 1.000000\ndiffuseColor 0.792157 0.819608 0.933333\nemissiveColor 0.000000 0.000000 0.000000\nspecularColor 0.396078 0.409804 0.466667\nshininess 0.400000\ntransparency 0.000000\n}\n}\ngeometry IndexedFaceSet {\ncolor Color {\ncolor [\n 0.792157 0.819608 0.933333\n]\n}\n");
	nTriangles = ts_isoNTriangles[isoCounter];
	nNodes = ts_isoNPoints[isoCounter];
    fprintf(fptr,"coord Coordinate {\npoint [\n");
	for (unsigned int node = 0; node < nNodes; node++) {
		x = scale*ts_isoXYZ[isoCounter][3*node];
		y = scale*ts_isoXYZ[isoCounter][3*node+1];
		z = scale*ts_isoXYZ[isoCounter][3*node+2];
        fprintf(fptr," %g %g %g",x,y,z);
		if (node < nNodes-1)
            fprintf(fptr,", ");
		else
            fprintf(fptr,"\n]\n}\n");

		if (node%4 == 0)
            fprintf(fptr,"\n");
	}
    fprintf(fptr,"\n");
    fprintf(fptr,"coordIndex [\n");
	for (unsigned int tri = 0; tri < nTriangles; tri++) {
		for (int i = 0; i < 3; i++)
			nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

        fprintf(fptr, "%d, %d, %d, -1",nodes[0],nodes[1],nodes[2]);
		if (tri < nTriangles-1)
            fprintf(fptr,", ");
		else
            fprintf(fptr,"\n]\ncolorPerVertex FALSE\nnormalPerVertex FALSE\n");

		if (tri%4 == 0)
            fprintf(fptr,"\n");
	}
    fprintf(fptr,"\n}\n}\n]\n}\n");
	fclose(fptr);
	return true;
}
/******************************************************************/
bool TopOptIsoSurface::createAndDumpIsoSurfaceFromPseudoDensity(const char *fileName,double cutoff)
{
    // Used  both for plotting isosurfaces (if fptr is NULL) and for saving isosurfaces (not NULL)
    int elem,ntri;
    double x[8],y[8],z[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3],nrm,pseudoDensityAtNodes[8];
    GRIDCELL grid;
    TRIANGLE triangles[10];
    float fbuf[12];
    int nodes[8];
    unsigned short att = 0;
    unsigned int nTriangles;

    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL) {
        writeMessageToParetoLog("Unable to open STL file: %s ",fileName);
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;

    for (elem = 0; elem < getNumElems(); elem++) {
        getElemNodes(elem,nodes);
        getElemDensityAtElemNodes(elem,pseudoDensityAtNodes);
        getElemNodeLocation(elem,x,y,z);
        for (int m = 0;m < 8;m++) {
            grid.p[m].x = x[m];
            grid.p[m].y = y[m];
            grid.p[m].z = z[m];
            grid.val[m] = pseudoDensityAtNodes[m];
            if (isNodeOnBoundary(nodes[m]))
                grid.val[m] = 0.01;
        }
        ntri = PolygoniseCube(grid,cutoff,triangles);

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
    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);
    return true;
}
/******************************************************************/
bool TopOptIsoSurface::createAndDumpIsoSurfaceFromTSfield(const char *fileName,int isoCounter,bool invert)
{
	// Used  both for plotting isosurfaces (if fptr is NULL) and for saving isosurfaces (not NULL)
    int elem,ntri;
	double x[8],y[8],z[8],TS[8],p0[3],p1[3],p2[3],p01[3],p02[3],normal[3],TSIsoCutoff,nrm;
	GRIDCELL grid;
	TRIANGLE triangles[10];
	float fbuf[12];
	unsigned short att = 0;
	unsigned int nTriangles;

	if (!doesTSExist()) return false;
    if (isoCounter < 0)
        isoCounter = ts_currentIsoCounter;
    if (isoCounter < 0) 	return false;


    fixNodalTSValues(isoCounter,invert);// first fix the nodal TS values for proper iso surface generation.
	FILE *fptr = fopen(fileName,"wb");
	if (fptr == NULL) {
        writeMessageToParetoLog("Unable to open STL file: %s ",fileName);
		return false;
	}
	char header[80];
	memset(header,'0', 80*sizeof( char));// dummy header
	fwrite(header,80,1, fptr);
	nTriangles = 0; // dummy, will overwrite later on
	fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
	
	nTriangles = 0;
    TSIsoCutoff = ts_isoValueSave[isoCounter];
	for (elem = 0; elem < getNumElems(); elem++) {
        getTopSensAtElemNodesAtCounter(elem,isoCounter,TS);
		getElemNodeLocation(elem,x,y,z);
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
	fseek(fptr,80,SEEK_SET);
	// write the number of triangles
	fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);	
	fclose(fptr);
	return true;
}
/******************************************************************/
bool TopOptIsoSurface::createAndDumpIsoSurfaceFromMesh(const char *fileName,int savedCounter)
{
    int elem,neighboringElem,neighboringElemsOfElem[27];
    double x[8],y[8],z[8];
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
    float fbuf[12];
    unsigned short att = 0;
    unsigned int nTriangles;
    bool neighboringElemsExists[7];


    if (!doesTSExist()) return false;
    if (savedCounter == -1)//default
        savedCounter = ts_currentIsoCounter;
    if (savedCounter < 0) 	return false;//nothing saved

    if (ts_verbose) cout << "TopOptIsoSurface: createAndDumpIsoSurfaceFromMesh savedCounter = " << savedCounter << endl;
    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL) {
        writeMessageToParetoLog("Unable to open STL file: %s ",fileName);
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;
    for (int partId =0; partId < getNumberOfParts(); partId++) {
        if (!shouldPartBeOptimized(partId)) continue;
        for (elem = 0; elem < getNumElems(); elem++) {
            if (!doesElemExistsAtTSCounter(elem,savedCounter)) continue;
            if (getPartIdOfElem(elem) != partId) continue;

            getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
            // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
            neighboringElemsExists[0] = true;//self
            for (int m = 1; m < 7; m++) {// start with 1 for neighbors
                neighboringElemsExists[m] = false;// means we need to generate the quad surface with this neighbor
                neighboringElem = neighboringElemsOfElem[m];
                if (neighboringElem == -1) continue;

                if (!doesElemExistsAtTSCounter(neighboringElem,savedCounter)) continue;
                if (!shouldPartBeOptimized(getPartIdOfElem(neighboringElem))) continue;
                if ((getNumberOfParts() > 1) && (getPartIdOfElem(neighboringElem) != partId)) continue;

                neighboringElemsExists[m] = true;// this means we don't need to generate the quad on this boundary
            }
            getElemNodeLocation(elem,x,y,z);
            for (int m = 0; m < 6;m++) {// consider the six quads
                // make sure all 4 nodes are on boundary
                if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists
                // we don't care about normal
                fbuf[0] = 1.0f;
                fbuf[1] = 0.0f;
                fbuf[2] = 0.0f;

                // first traingle
                fbuf[3]  = (float) x[quads[m][0]];
                fbuf[4]  = (float) y[quads[m][0]];
                fbuf[5]  = (float) z[quads[m][0]];

                fbuf[6]  = (float) x[quads[m][1]];
                fbuf[7]  = (float) y[quads[m][1]];
                fbuf[8]  = (float) z[quads[m][1]];

                fbuf[9]  = (float) x[quads[m][2]];
                fbuf[10]  = (float) y[quads[m][2]];
                fbuf[11]  = (float) z[quads[m][2]];

                fwrite(fbuf, 12*sizeof(float), 1, fptr);
                fwrite(&att,sizeof(unsigned short),1,fptr);

                // second triangle

                fbuf[3]  = (float) x[quads[m][0]];
                fbuf[4]  = (float) y[quads[m][0]];
                fbuf[5]  = (float) z[quads[m][0]];

                fbuf[6]  = (float) x[quads[m][2]];
                fbuf[7]  = (float) y[quads[m][2]];
                fbuf[8]  = (float) z[quads[m][2]];

                fbuf[9]  = (float) x[quads[m][3]];
                fbuf[10]  = (float) y[quads[m][3]];
                fbuf[11]  = (float) z[quads[m][3]];

                fwrite(fbuf, 12*sizeof(float), 1, fptr);
                fwrite(&att,sizeof(unsigned short),1,fptr);

                nTriangles +=2;
            }
        }
    }
    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);

    if (ts_verbose) cout << "TopOptIsoSurface: createAndDumpIsoSurfaceFromMesh saved " << endl;
    return true;
}
/******************************************************************/
bool TopOptIsoSurface::createAndDumpNegativeIsoSurfaceFromMesh(const char *fileName,int savedCounter)
{
    int elem,neighboringElem,neighboringElemsOfElem[27];
	double x[8],y[8],z[8];
	int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
	float fbuf[12];
	unsigned short att = 0;
    unsigned int nTriangles;
    bool neighboringElemDoesNotExist[7];

    // Dumps the negative component as a triangulation (.stl)

    if (!doesTSExist()) return false;
    if (savedCounter == -1)//default
        savedCounter = ts_currentIsoCounter;
    if (savedCounter < 0) 	return false;//nothing saved

    if ((strstr(fileName, ".stl") == NULL) && (strstr(fileName, ".STL") == NULL) ){
        writeMessageToParetoLog("Output must be an stl file: %s ",fileName);
        return false;
    }
    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL) {
        writeMessageToParetoLog("Unable to open file: %s ",fileName);
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;

    for (elem = 0; elem < getNumElems(); elem++) {
        if (doesElemExistsAtTSCounter(elem,savedCounter)) continue;//skip all elements in
        getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemDoesNotExist[0] = true;//self
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemDoesNotExist[m] = false;// element exists, means we need to generate the quad surface with this neighbor
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;

            if (doesElemExistsAtTSCounter(neighboringElem,savedCounter)) continue;
            neighboringElemDoesNotExist[m] = true;// this means we don't need to generate the quad on this boundary
        }
        getElemNodeLocation(elem,x,y,z);
        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemDoesNotExist[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists
            // we don't care about normal
            fbuf[0] = 1.0f;
            fbuf[1] = 0.0f;
            fbuf[2] = 0.0f;

            // first traingle
            fbuf[3]  = (float) x[quads[m][0]];
            fbuf[4]  = (float) y[quads[m][0]];
            fbuf[5]  = (float) z[quads[m][0]];

            fbuf[6]  = (float) x[quads[m][1]];
            fbuf[7]  = (float) y[quads[m][1]];
            fbuf[8]  = (float) z[quads[m][1]];

            fbuf[9]  = (float) x[quads[m][2]];
            fbuf[10]  = (float) y[quads[m][2]];
            fbuf[11]  = (float) z[quads[m][2]];

            fwrite(fbuf, 12*sizeof(float), 1, fptr);
            fwrite(&att,sizeof(unsigned short),1,fptr);

            // second triangle

            fbuf[3]  = (float) x[quads[m][0]];
            fbuf[4]  = (float) y[quads[m][0]];
            fbuf[5]  = (float) z[quads[m][0]];

            fbuf[6]  = (float) x[quads[m][2]];
            fbuf[7]  = (float) y[quads[m][2]];
            fbuf[8]  = (float) z[quads[m][2]];

            fbuf[9]  = (float) x[quads[m][3]];
            fbuf[10]  = (float) y[quads[m][3]];
            fbuf[11]  = (float) z[quads[m][3]];

            fwrite(fbuf, 12*sizeof(float), 1, fptr);
            fwrite(&att,sizeof(unsigned short),1,fptr);

            nTriangles +=2;
        }
    }

    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);

    if (ts_verbose) writeMessageToParetoLog("TopOptIsoSurface: File %s saved ",fileName );
    return true;
}
/******************************************************************/
bool TopOptIsoSurface::createAndDumpIsoSurfaceFromMesh_MultiMaterial(const char *fileName,int materialToSave,int savedCounter)
{
    int elem,neighboringElem,neighboringElemsOfElem[27],matIdThisElem;
    double x[8],y[8],z[8];
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
    float fbuf[12];
    unsigned short elemAttribute = 0;
    unsigned int nTriangles;
    bool neighboringElemsExists[7];


    if (!doesTSExist()) return false;
    if (savedCounter == -1)//default
        savedCounter = ts_currentIsoCounter;
    if (savedCounter < 0) 	return false;//nothing saved

    if (ts_verbose) cout << "TopOptIsoSurface: createAndDumpIsoSurfaceFromMesh savedCounter = " << savedCounter << endl;
    FILE *fptr = fopen(fileName,"wb");
    if (fptr == NULL) {
        cout << "Unable to open STL file: " << fileName << endl;
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;
    for (int partId =0; partId < getNumberOfParts(); partId++) {
        if (!shouldPartBeOptimized(partId)) continue;
        for (elem = 0; elem < getNumElems(); elem++) {
            if (getElemPseudoDensity(elem) < 0.5) continue;
            if (getPartIdOfElem(elem) != partId) continue;

            matIdThisElem = getMaterialIdOfElem(elem);
            if (matIdThisElem != materialToSave) continue;
            elemAttribute = (unsigned short)matIdThisElem;
            getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
            // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
            neighboringElemsExists[0] = true;//self
            for (int m = 1; m < 7; m++) {// start with 1 for neighbors
                neighboringElemsExists[m] = false;// means we need to generate the quad surface with this neighbor
                neighboringElem = neighboringElemsOfElem[m];
                if (neighboringElem == -1) continue;

                if (getElemPseudoDensity(neighboringElem) < 0.5) continue;
                if (!shouldPartBeOptimized(getPartIdOfElem(neighboringElem))) continue;
                if ((getNumberOfParts() > 1) && (getPartIdOfElem(neighboringElem) != partId)) continue;

                if (getMaterialIdOfElem(neighboringElem) != matIdThisElem) continue;
                neighboringElemsExists[m] = true;// this means we don't need to generate the quad on this boundary
            }

            getElemNodeLocation(elem,x,y,z);

            for (int m = 0; m < 6;m++) {// consider the six quads
                // make sure all 4 nodes are on boundary
                if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists
                // we don't care about normal
                fbuf[0] = 1.0f;
                fbuf[1] = 0.0f;
                fbuf[2] = 0.0f;

                // first traingle
                fbuf[3]  = (float) x[quads[m][0]];
                fbuf[4]  = (float) y[quads[m][0]];
                fbuf[5]  = (float) z[quads[m][0]];

                fbuf[6]  = (float) x[quads[m][1]];
                fbuf[7]  = (float) y[quads[m][1]];
                fbuf[8]  = (float) z[quads[m][1]];

                fbuf[9]  = (float) x[quads[m][2]];
                fbuf[10]  = (float) y[quads[m][2]];
                fbuf[11]  = (float) z[quads[m][2]];

                fwrite(fbuf, 12*sizeof(float), 1, fptr);
                fwrite(&elemAttribute,sizeof(unsigned short),1,fptr);

                // second triangle

                fbuf[3]  = (float) x[quads[m][0]];
                fbuf[4]  = (float) y[quads[m][0]];
                fbuf[5]  = (float) z[quads[m][0]];

                fbuf[6]  = (float) x[quads[m][2]];
                fbuf[7]  = (float) y[quads[m][2]];
                fbuf[8]  = (float) z[quads[m][2]];

                fbuf[9]  = (float) x[quads[m][3]];
                fbuf[10]  = (float) y[quads[m][3]];
                fbuf[11]  = (float) z[quads[m][3]];

                fwrite(fbuf, 12*sizeof(float), 1, fptr);
                fwrite(&elemAttribute,sizeof(unsigned short),1,fptr);

                nTriangles +=2;
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
bool	 TopOptIsoSurface::setIsoNodeXYZ(int isoCounter, int node, double xyz[3])
{
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return false;

	if ( (node < 0) || (node >= ts_isoNPoints[isoCounter])) return false;

	for (int i = 0; i < 3; i++)
		ts_isoXYZ[isoCounter][3*node+i] = xyz[i];

	return true;
}
/**********************************************************************/
int		 TopOptIsoSurface::getNumIsoNodes(int isoCounter)
{
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return 0;
	return (ts_isoNPoints[isoCounter]);
}
/**********************************************************************/
bool	 TopOptIsoSurface::getIsoTriNodes(int isoCounter,int tri, int nodes[3])
{
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return false;

	if ( (tri < 0) || (tri >= ts_isoNTriangles[isoCounter])) return false;

	for (int i = 0; i < 3; i++)
		nodes[i] = ts_isoTriangles[isoCounter][3*tri+i];

	return true;
}
/**********************************************************************/
bool	 TopOptIsoSurface::getIsoNodeXYZ(int isoCounter, int node, double xyz[3])
{
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return false;

	if ( (node < 0) || (node >= ts_isoNPoints[isoCounter])) return false;

	for (int i = 0; i < 3; i++)
		xyz[i] = ts_isoXYZ[isoCounter][3*node+i];

	return true;
}
/**********************************************************************/
bool	 TopOptIsoSurface::getIsoNodeNormal(int isoCounter, int node, double normal[3])
{
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return false;

	if ( (node < 0) || (node >= ts_isoNPoints[isoCounter])) return false;

	for (int i = 0; i < 3; i++)
		normal[i] = ts_isoVertexNormals[isoCounter][3*node+i];

	return true;
}
/**********************************************************************/
int TopOptIsoSurface::getNumIsoTriangles(int isoCounter)
{
	
	if ((isoCounter < 0) || (isoCounter >= MAX_ISO_SAVES)) return 0;
	if (isoCounter > ts_currentIsoCounter) return 0;
	return ts_isoNTriangles[isoCounter];
}
/**********************************************************************/
void TopOptIsoSurface:: getIsoTriNeighbors(int isoCounter,int tri,int triNeighbors[3])
{
	
	for (int i = 0; i < 3; i++) 
		triNeighbors[i] = ts_isoTrianglesNeighbors[isoCounter][3*tri+i];
}
/**********************************************************************/
int		TopOptIsoSurface::getIsoTriPartNum(int isoCounter,int tri)
{
	return ts_isoPartId[isoCounter][tri];
}
/**********************************************************************/
void TopOptIsoSurface::getIsoTriPts(int isoCounter,int tri,double xTri[3],double yTri[3],double zTri[3],double triNormal[3],
	double vertexNormals0[3], double vertexNormals1[3], double vertexNormals2[3])
{
	int node;

	if ((tri < 0) || (tri >= ts_isoNTriangles[isoCounter])) return;

	for (int i = 0; i < 3; i++) {
		node = ts_isoTriangles[isoCounter][3*tri+i];
		if (i == 0)
			for (int j = 0; j < 3; j++)
				vertexNormals0[j] = ts_isoVertexNormals[isoCounter][3 * node + j];
		else if (i == 1)
			for (int j = 0; j < 3; j++)
				vertexNormals1[j] = ts_isoVertexNormals[isoCounter][3 * node + j];
		else 
			for (int j = 0; j < 3; j++)
				vertexNormals2[j] = ts_isoVertexNormals[isoCounter][3 * node + j];
		xTri[i] = ts_isoXYZ[isoCounter][3*node];
		yTri[i] = ts_isoXYZ[isoCounter][3*node+1];
		zTri[i] = ts_isoXYZ[isoCounter][3*node+2];
		triNormal[i] = ts_isoNormals[isoCounter][3*tri+i];
	}
}
/**********************************************************************/
TopOptIsoSurface::~TopOptIsoSurface() 
{
	for (int i = 0; i < MAX_ISO_SAVES;i++) {
		ts_isoNPoints[i]  = 0;
		ts_isoNTriangles[i] = 0;
		safeFree(ts_isoXYZ[i]);
		safeFree(ts_isoTriangles[i]);
		safeFree(ts_isoNormals[i]);
		safeFree(ts_isoPartId[i]);

		ts_isoXYZ[i] = NULL;
		ts_isoTriangles[i] = NULL;
		ts_isoNormals[i] = NULL;
		ts_isoPartId[i] = NULL;
	}
	safeFree(ts_isoUVWFinal);
	safeFree(ts_isoVonMisesFinal);
    safeFree(ts_isoTemperatureFinal);
	ts_isoUVWFinal = NULL;
	ts_isoVonMisesFinal = NULL;
}


