#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ContourExtraction.h"
#include "queue.h"
#include <psimpl.h>
#include <iostream>
#include <fstream>
#include <array>
#include <earcut.hpp>
#include <Eigen/Eigenvalues>


/**********************************************************************/
ContourExtraction::ContourExtraction(void)
{
	initializeContours();
}
/**********************************************************************/
void ContourExtraction::initializeContours(void)
{
    ce_contourExtraction_verbose = false;
	ce_nMeshComponents = 0;

    // the following two variables are used in discarding small mesh components
    ce_componentError = 0.0005;
    ce_componentErrorScaling = 4;// can be controlled by user
    ce_componentSizeError = 100;// computed as meshSize*volumeFraction*ce_componentError*ce_componentErrorScaling

    // the variable is used for symmetry detection
    ce_symmetryErrorScaling = 4;

    // the following is used in determining whether to merge or split consecutive layers for sweeping/lofting
    ce_layerError = 0.025;
    ce_layerErrorScaling = 4;// can be controlled by user

    // the following is used to simplify and smoothen contours
    ce_contourError = 0.05;// the mesh edge length is used as reference
    ce_contourErrorScaling = 4;// can be controlled by user

    ce_exploitSymmetry = true;// can be turned to false for debugging, but default to true
    for (int component = 0; component < MAX_MESH_COMPONENTS;component++){
        ce_numContoursInComponent[component] = 0;
        centerOfComponent[component].x = 0;
        centerOfComponent[component].y = 0;
        centerOfComponent[component].z = 0;
    }
}
/**********************************************************************/
int ContourExtraction::getComponentOfGridElement(int nx,int ny,int nz)
{
	double xc,yc,zc;
	int elem;

	xc = h_xMin + (nx+0.5)*h_dx;
	yc = h_yMin + (ny+0.5)*h_dy;
	zc = h_zMin + (nz+0.5)*h_dz;
	elem = findElemWithCenter(xc,yc,zc);
	if ((elem < 0 ) || (elem > h_nElems)) //elem is outside
		return -1;

	return h_elemComponent[elem];

}
/**********************************************************************/
int ContourExtraction::getPixelValueXY(int currentComponent,int pX,int pY,int nz)
{
	/*   
	checking the 2x2 pixel grid, assigning these values to each pixel 
	+---+---+
	| 1 | 2 |
	+---+---+
	| 4 | 8 | <- current pixel (pX,pY)
	+---+---+  
	*/

	int squareValue = 0;
	// checking upper left pixel
	if (getComponentOfGridElement(pX-1,pY-1,nz) == currentComponent) 
		squareValue+=1;

	// checking upper pixel
	if (getComponentOfGridElement(pX,pY-1,nz) == currentComponent) 
		squareValue+=2;

	// checking left pixel
	if (getComponentOfGridElement(pX-1,pY,nz) == currentComponent) 
		squareValue+=4;

	// checking current pixel
	if (getComponentOfGridElement(pX,pY,nz) == currentComponent) 
		squareValue+=8;

	return squareValue;
}
/**********************************************************************/
int ContourExtraction::getPixelValueYZ(int currentComponent,int nx,int pY,int pZ)
{
	/*   
	checking the 2x2 pixel grid, assigning these values to each pixel 
	+---+---+
	| 1 | 2 |
	+---+---+
	| 4 | 8 | <- current pixel (pY,pZ)
	+---+---+  
	*/

	int squareValue = 0;
	// checking upper left pixel
	if (getComponentOfGridElement(nx,pY-1,pZ-1) == currentComponent) 
		squareValue+=1;

	// checking upper pixel
	if (getComponentOfGridElement(nx,pY-1,pZ) == currentComponent) 
		squareValue+=2;

	// checking left pixel
	if (getComponentOfGridElement(nx,pY,pZ-1) == currentComponent) 
		squareValue+=4;

	// checking current pixel
	if (getComponentOfGridElement(nx,pY,pZ) == currentComponent) 
		squareValue+=8;

	return squareValue;
}
/**********************************************************************/
int ContourExtraction::getPixelValueZX(int currentComponent,int pX,int ny,int pZ)
{
	/*   
	checking the 2x2 pixel grid, assigning these values to each pixel 
	+---+---+
	| 1 | 2 |
	+---+---+
	| 4 | 8 | <- current pixel (pX,pY)
	+---+---+  
	*/

	int squareValue = 0;
	// checking upper left pixel
	if (getComponentOfGridElement(pX-1,ny,pZ-1) == currentComponent) 
		squareValue+=1;

	// checking upper pixel
	if (getComponentOfGridElement(pX,ny,pZ-1) == currentComponent) 
		squareValue+=2;

	// checking left pixel
	if (getComponentOfGridElement(pX-1,ny,pZ) == currentComponent) 
		squareValue+=4;

	// checking current pixel
	if (getComponentOfGridElement(pX,ny,pZ) == currentComponent) 
		squareValue+=8;

	return squareValue;
}
/**********************************************************************/
void ContourExtraction::generateContour(int component,int contour, int relativeOffset, int gridIndices[3] )
{
     if(ce_contourExtraction_verbose) writeMessageToParetoLog("Generating contour %d for component %d ",contour,component);
    if (ce_sweepDirection[component] == 0) {// x direction
        generateContourYZ(component,contour,gridIndices[0],gridIndices[1],gridIndices[2]);
        offsetContourYZ(component,contour,relativeOffset);
    }
    else if (ce_sweepDirection[component] == 1){
        generateContourZX(component,contour,gridIndices[0],gridIndices[1],gridIndices[2]);
        offsetContourZX(component,contour,relativeOffset);
    }
    else {
        generateContourXY(component,contour,gridIndices[0],gridIndices[1],gridIndices[2]);
        offsetContourXY(component,contour,relativeOffset);
    }
}
/**********************************************************************/
void ContourExtraction::generateContourXY(int component, int contour,int startX, int startY, int nz )
{
	// "Using marching squares algorithm to trace the contour of an image"

	// Algorithm
	// 1. First find a voxel at the given height that lies on the interior
	// 2. Note the center of that voxel
	// 3. Consider a 2x2 voxel in the plane  surrounding this voxel
	// 4. Depending on which the voxels is in, move along the contour to the next voxel
	// 5. Go back to the Step-2 until the contour is closed
	double center[3];
	int stepX,stepY,prevX,prevY,pX,pY,squareValue,nLines;
	point pt;


	pX = startX;
	pY = startY;
	prevX = 0;
	prevY = 0;
	stepX = 1;
	stepY = 1;
	nLines = 0;

    ce_contourData[component][contour].clear();
	while (1) {	
		center[0] = h_xMin + (pX+0.5)*h_dx;
		center[1]  = h_yMin + (pY+0.5)*h_dy;
		center[2]  = h_zMin + (nz+0.5)*h_dz;

		pt.x = center[0];
		pt.y = center[1];
		pt.z = center[2];
        ce_contourData[component][contour].push_front(pt);

		// the value at the current 'pixel' is determined by four neighbors
		squareValue = getPixelValueXY(component,pX, pY, nz);
        //writeMessageToParetoLog("%d \n",squareValue);
		switch (squareValue) {
			/* going UP with these cases:
			+---+---+   +---+---+   +---+---+
			| 1 |   |   | 1 |   |   | 1 |   |
			+---+---+   +---+---+   +---+---+
			|   |   |   | 4 |   |   | 4 | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 1 :
		case 5 :
		case 13 :
			stepX=0;
			stepY=-1;
			break;
			/* going DOWN with these cases:
			+---+---+   +---+---+   +---+---+
			|   |   |   |   | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   | 8 |   |   | 8 |   |   | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 8 : 
		case 10 :
		case 11 :
			stepX=0;
			stepY=1;
			break;
			/* going LEFT with these cases:

			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   |   | 2 |
			+---+---+   +---+---+   +---+---+
			| 4 |   |   | 4 | 8 |   | 4 | 8 |
			+---+---+   +---+---+      +---+---+

			*/
		case 4 :
		case 12 :
		case 14 :
			stepX=-1;
			stepY=0;
			break;
			/* going RIGHT with these cases:
			+---+---+   +---+---+   +---+---+
			|   | 2 |   | 1 | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   | 4 |   |
			+---+---+   +---+---+   +---+---+

			*/
		case 2 :
		case 3 :
		case 7 :
			stepX=1;
			stepY=0;
			break;
		case 6 :
			/* special saddle point case 1:

			+---+---+
			|   | 2 |
			+---+---+
			| 4 |   |
			+---+---+
			going LEFT if coming from UP
			else going RIGHT
			*/
			if (prevX==0&&prevY==-1) {
				stepX=-1;
				stepY=0;
			}
			else {
				stepX=1;
				stepY=0;
			}
			break;
		case 9 :
			/* special saddle point case 2:
			+---+---+
			| 1 |   |
			+---+---+
			|   | 8 |
			+---+---+
			going UP if coming from RIGHT
			else going DOWN
			*/
			if (prevX==1&&prevY==0) {
				stepX=0;
				stepY=-1;
			}
			else {
				stepX=0;
				stepY=1;
			}
			break;
		}
		nLines++;
		// moving onto next point
		pX+=stepX;
		pY+=stepY;
		prevX=stepX;
		prevY=stepY;


		if ((pX == startX) && (pY == startY)) {
			break;	
		}
	}
}
/**********************************************************************/
void ContourExtraction::generateContourYZ(int component,int contour, int nx, int startY, int startZ )
{
	// "Using marching squares algorithm to trace the contour of an image"

	// Algorithm
	// 1. First find a voxel at the given height that lies on the interior
	// 2. Note the center of that voxel
	// 3. Consider a 2x2 voxel in the plane  surrounding this voxel
	// 4. Depending on which the voxels is in, move along the contour to the next voxel
	// 5. Go back to the Step-2 until the contour is closed
	double center[3];
	int stepY,stepZ,prevZ,prevY,pZ,pY,squareValue,nLines;
	point pt;

	pY = startY;
	pZ = startZ;
	prevY = 0;
	prevZ = 0;
	stepY = 1;
	stepZ = 1;
	nLines = 0;

    ce_contourData[component][contour].clear();
	while (1) {	
		center[0] = h_xMin + (nx+0.5)*h_dx;
		center[1]  = h_yMin + (pY+0.5)*h_dy;
		center[2]  = h_zMin + (pZ+0.5)*h_dz;

		pt.x = center[0];
		pt.y = center[1];
		pt.z = center[2];
        ce_contourData[component][contour].push_front(pt);

		// the value at the current 'pixel' is determined by four neighbors
		squareValue = getPixelValueYZ(component,nx, pY, pZ);
        //writeMessageToParetoLog("%d \n",squareValue);
		switch (squareValue) {
			/* going UP with these cases:
			+---+---+   +---+---+   +---+---+
			| 1 |   |   | 1 |   |   | 1 |   |
			+---+---+   +---+---+   +---+---+
			|   |   |   | 4 |   |   | 4 | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 1 :
		case 5 :
		case 13 :
			stepZ=0;
			stepY=-1;
			break;
			/* going DOWN with these cases:
			+---+---+   +---+---+   +---+---+
			|   |   |   |   | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   | 8 |   |   | 8 |   |   | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 8 : 
		case 10 :
		case 11 :
			stepZ=0;
			stepY=1;
			break;
			/* going LEFT with these cases:

			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   |   | 2 |
			+---+---+   +---+---+   +---+---+
			| 4 |   |   | 4 | 8 |   | 4 | 8 |
			+---+---+   +---+---+      +---+---+

			*/
		case 4 :
		case 12 :
		case 14 :
			stepZ=-1;
			stepY=0;
			break;
			/* going RIGHT with these cases:
			+---+---+   +---+---+   +---+---+
			|   | 2 |   | 1 | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   | 4 |   |
			+---+---+   +---+---+   +---+---+

			*/
		case 2 :
		case 3 :
		case 7 :
			stepZ=1;
			stepY=0;
			break;
		case 6 :
			/* special saddle point case 1:

			+---+---+
			|   | 2 |
			+---+---+
			| 4 |   |
			+---+---+
			going LEFT if coming from UP
			else going RIGHT
			*/
			if (prevZ==0 && prevY==-1) {
				stepZ=-1;
				stepY=0;
			}
			else {
				stepZ=1;
				stepY=0;
			}
			break;
		case 9 :
			/* special saddle point case 2:
			+---+---+
			| 1 |   |
			+---+---+
			|   | 8 |
			+---+---+
			going UP if coming from RIGHT
			else going DOWN
			*/
			if (prevZ==1&&prevY==0) {
				stepZ=0;
				stepY=-1;
			}
			else {
				stepZ=0;
				stepY=1;
			}
			break;
		}
		nLines++;
		// moving onto next point
		pY+=stepY;
		pZ+=stepZ;
		prevZ=stepZ;
		prevY=stepY;

        //writeMessageToParetoLog("%d %d \n",pY,pZ);
		if ((pZ== startZ) && (pY == startY)) {
			break;	
		}
	}
}
/**********************************************************************/
void ContourExtraction::generateContourZX(int component, int contour,int startX, int ny, int startZ )
{
	// "Using marching squares algorithm to trace the contour of an image"

	// Algorithm
	// 1. First find a voxel at the given height that lies on the interior
	// 2. Note the center of that voxel
	// 3. Consider a 2x2 voxel in the plane  surrounding this voxel
	// 4. Depending on which the voxels is in, move along the contour to the next voxel
	// 5. Go back to the Step-2 until the contour is closed
	double center[3];
	int stepX,stepZ,prevX,prevZ,pX,pZ,squareValue,nLines;
	point pt;


	pX = startX;
	pZ = startZ;
	prevX = 0;
	prevZ = 0;
	stepX = 1;
	stepZ = 1;
	nLines = 0;

    ce_contourData[component][contour].clear();

    //writeMessageToParetoLog("Start %d %d \n",startX,startZ);
	while (1) {	
		center[0] = h_xMin + (pX+0.5)*h_dx;
		center[1]  = h_yMin + (ny+0.5)*h_dy;
		center[2]  = h_zMin + (pZ+0.5)*h_dz;

		pt.x = center[0];
		pt.y = center[1];
		pt.z = center[2];
        ce_contourData[component][contour].push_back(pt);

		// the value at the current 'pixel' is determined by four neighbors
		squareValue = getPixelValueZX(component,pX, ny, pZ);
        //writeMessageToParetoLog("%d \n",squareValue);
		switch (squareValue) {
			/* going UP with these cases:
			+---+---+   +---+---+   +---+---+
			| 1 |   |   | 1 |   |   | 1 |   |
			+---+---+   +---+---+   +---+---+
			|   |   |   | 4 |   |   | 4 | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 1 :
		case 5 :
		case 13 :
			stepX=0;
			stepZ=-1;
			break;
			/* going DOWN with these cases:
			+---+---+   +---+---+   +---+---+
			|   |   |   |   | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   | 8 |   |   | 8 |   |   | 8 |
			+---+---+   +---+---+   +---+---+
			*/
		case 8 : 
		case 10 :
		case 11 :
			stepX=0;
			stepZ=1;
			break;
			/* going LEFT with these cases:

			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   |   | 2 |
			+---+---+   +---+---+   +---+---+
			| 4 |   |   | 4 | 8 |   | 4 | 8 |
			+---+---+   +---+---+      +---+---+

			*/
		case 4 :
		case 12 :
		case 14 :
			stepX=-1;
			stepZ=0;
			break;
			/* going RIGHT with these cases:
			+---+---+   +---+---+   +---+---+
			|   | 2 |   | 1 | 2 |   | 1 | 2 |
			+---+---+   +---+---+   +---+---+
			|   |   |   |   |   |   | 4 |   |
			+---+---+   +---+---+   +---+---+

			*/
		case 2 :
		case 3 :
		case 7 :
			stepX=1;
			stepZ=0;
			break;
		case 6 :
			/* special saddle point case 1:

			+---+---+
			|   | 2 |
			+---+---+
			| 4 |   |
			+---+---+
			going LEFT if coming from UP
			else going RIGHT
			*/
			if (prevX==0&&prevZ==-1) {
				stepX=-1;
				stepZ=0;
			}
			else {
				stepX=1;
				stepZ=0;
			}
			break;
		case 9 :
			/* special saddle point case 2:
			+---+---+
			| 1 |   |
			+---+---+
			|   | 8 |
			+---+---+
			going UP if coming from RIGHT
			else going DOWN
			*/
			if (prevX==1&&prevZ==0) {
				stepX=0;
				stepZ=-1;
			}
			else {
				stepX=0;
				stepZ=1;
			}
			break;
		}
		nLines++;
		// moving onto next point
		pX+=stepX;
		pZ+=stepZ;
		prevX=stepX;
		prevZ=stepZ;

        //writeMessageToParetoLog("%d %d \n",pX,pZ);

		if ((pX == startX) && (pZ == startZ)) {
			break;	
		}
	}
}
/**********************************************************************/
void ContourExtraction::offsetContourXY(int component,int contour,int relativeOffset)
{
	int elem;
	point pt;
	list<point>::iterator iter;

	
	// This offset targets contours that are close to boundary. Through this offset, slivers are not left behind near the boundary
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		pt = *iter;
		elem = findElemWithCenter(pt.x,pt.y,pt.z);
		if (elem < 0) {//element is already outside
			if (findElemWithCenter(pt.x-h_dx,pt.y,pt.z) >= 0) {// there is an element to the left
				iter->x += relativeOffset*h_dx;//move right
				continue;
			}
			if (findElemWithCenter(pt.x,pt.y-h_dy,pt.z) >= 0) {
				iter->y += relativeOffset*h_dy;
				continue;
			}
		}
		else {
		 //not sure what this code does
		if (findElemWithCenter(pt.x-h_dx,pt.y,pt.z) < 0) {
			iter->x -= (relativeOffset+1)*h_dx;
			continue;
		}
		if (findElemWithCenter(pt.x,pt.y-h_dy,pt.z) < 0) {
			iter->y -= (relativeOffset+1)*h_dy;
			continue;
		}
		}
	}
	// finally shift entire contour
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		iter->x -= h_dx/2;
		iter->y -= h_dy/2;
	}
	
}
/**********************************************************************/
void ContourExtraction::offsetContourYZ(int component,int contour,int relativeOffset)
{
	int elem;
	point pt;
	list<point>::iterator iter;

	// This offset targets contours that are close to boundary. Through this offset, slivers are not left behind near the boundary
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		pt = *iter;
		elem = findElemWithCenter(pt.x,pt.y,pt.z);
		if (elem < 0) {//element is already outside
			if (findElemWithCenter(pt.x,pt.y-h_dy,pt.z) >= 0) {// there is an element to the left
				iter->y += relativeOffset*h_dy;//move right
				continue;
			}
			if (findElemWithCenter(pt.x,pt.y,pt.z-h_dz) >= 0) {
				iter->z += relativeOffset*h_dz;
				continue;
			}
		}

		if (findElemWithCenter(pt.x,pt.y-h_dy,pt.z) < 0) {
			iter->y -= (relativeOffset+1)*h_dy;
			continue;
		}
		if (findElemWithCenter(pt.x,pt.y,pt.z-h_dz) < 0) {
			iter->z -= (relativeOffset+1)*h_dz;
			continue;
		}
	}

	// finally shift entire contour
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		iter->y -= h_dy/2;
		iter->z -= h_dz/2;
	}
}

/**********************************************************************/
void ContourExtraction::offsetContourZX(int component,int contour,int relativeOffset)
{
	int elem;
	point pt;
	list<point>::iterator iter;

	// This offset targets contours that are close to boundary. Through this offset, slivers are not left behind near the boundary
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		pt = *iter;
		elem = findElemWithCenter(pt.x,pt.y,pt.z);
		if (elem < 0) {//element is already outside
			if (findElemWithCenter(pt.x-h_dx,pt.y,pt.z) >= 0) {// there is an element to the left
				iter->x += relativeOffset*h_dx;//move right
				continue;
			}
			if (findElemWithCenter(pt.x,pt.y,pt.z-h_dz) >= 0) {
				iter->z += relativeOffset*h_dz;
				continue;
			}
		}

		if (findElemWithCenter(pt.x-h_dx,pt.y,pt.z) < 0) {
			iter->x -= (relativeOffset+1)*h_dx;
			continue;
		}
		if (findElemWithCenter(pt.x,pt.y,pt.z-h_dz) < 0) {
			iter->z -= (relativeOffset+1)*h_dz;
			continue;
		}
	}

	// finally shift entire contour
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
		iter->z -= h_dz/2;
		iter->x -= h_dx/2;
	}
}
/**********************************************************************/
void ContourExtraction::computeMassPropertiesOfContour(int component,int contour,double center[3], double principalAxis[2][2])
{
    // compute properties of contour
    // the moment of inertia of contour in its plane, and about the center
    list<point>::iterator iterCurrent,iterNext;
    point p1,p2;
    double x1,y1,x2,y2,a, area,xc,yc;


    // Reference: http://richardson.eng.ua.edu/Former_Courses/CE_331_fa09/Projects/A_and_I_of_Polygon.pdf
    xc = yc = 0;
    area = 0;
    for( iterCurrent=ce_contourData[component][contour].begin(); iterCurrent != ce_contourData[component][contour].end(); ++iterCurrent) {
        iterNext = iterCurrent;
        iterNext++;//next

        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();

        p1 = *iterCurrent;
        p2 = *iterNext;

        if (ce_sweepDirection[component] == 0){
            x1 = p1.y;
            y1 = p1.z;
            x2 = p2.y;
            y2 = p2.z;
        }
        else if (ce_sweepDirection[component] == 1){
            x1 = p1.z;
            y1 = p1.x;
            x2 = p2.z;
            y2 = p2.x;
        }
        else {
            x1 = p1.x;
            y1 = p1.y;
            x2 = p2.x;
            y2 = p2.y;
        }

        area += 0.5*(x1*y2 - x2*y1);
        xc += (x1 + x2)*(x1*y2 - x2*y1);
        yc += (y1 + y2)*(x1*y2 - x2*y1);
    }

    xc /= (6*area);
    yc /= (6*area);


    p1 = *(ce_contourData[component][contour].begin());
    if (ce_sweepDirection[component] == 0){
        center[0] = p1.x;
        center[1] = yc;
        center[2] = xc;
    }
    else if (ce_sweepDirection[component] == 1){
        center[0] = yc;
        center[1] = p1.y;
        center[2] = xc;
    }
    else {
        center[0] = xc;
        center[1] = yc;
        center[2] = p1.z;
    }

    MatrixXd IMatrix = MatrixXd::Zero(2,2);

    for( iterCurrent=ce_contourData[component][contour].begin(); iterCurrent != ce_contourData[component][contour].end(); ++iterCurrent) {
        iterNext = iterCurrent;
        iterNext++;//next
        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();

        p1 = *iterCurrent;
        p2 = *iterNext;

        p1.x -= center[0];//shift
        p1.y -= center[1];
        p1.z -= center[2];


        p2.x -= center[0];//shift
        p2.y -= center[1];
        p2.z -= center[2];

        if (ce_sweepDirection[component] == 0){
            x1 = p1.y;
            y1 = p1.z;
            x2 = p2.y;
            y2 = p2.z;
        }
        else if (ce_sweepDirection[component] == 1){
            x1 = p1.z;
            y1 = p1.x;
            x2 = p2.z;
            y2 = p2.x;
        }
        else {
            x1 = p1.x;
            y1 = p1.y;
            x2 = p2.x;
            y2 = p2.y;
        }


        a = -(x1*y2 - x2*y1);
        IMatrix(0,0) += (1./12)*(y1*y1 + y1*y2 + y2*y2)*a;
        IMatrix(1,1) += (1./12)*(x1*x1 + x1*x2 + x2*x2)*a;
        IMatrix(0,1) += (1./24)*(x1*y2 + 2*x1*y1 + 2*x2*y2 + x2*y1)*a;

    }
    IMatrix(1,0) = IMatrix(0,1);

    SelfAdjointEigenSolver<MatrixXd> es(IMatrix);

    for (int i = 0; i < 2; i++)
       for (int j = 0; j < 2; j++)
        principalAxis[i][j] = es.eigenvectors()(i,j);

}
/**********************************************************************/
double ContourExtraction::findClosestPtOnContour(int component,int contour,point inputPt, point &closestPt)
{
    list<point>::iterator iter,iterNext;
    point pt1,pt2,ptTemp;
    double dist,distMin;

    distMin = 1e12;
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        iterNext = iter;
        iterNext++;//next
        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();

        pt1 = *iter;
        pt2 = *iterNext;
        dist = distanceOfPointFromLine3D(inputPt,pt1,pt2,ptTemp);
        if (dist < distMin){
            distMin = dist;
            closestPt.x = ptTemp.x;
            closestPt.y = ptTemp.y;
            closestPt.z = ptTemp.z;
        }
    }

    return distMin;
}
/**********************************************************************/
double ContourExtraction::distanceOfPointFromLine3D(point inputPt,point linePt1,point linePt2,point &closestPt)
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


    point pt1_pt2, pt2_pt;
    double a,b, distance,t;
    int i;

    pt1_pt2.x= linePt1.x - linePt2.x;
    pt1_pt2.y= linePt1.y - linePt2.y;
    pt1_pt2.z= linePt1.z - linePt2.z;

    pt2_pt.x = linePt2.x - inputPt.x;
    pt2_pt.y = linePt2.y - inputPt.y;
    pt2_pt.z = linePt2.z - inputPt.z;


    a = pt1_pt2.x*pt2_pt.x + pt1_pt2.y*pt2_pt.y + pt1_pt2.z*pt2_pt.z ;//DOTPROD3D(pt1_pt2,pt2_pt);
    b = pt1_pt2.x*pt1_pt2.x + pt1_pt2.y*pt1_pt2.y + pt1_pt2.z*pt1_pt2.z;//DOTPROD3D(pt1_pt2,pt1_pt2);


    t = -a/(b+1e-12);
    t = MIN(1.,MAX(0,t));
    closestPt.x = linePt1.x *t + linePt2.x *(1-t);
    closestPt.y = linePt1.y *t + linePt2.y *(1-t);
    closestPt.z = linePt1.z *t + linePt2.z *(1-t);

    distance = distanceBetweenPoints(inputPt,closestPt);
    return distance;
}

/**********************************************************************/
bool ContourExtraction::mapContourToMaster(int component,int slaveContour,int masterContour)
{
    // use points on the master contour to create new points on the slaveContour
    list<point>::iterator iter;
    list<point> tempListOfPoints;
    point masterPt,closestPt,tempPt;

    tempListOfPoints.clear();
    for( iter=ce_contourData[component][masterContour].begin(); iter != ce_contourData[component][masterContour].end(); ++iter) {
        masterPt = *iter;
        findClosestPtOnContour(component,slaveContour,masterPt,closestPt);
        tempListOfPoints.push_back(closestPt);
    }

    //copy these over
    ce_contourData[component][slaveContour].clear();
    for( iter= tempListOfPoints.begin(); iter != tempListOfPoints.end(); ++iter) {
        tempPt = *iter;
        ce_contourData[component][slaveContour].push_front(tempPt);
    }

}
/**********************************************************************/
int ContourExtraction::findOptimalDirection(int component, int *nContoursEstimated)
{
    int nTopologicalComponents[3],minTopologicalComponents;
    int preferredDirection,extrudeThickness,meshThickness[3];

    meshThickness[0] = h_nelx;
    meshThickness[1] = h_nely;
    meshThickness[2] = h_nelz;

    if (ts_extrudeDirection > 0){// first preference for through cut direction
        preferredDirection = ts_extrudeDirection-1; // offset by 1
        estimateNumberOfContoursInMeshComponent( component, preferredDirection,nContoursEstimated,&extrudeThickness);
    }
    else if (ts_drawDirection > 0) {// second preference for draw direction
        preferredDirection = ts_drawDirection-1; // offset by 1
        estimateNumberOfContoursInMeshComponent( component, preferredDirection,nContoursEstimated,&extrudeThickness);
    }
    else {// find the directions that requires the least split based on topology between layers
        preferredDirection = 0;
        minTopologicalComponents = 100000;
        for (int dir = 0; dir < 3; dir++ ){
           nTopologicalComponents[dir] = splitComponentBasedOnInternalContours(component,dir,false);
           if(ce_contourExtraction_verbose) writeMessageToParetoLog("Dir: %d, nTopologicalComponents: %d ",dir,nTopologicalComponents[dir]);
           if (nTopologicalComponents[dir] < minTopologicalComponents){
                preferredDirection = dir;
                minTopologicalComponents = nTopologicalComponents[dir];
           }
           else if (nTopologicalComponents[dir] == minTopologicalComponents){
                if (meshThickness[dir] < meshThickness[preferredDirection]){
                    preferredDirection = dir;
                    minTopologicalComponents = nTopologicalComponents[dir];
                }
           }
        }
        estimateNumberOfContoursInMeshComponent( component, preferredDirection,nContoursEstimated,&extrudeThickness);
    }
    /*
    else if ((ts_symmetryIndex > 0) && (ts_symmetryIndex < 7)) {// third preference for symmetry constraints
        switch (ts_symmetryIndex){
        case 1: preferredDirection = 0; break;// x symmetry
        case 2: preferredDirection = 1; break;// y symmetry
        case 3: // x and y symmetry
            if (h_nelx < h_nely) preferredDirection = 0;
            else preferredDirection = 0;
            break;
        case 4: preferredDirection = 2; break;// z symmetry
        case 5: // x and z symmetry
            if (h_nelx < h_nelz) preferredDirection = 0;
            else preferredDirection = 2;
            break;
        case 6: // y and z symmetry
            if (h_nely < h_nelz) preferredDirection = 1;
            else preferredDirection = 2;
            break;
        }
        estimateNumberOfContoursInMeshComponent( component, preferredDirection,&nContoursMin,&extrudeThickness);
    }
    else  {// Find the direction with the least number of contours
        nContoursMin = MAX_CONTOURS_PER_COMPONENT;
        extrudeThicknessMin = 1000000;
        for (int dir = 0; dir < 3; dir++){// try all 3 directions
            nContoursAlongDir[dir] = MAX_CONTOURS_PER_COMPONENT;
            // estimate the number of contours needed to sweep or loft component within a desired accuracy.
            estimateNumberOfContoursInMeshComponent( component, dir,&(nContoursAlongDir[dir]),&extrudeThickness);
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Dir: %d, nContours = %d \n",dir,nContoursAlongDir[dir]);
            if ((nContoursAlongDir[dir] <= nContoursMin ) && (extrudeThickness < extrudeThicknessMin) ) {// pick the direction with least number of contours
                nContoursMin = nContoursAlongDir[dir];
                extrudeThicknessMin = extrudeThickness;
                preferredDirection= dir;
            }
        }
        if (nContoursMin > 1 ){// more than one contour, pick the smallest length direction
            preferredDirection = 0;
            if (h_nely < h_nelx) preferredDirection = 1;
            if (h_nelz < h_nelx) preferredDirection = 2;
            if (h_nelz < h_nely) preferredDirection = 2;
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Picking dir: %d \n",preferredDirection );
        }
    }
    */
    return preferredDirection;
}
/**********************************************************************/
int ContourExtraction::splitComponentBasedOnInternalContours(int component, int sweepDirection, bool splitComponents)
{
    // for each layer of the component along the given direction,
    // find the number of internal contours.

    // If doNotSplit is set to true, then only the number of components is computed.
    // but the component is not split

    Queue Q;
    list<int>::iterator iter;
    list<int> layerList[1000];
    int elem,layer,layerStart,layerEnd,gridIndices[3],nElems,neighborElem;
    int nInternalContours[1000];//for each layer
    int nInternalContoursRef,nInternalContoursCurrent;
    bool stopInsertingElemsIntoCurrentComponent;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Finding internal contours of component %d along dir %d ",component,sweepDirection);


    layerStart = 100000;
    layerEnd = -100000;

    // first find the start and end of layers along this direction
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layerStart = MIN(layerStart,gridIndices[sweepDirection]);
        layerEnd = MAX(layerEnd,gridIndices[sweepDirection]);
    }

    // clear
    for (layer = layerStart; layer <= layerEnd;layer++) {// only need to initialzie the relevant intermediate lists
        layerList[layer].clear();
        nInternalContours[layer] = 0;
    }

     // To find islands, push all  elements that do NOT belong to this component into each layer
    for( elem=0; elem < h_nElems; ++elem) {
        if (h_elemComponent[elem] == component) continue;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[sweepDirection];
        if ((layer < layerStart) || (layer > layerEnd)) continue;
        layerList[layer].push_front(elem);
    }
    nElems = 0;
    for (layer = layerStart; layer <= layerEnd;layer++) {// only need to initialie the relevant intermediate lists
        nElems = MAX(nElems,layerList[layer].size());
    }


    //create a queue
    Q = CreateQueue(MAX(nElems,10));
    bool *elemVisited = (bool *)malloc(h_nElems*sizeof(bool));
    memset(elemVisited,0,h_nElems*sizeof(bool));
    memset(elemVisited,0,h_nElems*sizeof(bool));

    bool disregardThisComponent;
    int nNodesOnBoundary,nodes[8];
    for (layer = layerStart; layer <= layerEnd; layer++) {// for each layer
        nInternalContours[layer] = 0;
        for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
            elem = *iter;
            if (elemVisited[elem]) continue;
            MakeEmpty(Q);
            Enqueue(elem, Q);
            elemVisited[elem] = true;
            // if we are here, then we found one more internal contour in layer
            nInternalContours[layer]++;
            disregardThisComponent = false; // we have to detect and disregard components bordering the boundary
            while (!IsEmpty(Q)) {//start the queue
                elem = Front(Q);
                // to detect if an element on the edge, we check if 4 or more of its nodes are on boundary
                nNodesOnBoundary = 0;
                getElemNodes(elem,nodes);
                for (int i =0; i < 8; i++)
                    if (isNodeOnBoundary(nodes[i]))
                        nNodesOnBoundary++;
                if (nNodesOnBoundary > 4)// this element and component is on the boundary of the geometry
                    disregardThisComponent = true;
                Dequeue(Q);
                for (int i=0;i<7;i++){// only consider the 6 face neighbors + self
                    neighborElem = h_StructuringElems[h_StructuringSize*elem+i];
                    if (neighborElem == -1) continue;
                    if (h_elemComponent[neighborElem] == component) continue;
                    getGridIndicesOfElem(neighborElem,gridIndices);
                    if (gridIndices[sweepDirection] != layer) continue;// we only want to move in this layer
                    if (elemVisited[neighborElem]) continue;
                    elemVisited[neighborElem] = true;
                    Enqueue(neighborElem, Q);
                }
            }
            if (disregardThisComponent)// this component is in the border, not an internal component
                nInternalContours[layer]--;
        }
    }
    //clear the memory
    DisposeQueue(Q);
    safeFree(elemVisited);


    //compute the number of  potential splits
    int nComponentsNeeded;

    nComponentsNeeded = 1;
    nInternalContoursRef = nInternalContours[layerStart];
    for (layer = layerStart+1; layer <= layerEnd;layer++){
       nInternalContoursCurrent = nInternalContours[layer];
       if ((nInternalContoursCurrent != nInternalContoursRef)){
           nComponentsNeeded++;
           nInternalContoursRef = nInternalContours[layer];
       }
    }

    for (layer = layerStart; layer <= layerEnd;layer++){
        //if(ce_contourExtraction_verbose) writeMessageToParetoLog("layer = %d; internal: %d",layer,nInternalContours[layer]);
        layerList[layer].clear();
    }

    if (!splitComponents) // users says no need to actually split, estimate is sufficient
        return nComponentsNeeded;


    if (nComponentsNeeded == 1) {// only one component needed, no need to go further
        writeMessageToParetoLog("No need to split topology of component %d ",component);
         return nComponentsNeeded;
    }


    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Splitting topology of component %d along dir %d ",component,sweepDirection);

    // split the component based on topology
    // in order to create a loft or sweep, a component must have the same number of internal contours

    // push the component elements again into layers and remove the id for possible reassignment
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[sweepDirection];
        layerList[layer].push_front(elem);
        h_elemComponent[elem] = -1;
    }
     elemComponentList[component].clear();

     nInternalContoursRef = nInternalContours[layerStart];
     // include elements from first layer
     for( iter=layerList[layerStart].begin(); iter != layerList[layerStart].end(); ++iter) {// for each elem in layer
         elem = *iter;
         h_elemComponent[elem] = component;
         elemComponentList[component].push_front(elem);
     }

     stopInsertingElemsIntoCurrentComponent = false;
     for (layer = layerStart+1; layer <= layerEnd;layer++){
        nInternalContoursCurrent = nInternalContours[layer];
        if ((nInternalContoursCurrent != nInternalContoursRef)){
            if (ce_nMeshComponents >= MAX_MESH_COMPONENTS-1) {
                cout << "Exceeded max components; unable to create component" << endl;
                continue;
            }
            if(ce_contourExtraction_verbose) cout <<"Breaking component at layer " << layer << endl;
            break;
            // topology change; push elements in this layer and beyond into a new component
            if (!stopInsertingElemsIntoCurrentComponent) {
                ce_nMeshComponents++;
                if(ce_contourExtraction_verbose) cout <<"Created new component = " << ce_nMeshComponents-1 << endl;
            }
            elemComponentList[ce_nMeshComponents-1].clear();
            for (int layerTemp = layer; layerTemp <= layerEnd; layerTemp++){
                for( iter=layerList[layerTemp].begin(); iter != layerList[layerTemp].end(); ++iter) {// for each elem in layer
                    elem = *iter;
                    h_elemComponent[elem] = ce_nMeshComponents-1;
                    elemComponentList[ce_nMeshComponents-1].push_front(elem);
                }
            }
            nInternalContoursRef = nInternalContours[layer];
            stopInsertingElemsIntoCurrentComponent = true;
        }
        else if (!stopInsertingElemsIntoCurrentComponent) {
            for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
                elem = *iter;
                h_elemComponent[elem] = component;
                elemComponentList[component].push_front(elem);
            }
        }
     }

     for (layer = layerStart; layer <= layerEnd;layer++)
         layerList[layer].clear();

     return nComponentsNeeded;
}
/**********************************************************************/
int ContourExtraction::splitComponentBasedOnExternalContours(int component, int sweepDirection, bool splitComponents)
{
    // for each layer of the component along the given direction,
    // find the number of external contours.

    // If doNotSplit is set to true, then only the number of components is computed.
    // but the component is not split

    Queue Q;
    list<int>::iterator iter;
    list<int> layerList[1000];
    int elem,layer,layerStart,layerEnd,gridIndices[3],nElems,neighborElem;
    int nExternalContours[1000];//for each layer
    int nExternalContoursRef,nExternalContoursCurrent,nExternalLayersMax;
    bool stopInsertingElemsIntoCurrentComponent;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Finding external contours of component %d along dir %d ",component,sweepDirection);


    layerStart = 100000;
    layerEnd = -100000;

    // first find the start and end of layers along this direction
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layerStart = MIN(layerStart,gridIndices[sweepDirection]);
        layerEnd = MAX(layerEnd,gridIndices[sweepDirection]);
    }

    writeMessageToParetoLog("layerStart = %d, layerEnd = %d ",layerStart,layerEnd);
    // clear
    for (layer = layerStart; layer <= layerEnd;layer++) {// only need to initialzie the relevant intermediate lists
        layerList[layer].clear();
        nExternalContours[layer] = 0;
    }

     // To find islands, push all  elements that belong to this component into each layer
    for( elem=0; elem < h_nElems; ++elem) {
        if (h_elemComponent[elem] != component) continue;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[sweepDirection];
        if ((layer < layerStart) || (layer > layerEnd)) continue;
        layerList[layer].push_front(elem);
    }
    nElems = 0;
    for (layer = layerStart; layer <= layerEnd;layer++) {// only need to initialize the relevant intermediate lists
        nElems = MAX(nElems,layerList[layer].size());
    }


    //create a queue
    Q = CreateQueue(MAX(nElems,10));
    bool *elemVisited = (bool *)malloc(h_nElems*sizeof(bool));
    memset(elemVisited,0,h_nElems*sizeof(bool));
    memset(elemVisited,0,h_nElems*sizeof(bool));

    for (layer = layerStart; layer <= layerEnd; layer++) {// for each layer
        nExternalContours[layer] = 0;
        for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
            elem = *iter;
            if (elemVisited[elem]) continue;
            MakeEmpty(Q);
            Enqueue(elem, Q);
            elemVisited[elem] = true;
            // if we are here, then we found one more external contour in layer
            nExternalContours[layer]++;
            while (!IsEmpty(Q)) {//start the queue
                elem = Front(Q);
                Dequeue(Q);
                for (int i=0;i<7;i++){// only consider the 6 face neighbors + self
                    neighborElem = h_StructuringElems[h_StructuringSize*elem+i];
                    if (neighborElem == -1) continue;
                    if (h_elemComponent[neighborElem] != component) continue;
                    getGridIndicesOfElem(neighborElem,gridIndices);
                    if (gridIndices[sweepDirection] != layer) continue;// we only want to move in this layer
                    if (elemVisited[neighborElem]) continue;
                    elemVisited[neighborElem] = true;
                    Enqueue(neighborElem, Q);
                }
            }
        }
    }
    //clear the memory
    DisposeQueue(Q);
    safeFree(elemVisited);


    //compute the number of  potential splits
    int nComponentsNeeded;

    nComponentsNeeded = 1;
    nExternalLayersMax = 0;
    nExternalContoursRef = nExternalContours[layerStart];
    for (layer = layerStart+1; layer <= layerEnd;layer++){
       nExternalContoursCurrent = nExternalContours[layer];
       nExternalLayersMax = MAX(nExternalLayersMax,nExternalContoursCurrent);
       if ((nExternalContoursCurrent != nExternalContoursRef)){
           nComponentsNeeded++;
           nExternalContoursRef = nExternalContours[layer];
       }
    }

    for (layer = layerStart; layer <= layerEnd;layer++){
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("layer = %d; external: %d",layer,nExternalContours[layer]);
        layerList[layer].clear();
    }

    if (!splitComponents) // no need to actually split, estimate is sufficient
        return nComponentsNeeded;
    if (nComponentsNeeded == 1) {// only one component needed, no need to go further
        writeMessageToParetoLog("No need to split topology of component %d ",component);
        return nComponentsNeeded;
    }
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Splitting topology of component %d along dir %d ",component,sweepDirection);

    // split the component based on topology
    // in order to create a loft or sweep, a component must have the same number of external contours

    // push the component elements again into layers and remove the id for possible reassignment
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[sweepDirection];
        layerList[layer].push_front(elem);
        h_elemComponent[elem] = -1;
    }
     elemComponentList[component].clear();

     nExternalContoursRef = nExternalContours[layerStart];
     // include elements from first layer
     for( iter=layerList[layerStart].begin(); iter != layerList[layerStart].end(); ++iter) {// for each elem in layer
         elem = *iter;
         h_elemComponent[elem] = component;
         elemComponentList[component].push_front(elem);
     }

     stopInsertingElemsIntoCurrentComponent = false;
     for (layer = layerStart+1; layer <= layerEnd;layer++){
        nExternalContoursCurrent = nExternalContours[layer];
        if ((nExternalContoursCurrent != nExternalContoursRef)){
            if (ce_nMeshComponents >= MAX_MESH_COMPONENTS-1) {
                cout << "Exceeded max components; unable to create component" << endl;
                continue;
            }
            if(ce_contourExtraction_verbose) cout <<"Breaking component at layer " << layer << endl;
            stopInsertingElemsIntoCurrentComponent = true;
        }
        else if (!stopInsertingElemsIntoCurrentComponent) {
            for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
                elem = *iter;
                h_elemComponent[elem] = component;
                elemComponentList[component].push_front(elem);
            }
        }
     }

     for (layer = layerStart; layer <= layerEnd;layer++)
         layerList[layer].clear();

     return nComponentsNeeded;
}
/**********************************************************************/
bool ContourExtraction::findSymmetricComponent(int component)
{
    bool foundSymmetry;

    foundSymmetry = false;
    // check if there exists a previous component that is symmetric to this component
    if (ce_exploitSymmetry){
        if (ts_symmetryIndex > 0) {// x, y or z symmetry
            for (int master = 0; master < component; master++){
                if (ce_numContoursInComponent[master] == 0) continue; // no point checking
                if (checkForPlanarSymmetry(component,master,ts_symmetryIndex)){
                    copyContourWithSymmetry(master, component,ts_symmetryIndex);
                    if(ce_contourExtraction_verbose) cout << component << " is symmetric to  " << master << endl;
                    foundSymmetry = true;
                    break;
                }
            }
        }
        else if (ts_axiSymmetryIndex > 0) {// axi symmetry about z axis
            double rotationAngle;
            for (int master = 0; master < component; master++){
                if (ce_numContoursInComponent[master] == 0) continue;// no point checking
                if (checkForAxiSymmetry(master,component,ts_axiSymmetryIndex,&rotationAngle)){
                    if(ce_contourExtraction_verbose)cout << " Component " << component << " is axisymmetric to " << master << endl;
                    copyContourWithAxiSymmetry(master, component,rotationAngle);
                    foundSymmetry = true;
                    break;
                }
            }
        }
    }
    return foundSymmetry;
}
/**********************************************************************/
bool ContourExtraction::imposeSymmetryOnContour(int component,int contour)
{
    // Given a contour, impose internal symmetry.

    // If the topology optimization has a required plane of symmetry,
    // and if the contour intersects this plane of symmetry
    // then this symmetry is imposed (see below for algorithm)

    // Else, we identify planes of symmetry (driven by tolerances)
    // Then the symmetry is imposed as belpw

    // Algorithm to impose symmetry on a contour
    // Find all points below (or above) the plane of symmetry
    // delete points above (or below)
    // Duplicate the remaining points in the right order

    list<point>::iterator iter;
    list<point> tempListOfPoints;
    point pt;
    bool keepNegativeSide, enforceSymmetry;
    bool xSymmetry= false,ySymmetry = false,zSymmetry = false;
    double centerOfContour[3],principalAxisOfContour[2][2],centerOfSymmetry[3],centerOfGeometry[3];
    double symmetryTolerance;
    int symmetryIndex;

    //printContourPoints( component, contour);
    if (ts_symmetryIndex == 0) return false;
    getCenterOfGeometry(centerOfGeometry);
    computeMassPropertiesOfContour(component,contour,centerOfContour,principalAxisOfContour);
    //if(ce_contourExtraction_verbose) writeMessageToParetoLog("principalAxisOfContour: %.3g %.3g; %.3g %.3g ", principalAxisOfContour[0][0],principalAxisOfContour[0][1], principalAxisOfContour[1][0],principalAxisOfContour[1][1]);

    for (int i =0; i < 3; i++)
        centerOfSymmetry[i] = centerOfContour[i];// this may be overuled by global symmetry requirements


    symmetryTolerance = 1.5*h_dx;
    symmetryIndex = ts_symmetryIndex;

    if (symmetryIndex >= 4) zSymmetry = true;
    symmetryIndex -= 4*zSymmetry;
    if (symmetryIndex >= 2) ySymmetry = true;
    symmetryIndex -= 2*ySymmetry;
    if (symmetryIndex >= 1) xSymmetry = true;

    enforceSymmetry= false;
    // Find the plane of contour (component)
    // override the center of contour with center of geometry if appropriate
    switch (ce_sweepDirection[component]){
    case 0:// contour is on YZ plane
        if ((ySymmetry) && (fabs(centerOfContour[1] - centerOfGeometry[1]) < symmetryTolerance)) {
             enforceSymmetry= true;
             centerOfSymmetry[1] = centerOfGeometry[1];
        }
        if ((zSymmetry) &&  (fabs(centerOfContour[2] - centerOfGeometry[2]) < symmetryTolerance)) {
            enforceSymmetry= true;
            centerOfSymmetry[2] = centerOfGeometry[2];
        }
        break;
    case 1:// contour is on XZ plane
        if ((xSymmetry) &&  (fabs(centerOfContour[0] - centerOfGeometry[0]) < symmetryTolerance)){
            enforceSymmetry= true;
            centerOfSymmetry[0] = centerOfGeometry[0];
        }
        if ((zSymmetry) && (fabs(centerOfContour[2] - centerOfGeometry[2]) < symmetryTolerance)){
            enforceSymmetry= true;
            centerOfSymmetry[2] = centerOfGeometry[2];
        }
        break;
    case 2:// contour is on XY plane
        if ((xSymmetry) && (fabs(centerOfContour[0] - centerOfGeometry[0]) < symmetryTolerance) ){
            enforceSymmetry= true;
            centerOfSymmetry[0] = centerOfGeometry[0];
        }
        if ((ySymmetry) && (fabs(centerOfContour[1] - centerOfGeometry[1]) < symmetryTolerance)){
            enforceSymmetry= true;
            centerOfSymmetry[1] = centerOfGeometry[1];
        }
        break;
    }

    if (!enforceSymmetry) return false;



    iter=ce_contourData[component][contour].begin();
    pt = *iter;

    // skip the side that has the first point ... makes it easier to walk around the loop
    // gather all points on one side of the symmetry plane
    switch (ce_sweepDirection[component]){
    case 0:// contour is on YZ plane
        return false;
        break;
    case 1:// XZ plane
        break;
    case 2:// XY plane
        if (xSymmetry){
            if (pt.x < centerOfSymmetry[0])
                keepNegativeSide = false;
            else
                keepNegativeSide = true;
        }
        else if (ySymmetry){
            if (pt.y < centerOfSymmetry[1])
                keepNegativeSide = false;
            else
                keepNegativeSide = true;
        }
        break;
    }

    cout << "Symmetrizing!" << endl;


    tempListOfPoints.clear();
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;
        switch (ce_sweepDirection[component]){
        case 0:// contour is on YZ plane
            return false;
            break;
        case 1:// XZ plane
            return false;
            break;
        case 2:// XY plane
            if (xSymmetry){
                if ((keepNegativeSide) && (pt.x < centerOfSymmetry[0]))
                    tempListOfPoints.push_front(pt);
                if ((!keepNegativeSide) && (pt.x > centerOfSymmetry[0]))
                    tempListOfPoints.push_front(pt);
            }
            else if (ySymmetry){
                if ((keepNegativeSide) && (pt.y < centerOfSymmetry[1]))
                    tempListOfPoints.push_front(pt);
                if ((!keepNegativeSide) && (pt.y > centerOfSymmetry[1]))
                    tempListOfPoints.push_front(pt);
            }
            break;
        }

    }
    //copy these over
    ce_contourData[component][contour].clear();
    for( iter= tempListOfPoints.begin(); iter != tempListOfPoints.end(); ++iter) {
        pt = *iter;
        ce_contourData[component][contour].push_front(pt);
    }
    // now duplicate these points in reverse order with change of sign
    for( iter= tempListOfPoints.begin(); iter != tempListOfPoints.end(); ++iter) {
        pt = *iter;
        if (xSymmetry)
            pt.x = centerOfSymmetry[0] + (centerOfSymmetry[0] -pt.x);
        else if (ySymmetry)
            pt.y = centerOfSymmetry[1] + (centerOfSymmetry[1] -pt.y);
        else if (zSymmetry)
            pt.z = centerOfSymmetry[2] + (centerOfSymmetry[2] -pt.z);
        ce_contourData[component][contour].push_back(pt);
    }
    tempListOfPoints.clear();
    //printContourPoints( component, contour);
    //expandOrContractContour(component,contour,-0.25);//shrink contour
    return true;
}
/**********************************************************************/
void ContourExtraction::simplifyContour(int component,int contour)
{
	std::list <double> polyline; // original polyline, assume not empty
    double *result,*finalResult,tolScaling,tolerance;
    list<point>::iterator iter;
	int nNewPoints;
	point pt;


    tolScaling = MIN(1.0*ce_contourData[component][contour].size()/20,1.0);// small tolerance for small number of contour points


    tolerance = getMeshElementSize()*ce_contourError*ce_contourErrorScaling*tolScaling;

	polyline.clear();

    for(iter=ce_contourData[component][contour].begin() ; iter != ce_contourData[component][contour].end(); ++iter) {
		pt = *iter;
		polyline.push_back(pt.x);
		polyline.push_back(pt.y);
		polyline.push_back(pt.z);
	}
	// make sure the result array is large enough
	result = new double [polyline.size ()];

	// simplify the 3d polyline
	finalResult = psimpl::simplify_douglas_peucker <3> (polyline.begin (), polyline.end (),tolerance, result);

    ce_contourData[component][contour].clear();
	nNewPoints = floor((finalResult-result)/3.0);
    for (int i = 0; i <nNewPoints;i++) {
		pt.x = result[3*i];
		pt.y = result[3*i+1];
		pt.z = result[3*i+2];
        ce_contourData[component][contour].push_front(pt);
	}

	delete result;
}
/**********************************************************************/
int ContourExtraction::getNumberOfContoursInComponent(int component)
{
    if ((component < 0) || (component >= MAX_MESH_COMPONENTS )) return 0;
    return ce_numContoursInComponent[component];
}
/**********************************************************************/
int ContourExtraction::getNumberOfMeshComponents(void)
{
	return ce_nMeshComponents;
}
/**********************************************************************/
int	ContourExtraction::getMeshComponentOfElem(int elem)
{
    if ((elem < 0) || (elem >= h_nElems)) return -1;
	return h_elemComponent[elem];
}
/**********************************************************************/
void ContourExtraction::smoothenContour(int component,int contour)
{
	list<point>::iterator iterCurrent,iterPrev,iterNext;

    for( iterCurrent=ce_contourData[component][contour].begin(); iterCurrent != ce_contourData[component][contour].end(); ++iterCurrent) {
        if (iterCurrent == ce_contourData[component][contour].begin()){
            iterPrev = ce_contourData[component][contour].end();//invalid point
			iterPrev--;//one before end
		}
		else{
			iterPrev = iterCurrent;
			iterPrev--;//previous
		}
		iterNext = iterCurrent;
		iterNext++;//next
        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();

		iterCurrent->x = 0.5*(iterPrev->x + iterNext->x);
		iterCurrent->y = 0.5*(iterPrev->y + iterNext->y);
		iterCurrent->z = 0.5*(iterPrev->z + iterNext->z);
	}	
}
/**********************************************************************/
double ContourExtraction::areaOfTriangle(int projectionDir,point p1,point p2,point p3)
{
	// compute the area of a triangle defined by three points
	/// the points are in 3D but the triangle is assumed to be on XY,YZ or ZX plane
	// as defined by the projection direction
	double triArea;

	if (projectionDir == 0)// x direction
		triArea = (p1.y*p2.z-p1.z*p2.y) + (p2.y*p3.z-p2.z*p3.y) + (p3.y*p1.z-p3.z*p1.y) ;
	else if (projectionDir == 1) // y direction
		triArea = (p1.z*p2.x-p1.x*p2.z) + (p2.z*p3.x-p2.x*p3.z) + (p3.z*p1.x-p3.x*p1.z) ;
	else // z direction
		triArea = (p1.x*p2.y-p1.y*p2.x) + (p2.x*p3.y-p2.y*p3.x) + (p3.x*p1.y-p3.y*p1.x) ;

	triArea /= 2.0;
	return triArea;
}
/**********************************************************************/
void ContourExtraction::smoothenContourAreaPreserving(int component,int contour,int projectionDir)
{
	// smoothen the polygon but maintain the area
	// Reference: A Non-trignometric ... by Brown and Baird, USAF

	list<point>::iterator iterCurrent,iterPrev,iterPrevPrev,iterNext,iterNextNext;
	point p1,p2,p3,p4,p5;
	double A2,A3,A4,AvgArea,b1,b2,bsqr,delta;

    if (ce_contourData[component][contour].size() < 5){
		return;
	}

    iterPrev = ce_contourData[component][contour].end();//invalid point
	iterPrev--;//last point
	iterPrevPrev = iterPrev;
	iterPrevPrev--;// last but one point
	delta = sqrt(h_dx*h_dx + h_dy*h_dy + h_dz*h_dz);
    for( iterCurrent=ce_contourData[component][contour].begin(); iterCurrent != ce_contourData[component][contour].end(); ++iterCurrent) {
		iterNext = iterCurrent;
		iterNext++;//next
        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();

		iterNextNext = iterNext;
		iterNextNext++;

        if (iterNextNext == ce_contourData[component][contour].end())//invalid point
            iterNextNext = ce_contourData[component][contour].begin();

		p1 = *iterPrevPrev;
		p2 = *iterPrev;
		p3 = *iterCurrent;
		p4 = *iterNext;
		p5 = *iterNextNext;


		A2 = areaOfTriangle(projectionDir,p1,p2,p3);
		A3 = areaOfTriangle(projectionDir,p2,p3,p4);
		A4 = areaOfTriangle(projectionDir,p3,p4,p5);

		AvgArea = (A2+A3+A4)/3.0;

		if (projectionDir == 0){
			b1 = p4.y - p2.y;
			b2 = p4.z - p2.z;
			bsqr = b1*b1 + b2*b2;

			iterCurrent->y = (p2.y+p4.y)/2 - AvgArea*(-b2)/bsqr;
			iterCurrent->z = (p2.z+p4.z)/2 - AvgArea*(b1)/bsqr;
		}
		else if (projectionDir == 1){
			b1 = p4.z - p2.z;
			b2 = p4.x - p2.x;
			bsqr = b1*b1 + b2*b2;

			iterCurrent->z = (p2.z+p4.z)/2 - AvgArea*(-b2)/bsqr;
			iterCurrent->x = (p2.x+p4.x)/2 - AvgArea*(b1)/bsqr;
		}
		else {

			b1 = p4.x - p2.x;
			b2 = p4.y - p2.y;
			bsqr = b1*b1 + b2*b2;

			iterCurrent->x = (p2.x+p4.x)/2 - AvgArea*(-b2)/bsqr;
			iterCurrent->y = (p2.y+p4.y)/2 - AvgArea*(b1)/bsqr;
		}
		iterPrevPrev = iterPrev;
		iterPrev = iterCurrent;
	}	
}
/**********************************************************************/
void ContourExtraction::offsetLoftContours(int component)
{
	// since the contours are based on the center of the elements, we have to move the first and last contours outward

	list<point>::iterator iter;
	int contour,nContours;
    double relativeOffset = 0.9;
	
    nContours = ce_numContoursInComponent[component];
	if (nContours == 1) return;// this function only relevant for multiple contours

	// offset the first contour
	contour = 0;
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        if (ce_sweepDirection[component] == 0)
			iter->x -= relativeOffset*h_dx;
        else if (ce_sweepDirection[component] == 1)
			iter->y -= relativeOffset*h_dy;
		else 
			iter->z -= relativeOffset*h_dz;
	}

	// offset last contour
	contour = nContours-1;
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        if (ce_sweepDirection[component] == 0)
			iter->x += relativeOffset*h_dx;
        else if (ce_sweepDirection[component] == 1)
			iter->y += relativeOffset*h_dy;
		else 
			iter->z += relativeOffset*h_dz;
	}

}
/**********************************************************************/
double ContourExtraction::distanceBetweenPoints(point pt1,point pt2)
{
	return sqrt( pow(pt1.x-pt2.x,2.0) + pow(pt1.y-pt2.y,2.0)  + pow(pt1.z-pt2.z,2.0)  );
}

/**********************************************************************/
void ContourExtraction::expandOrContractContour(int component, int contour, double relativeValue)
{
    // since the contours are based on the center of the elements, we often have to expand it slightly to preserve volume

	list<point>::iterator iterCurrent,iterPrev,iterNext;
	point p2,p3,p4;
	double b1,b2,bsqr,delta;


    if (ce_contourData[component][contour].size() < 5){
        writeMessageToParetoLog("ContourExtraction: Need at least 5 points for expandContour ");
		return;
	}

    iterPrev = ce_contourData[component][contour].end();//invalid point
	iterPrev--;//last point
	delta = sqrt(h_dx*h_dx + h_dy*h_dy + h_dz*h_dz);
    for( iterCurrent=ce_contourData[component][contour].begin(); iterCurrent != ce_contourData[component][contour].end(); ++iterCurrent) {
		iterNext = iterCurrent;
		iterNext++;//next
        if (iterNext == ce_contourData[component][contour].end())//invalid point
            iterNext = ce_contourData[component][contour].begin();
		p2 = *iterPrev;
		p3 = *iterCurrent;
		p4 = *iterNext;

        if (ce_sweepDirection[component] == 0){
			b1 = p4.y - p2.y;
			b2 = p4.z - p2.z;
			bsqr = b1*b1 + b2*b2;

            iterCurrent->y = (p2.y+p4.y)/2 + relativeValue*delta*b2/sqrt(bsqr);
            iterCurrent->z = (p2.z+p4.z)/2 - relativeValue*delta*b1/sqrt(bsqr);
		}
        else if (ce_sweepDirection[component] == 1){
			b1 = p4.z - p2.z;
			b2 = p4.x - p2.x;
			bsqr = b1*b1 + b2*b2;

            iterCurrent->z = (p2.z+p4.z)/2 - relativeValue*delta*b2/sqrt(bsqr);
            iterCurrent->x = (p2.x+p4.x)/2 + relativeValue*delta*b1/sqrt(bsqr);
		}
		else {
			b1 = p4.x - p2.x;
			b2 = p4.y - p2.y;
			bsqr = b1*b1 + b2*b2;

            iterCurrent->x = (p2.x+p4.x)/2 - relativeValue*delta*b2/sqrt(bsqr);
            iterCurrent->y = (p2.y+p4.y)/2 + relativeValue*delta*b1/sqrt(bsqr);
		}
		iterPrev = iterCurrent;
	}	
}
/**********************************************************************/
bool ContourExtraction::splitComponentIfDisconnected(int component)
{
    int elem,currentElem,neighborElem;
    bool *elemVisited;
    Queue Q;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("ContourExtraction: Checking if component %d is connected", component);


    elemVisited = (bool *)malloc(h_nElems*sizeof(bool));
    memset(elemVisited,false,h_nElems*sizeof(bool));
    Q = CreateQueue(h_nElems);
    MakeEmpty(Q);
    bool firstConnectedComponentFound, disconnectedComponent;
    disconnectedComponent = false;
    firstConnectedComponentFound = false;
    for (elem = 0; elem < h_nElems;elem++) {
        if (elemVisited[elem]) continue;
        if (h_elemComponent[elem] != component) continue;
        if (h_elemComponent[elem] == -1) continue;
        if (firstConnectedComponentFound){// the rest are removed from this componet
            h_elemComponent[elem] = -1;
            disconnectedComponent = true;
            continue;
        }
        Enqueue(elem, Q); // add elem
        elemVisited[elem] = true;
        while (!IsEmpty(Q)) {
            currentElem = Front(Q);
            Dequeue(Q);
            for (int i=0;i<7;i++){// only consider 6 face neighbors + self
                neighborElem = h_StructuringElems[h_StructuringSize*currentElem+i];
                if (neighborElem == -1) continue;
                if (h_elemComponent[neighborElem] != component) continue;
                if (elemVisited[neighborElem]) continue;
                elemVisited[neighborElem] = true;
                Enqueue(neighborElem, Q);
            }
        }
        // if there are more elements, they are disconnected
        firstConnectedComponentFound = true;
    }
    MakeEmpty(Q);
    DisposeQueue(Q);
    if (disconnectedComponent){
        elemComponentList[component].clear();
        for (elem = 0; elem < h_nElems;elem++) {
            if (h_elemComponent[elem] == component)
                elemComponentList[component].push_front(elem);

        }
    }
    free(elemVisited);
    return disconnectedComponent;
}
/**********************************************************************/
bool ContourExtraction::findConnectedComponentsOfMesh(bool reset)
{
	int elem,currentComponent,currentElem,neighborElem,component,startComponent;

	if (!h_voxelgrid) {
        writeMessageToParetoLog("ContourExtraction: Connected components only works on voxel grid");
		return false;
	}

	Queue Q;
	bool isElemIn,isNeighborElemIn;

	if (reset){
		memset(h_elemComponent,-1,h_nElems*sizeof(int));// default of -1
		startComponent = 0;
	}
	else
		startComponent = ce_nMeshComponents;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("------------------------------------");
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("ContourExtraction: Finding connected components of mesh starting at component %d", startComponent);
	currentComponent = startComponent;
	Q = CreateQueue(h_nElems);
	MakeEmpty(Q);
	for (elem = 0; elem < h_nElems;elem++) {	
		if (h_elemComponent[elem] >=0) continue;// already assigned
		isElemIn = (getElemPseudoDensity(elem) > 0.5);
		Enqueue(elem, Q); // add elem
		h_elemComponent[elem] = currentComponent;
		while (!IsEmpty(Q)) {
			currentElem = Front(Q);
			Dequeue(Q);
			for (int i=0;i<7;i++){// only consider 6 face neighbors + self
				neighborElem = h_StructuringElems[h_StructuringSize*currentElem+i];
				if (neighborElem == -1) continue;
				if (h_elemComponent[neighborElem] >= 0) continue;
				isNeighborElemIn = (getElemPseudoDensity(neighborElem) > 0.5);
				if (isNeighborElemIn != isElemIn) continue;
				Enqueue(neighborElem, Q);
				h_elemComponent[neighborElem] = currentComponent;
			}
		}
		currentComponent++;
		MakeEmpty(Q);
	}
	DisposeQueue(Q);

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Number of Mesh Components = %d ",currentComponent);
	// many of these compoenents may contain very few elements ...get rid of these components

	int *tempCounter = (int *)malloc(currentComponent*sizeof(int));
	memset(tempCounter,0,currentComponent*sizeof(int));
	// count the number of elements in each component
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		if ((component < 0) || (component >= currentComponent)) continue;
		tempCounter[component]++;
	}
	if(ce_contourExtraction_verbose)
		for (component = 0; component < currentComponent;component++)
            writeMessageToParetoLog("#Elements in component %d is %d ",component,tempCounter[component]);

	//now map components
	int *mapping = (int *)malloc(currentComponent*sizeof(int));
	int counter;
	counter = 0;
	for (component = 0; component < currentComponent;component++){
		mapping[component] = -1;//default
        if (tempCounter[component] > ce_componentSizeError) {
			mapping[component] = counter;
			counter++;
		}
	}
	currentComponent = counter;
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		h_elemComponent[elem] = mapping[component];
	}
	safeFree(mapping);
	safeFree(tempCounter);
	
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Number of Mesh Components after deletion = %d ",currentComponent);
	if (currentComponent > MAX_MESH_COMPONENTS){
        writeMessageToParetoLog("Number of components (%d) exceeds max allowable of %d ",currentComponent,MAX_MESH_COMPONENTS);
		return false;
	}

	ce_nMeshComponents = currentComponent;

	for ( component = 0; component < MAX_MESH_COMPONENTS;component++) 
		elemComponentList[component].clear();

	//make a list of elements in each component for easy traversal
    double elemCenter[3];
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		if ((component < 0) || (component >= ce_nMeshComponents)) continue;
		elemComponentList[component].push_front(elem);
        getElemCenter(elem,elemCenter);
        centerOfComponent[component].x += elemCenter[0];
        centerOfComponent[component].y += elemCenter[1];
        centerOfComponent[component].z += elemCenter[2];
	}
    for ( component = 0; component < ce_nMeshComponents;component++){
        centerOfComponent[component].x /= elemComponentList[component].size();
        centerOfComponent[component].y /= elemComponentList[component].size();
        centerOfComponent[component].z /= elemComponentList[component].size();
    }

    ce_positiveComponent = -1;
    for ( component = 0; component < ce_nMeshComponents;component++){
        elem = *(elemComponentList[component].begin());
        if (getElemPseudoDensity(elem) > 0.5) {
            if (ce_positiveComponent == -1) // not assigned yet
                ce_positiveComponent = component;
            else if (component == ce_positiveComponent)// same as assigned
                ;// nothing to do
            else { // there are two different positive components that are not connected!
                writeMessageToParetoLog("Contour Extraction: Multiple positive components found (error)");
                return false;
            }
        }
    }
    if (ce_positiveComponent == -1) {
        writeMessageToParetoLog("Contour Extraction: Positive component not found (error)");
        return false;
    }
	return true;
}
/**********************************************************************/
bool ContourExtraction::findConnectedComponentsOfMesh_Material(bool reset)
{
	int elem,currentComponent,currentElem,neighborElem,component,startComponent;

	if (!h_voxelgrid) {
        writeMessageToParetoLog("ContourExtraction: Connected components only works on voxel grid");
		return false;
	}

	ce_contourExtraction_verbose = false;
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("ContourExtraction: Finding connected components of mesh");
	Queue Q;
	bool isElemIn,isNeighborElemIn;
	int ElemMaterialID,NeighborMaterialID;
	if (reset){
		memset(h_elemComponent,-1,h_nElems*sizeof(int));// default of -1
		startComponent = 0;
	}
	else
		startComponent = ce_nMeshComponents;

	currentComponent = startComponent;
	Q = CreateQueue(h_nElems);
	MakeEmpty(Q);
	for (elem = 0; elem < h_nElems;elem++) {
		if (h_elemComponent[elem] >=0) continue;

		isElemIn = (getElemPseudoDensity(elem) > 0.5);
		if (!isElemIn) continue;
		ElemMaterialID = getMaterialIdOfElem(elem);
		Enqueue(elem, Q); // add elem
		h_elemComponent[elem] = currentComponent;
		while (!IsEmpty(Q)) {
			currentElem = Front(Q);
			Dequeue(Q);
			h_elemComponent[currentElem] = currentComponent;
			for (int i=0;i<h_StructuringSize;i++){
				neighborElem = h_StructuringElems[h_StructuringSize*currentElem+i];
				if (neighborElem == -1) continue;
				isNeighborElemIn = (getElemPseudoDensity(neighborElem) > 0.5);
				if (isNeighborElemIn != isElemIn) continue;
				NeighborMaterialID = getMaterialIdOfElem(neighborElem);
				if (NeighborMaterialID != ElemMaterialID) continue;
				if ( (neighborElem != -1) && (h_elemComponent[neighborElem] == -1)){//not visited
					Enqueue(neighborElem, Q);
					h_elemComponent[neighborElem] = currentComponent;
				}
			}
		}
		currentComponent++;
	}
	DisposeQueue(Q);
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Number of Mesh Components = %d ",currentComponent);
	// many of these compoenents may contain very few elements ...get rid of these components

	int *tempCounter = (int *)malloc(currentComponent*sizeof(int));
	memset(tempCounter,0,currentComponent*sizeof(int));
	// count the number of elements in each component
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		if ((component < 0) || (component >= currentComponent)) continue;
		tempCounter[component]++;
	}
	//now map components
	int *mapping = (int *)malloc(currentComponent*sizeof(int));
	int counter;
	counter = 0;
	for (component = 0; component < currentComponent;component++){
        if(ce_contourExtraction_verbose)writeMessageToParetoLog("Component %d size is %d ",component,tempCounter[component]);
		mapping[component] = -1;//default
        if (tempCounter[component] > ce_componentSizeError) {
			mapping[component] = counter;
			counter++;
		}
	}
	currentComponent = counter;
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		h_elemComponent[elem] = mapping[component];
	}
	free(mapping);
	free(tempCounter);
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Number of Mesh Components after deletion = %d ",currentComponent);
	if (currentComponent > MAX_MESH_COMPONENTS){
        writeMessageToParetoLog("Number of components (%d) exceeds max allowable of %d ",currentComponent,MAX_MESH_COMPONENTS);
		return false;
	}

	ce_nMeshComponents = currentComponent;

	for ( component = 0; component < MAX_MESH_COMPONENTS;component++) 
		elemComponentList[component].clear();
	//make a list of elements in each component for easy traversal
	for (elem = 0; elem < h_nElems; elem++) {
		component = h_elemComponent[elem];
		if ((component < 0) || (component >= ce_nMeshComponents)) continue;
		elemComponentList[component].push_front(elem);
	}
	return true;
}

/**********************************************************************/
void ContourExtraction::computeMeanStdDev(double *x,int start,int end,double *mean,double *stddev)
{
	int i,N;
	double temp;

	N = end-start+1;
	if (N == 0)
		return;
	if (N == 1){
		*mean = x[0];
		*stddev = 0;
		return;
	}

	*mean = 0;
	*stddev = 0;
	for (i = start; i <=end; i++)
		*mean +=  x[i];
	*mean /=N;

	temp = 0;
	for (i = start; i <=end; i++)
		temp += pow((x[i]-*mean),2.0);

	*stddev = sqrt(temp/(N-1));
}
/**********************************************************************/
void ContourExtraction::computeMeanStdDev(int *x,int start,int end,double *mean,double *stddev)
{
	int i,N;
	double temp;

	N = end-start+1;
	if (N == 0)
		return;
	if (N == 1){
		*mean = x[0];
		*stddev = 0;
		return;
	}
	*mean = 0;
	*stddev = 0;

	for (i = start; i <=end; i++)
		*mean +=  x[i];
	*mean /=N;

	temp = 0;
	for (i = start; i <=end; i++)
		temp += pow(((double) x[i]-*mean),2.0);

	*stddev = sqrt(temp/(N-1));
}
/**********************************************************************/
bool ContourExtraction::splitComponentBasedOnGeometry(int component,int dir)
{
    // Split a component if its cross-section changes abruplty.

    list<int>::iterator iter;
    list<int> layerList[1000];
    int elem,layer,layerStart,layerEnd,gridIndices[3],layerToCompare,elemReference,elemToCompare;
    int nDifferences,nElemsInComponent,layerReference,nElementsInLayer;
    double elemCenterRef[3],percentError;
    bool hasBeenSplit;

    //Algorithm:
    // 1. Start with the first layer
    // 2. Set the current layer to A. If this is the last layer, quit
    // 3. Move to the next layer B
    // 4. Find X = (A-B) union (B-A)
    // 5. If |X|/|A| < error,  go to step 3
    // 6. Else increment split this component

    layerStart = 100000;
    layerEnd = -100000;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Checking for geometry split of component %d ",component);

    // first find the start and end of layers along this direction
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layerStart = MIN(layerStart,gridIndices[dir]);
        layerEnd = MAX(layerEnd,gridIndices[dir]);
    }

    writeMessageToParetoLog("layerStart = %d, layerEnd = %d ",layerStart,layerEnd);
    // group elements into layers
    for (layer = layerStart; layer <= layerEnd;layer++) // only need to initialize the relevant intermediate lists
        layerList[layer].clear();

    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[dir];
        layerList[layer].push_front(elem);
    }

    nElemsInComponent = (int) elemComponentList[component].size();
    ce_numContoursInComponent[component] = 1;
    ce_contourLayer[component][0] = layerStart;// this will be used later to generate the contour

    layerReference = layerStart;
    hasBeenSplit = false;
    for( layerToCompare = layerStart+1;layerToCompare  <= layerEnd; layerToCompare++) {
        nDifferences = 0;
        //writeMessageToParetoLog("referenceLayer = %d, layerToCompare = %d ",layerReference,layerToCompare);
        for( iter=layerList[layerReference].begin(); iter != layerList[layerReference].end(); ++iter) {
            elemReference = *iter;
            getElemCenter(elemReference,elemCenterRef);

            // move the center to the new layer
            if (dir == 0)
                elemCenterRef[dir] += (layerToCompare-layerReference)*h_dx;
            else if (dir == 1)
                elemCenterRef[dir] += (layerToCompare-layerReference)*h_dy;
            else
                elemCenterRef[dir] += (layerToCompare-layerReference)*h_dz;
            // find the element
            elemToCompare = findElemWithCenter(elemCenterRef[0],elemCenterRef[1],elemCenterRef[2]);

            if ((elemToCompare > 0) && (h_elemComponent[elemToCompare] != component) )  //element exists but belongs to a different component
                nDifferences++;
        }

        // reverse
        for( iter=layerList[layerToCompare].begin(); iter != layerList[layerToCompare].end(); ++iter) {
            elemReference = *iter;
            getElemCenter(elemReference,elemCenterRef);

            // move the center to the new layer
            if (dir == 0)
                elemCenterRef[dir] -= (layerToCompare-layerReference)*h_dx;
            else if (dir == 1)
                elemCenterRef[dir] -= (layerToCompare-layerReference)*h_dy;
            else
                elemCenterRef[dir] -= (layerToCompare-layerReference)*h_dz;
            // find the element
            elemToCompare = findElemWithCenter(elemCenterRef[0],elemCenterRef[1],elemCenterRef[2]);

            if ((elemToCompare > 0) && (h_elemComponent[elemToCompare] != component) )  //element exists but belongs to a different component
                nDifferences++;
        }

        nElementsInLayer = (layerList[layerReference].size() + layerList[layerToCompare].size())/2.0;
        percentError = 1.0*nDifferences/(nElementsInLayer);

        //if (ce_contourExtraction_verbose) writeMessageToParetoLog("GeometryMatch: laterRef: %d, layerCompare: %d; percentError %f ",layerReference,layerToCompare,percentError);
        // There are three cases to consider:
        // 1. Error is very small, continue with current contour and component
        // 2. Error is moderate, create a marker for a new contour, but continue with component
        // 3. Error is large, split this component
        if (percentError < ce_layerError*ce_layerErrorScaling) {//case 1
            // if this layer has larger number of elements, use this layer to generate contour
            if (layerList[layerToCompare].size() > layerList[layerReference].size()){
                ce_contourLayer[component][ce_numContoursInComponent[component]-1] = layerToCompare;
                layerReference = layerToCompare;
            }
            continue;// move to the next layer
        }
        else if (percentError < 5*ce_layerError*ce_layerErrorScaling) {//case 2, a multiple of above error
            writeMessageToParetoLog("Loft detected: between layer %d and layer %d ",layerReference,layerToCompare);
            ce_numContoursInComponent[component]++;
            ce_contourLayer[component][ce_numContoursInComponent[component]-1] = layerToCompare-1;// save the previous layer
            layerReference = layerToCompare;
        }
        else {// case 3: this component needs to be split
             writeMessageToParetoLog("This component needs to be split based on geometry");
             elemComponentList[component].clear();//  clear
             for (layer = layerStart; layer <= layerEnd; layer++) {// for each layer
                 for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
                     elem = *iter;
                     h_elemComponent[elem] = -1;// unassign
                 }
             }
             // push
             for (layer = layerStart; layer < layerToCompare; layer++) {// for each layer
                 for( iter=layerList[layer].begin(); iter != layerList[layer].end(); ++iter) {// for each elem in layer
                     elem = *iter;
                     h_elemComponent[elem] = component;// assign
                     elemComponentList[component].push_front(elem);
                 }
             }
             hasBeenSplit = true;
             break;
        }
    }

    if (ce_numContoursInComponent[component] > 1){
        ce_numContoursInComponent[component]++;
        ce_contourLayer[component][ce_numContoursInComponent[component]-1] = layerEnd;// for lofts save the last layer
    }
    //clear the memory
    for (layer = layerStart; layer <= layerEnd;layer++)
        layerList[layer].clear();

    return hasBeenSplit;
}
/**********************************************************************/
bool ContourExtraction::findSweepableSubset(int component,int dir)
{
    // given a component and direction, find the subset of elements that are sweepable.

    //Algorithm:
    // 1. For every element on first layer, walk through all layers
    // 2. If that element exists in every layer, mark as current component
    // 3. Else mark that element as -1

    list<int>::iterator iter;
    list<int> layerList[1000];
    int layer,elem,layerStart,layerEnd,gridIndices[3],layerToCompare,elemReference,elemToCompare;
    double elemCenterRef[3],elementCenterCurrent[3];
    bool keepElementsAlongRay,sweepable;
    IntegerSet elementSet;

    layerStart = 100000;
    layerEnd = -100000;

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("ContourExtraction: findSweepableSubset for component %d ",component);

    // first find the start and end of layers along this direction
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layerStart = MIN(layerStart,gridIndices[dir]);
        layerEnd = MAX(layerEnd,gridIndices[dir]);
    }

    // group elements into layers
    for (layer = layerStart; layer <= layerEnd;layer++) // only need to initialize the relevant intermediate lists
        layerList[layer].clear();

    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layer = gridIndices[dir];
        layerList[layer].push_front(elem);
    }

    cout << "Before " << elemComponentList[component].size() <<endl;

    elemComponentList[component].clear();
    sweepable = true;

    // for all elements in the first layer
    for( iter=layerList[layerStart].begin(); iter != layerList[layerStart].end(); ++iter) {// for all elements in first layer
        elemReference = *iter;
        getElemCenter(elemReference,elemCenterRef);
        getElemCenter(elemReference,elementCenterCurrent);

        keepElementsAlongRay = true;
        elementSet.clear();
        elementSet.insertNoCheckForUnique(elemReference);
        // for every other layer
        for( layerToCompare = layerStart+1;layerToCompare  <= layerEnd; layerToCompare++) {
            // move the center to the new layer
            if (dir == 0)
                elementCenterCurrent[dir] = elemCenterRef[dir] + (layerToCompare-layerStart)*h_dx;
            else if (dir == 1)
                elementCenterCurrent[dir] = elemCenterRef[dir] + (layerToCompare-layerStart)*h_dy;
            else
                elementCenterCurrent[dir] = elemCenterRef[dir] + (layerToCompare-layerStart)*h_dz;
            // find the element
            elemToCompare = findElemWithCenter(elementCenterCurrent[0],elementCenterCurrent[1],elementCenterCurrent[2]);

            if (elemToCompare < 0)// empty space
                continue;
            if (h_elemComponent[elemToCompare] != component ) { //element does not exist or belongs to a different component
                keepElementsAlongRay = false;// all elements along this ray are removed from component
                sweepable = false;//
                break;
            }
            else
                elementSet.insertNoCheckForUnique(elemToCompare);
        }
        if (keepElementsAlongRay){
            for (int i = 0; i < elementSet.size(); i++){
                h_elemComponent[elementSet.elementAt(i)] = component;
                elemComponentList[component].push_back(elementSet.elementAt(i));
            }
        }
        else{
            for (int i = 0; i < elementSet.size(); i++)
                h_elemComponent[elementSet.elementAt(i)] = -1;
        }
    }
    if (sweepable){// all rays sweepable
        ce_numContoursInComponent[component] = 1;
        ce_contourLayer[component][0] = layerStart;
        if(ce_contourExtraction_verbose)
            writeMessageToParetoLog("ContourExtraction: Component %d is sweepable ",component);
    }
    else {
        if(ce_contourExtraction_verbose)
            writeMessageToParetoLog("ContourExtraction: Component %d is NOT sweepable ",component);
    }
    cout << "After " << elemComponentList[component].size() <<endl;
    return sweepable;
}
/**********************************************************************/
int ContourExtraction::getDepthOfComponentAlongDir(int component,int dir)
{
    int layerStart,layerEnd,gridIndices[3],elem;
    list<int>::iterator iter;
    layerStart = 100000;
    layerEnd = -100000;
    //if(ce_contourExtraction_verbose) writeMessageToParetoLog("CountourExtraction: Estimating depth of component %d along dir %d ",component, dir);
    // first find the start and end of layers along this direction
    for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;
        getGridIndicesOfElem(elem,gridIndices);
        layerStart = MIN(layerStart,gridIndices[dir]);
        layerEnd = MAX(layerEnd,gridIndices[dir]);
    }
    return (layerEnd - layerStart );
}
/**********************************************************************/
void ContourExtraction::estimateNumberOfContoursInMeshComponent(int component, int dir, int *nContours, int *extrudeThickness)
{
    list<int>::iterator iter;
	list<int> layerList[1000];
	int elem,layer,layerStart,layerEnd,gridIndices[3],referenceLayer,layerToCompare,elemReference,elemToCompare;
    int nDifferences,nElemsInComponent,nElementsMaxInCurrentLayer;
    double elemCenterRef[3],elementCenterCurrent[3],percentError;

	//Algorithm: Shape Matching of XSections (layers)
	// 1. Start with the first layer, set number of contours N = 1 
	// 2. Set the current layer to A. If this is the last layer, quit
	// 3. Move to the next layer B
	// 4. Find X = (A-B) union (B-A)
	// 5. If |X|/|A| < error,  go to step 3
	// 6. Else increment N, go to step 2


    // In Step 4, we have an option to disregard empty space or not.
    bool disregardEmptySpace = true;// default to true, set to false for debugging

	layerStart = 100000;
	layerEnd = -100000;
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("CountourExtraction: Estimating #contours in component %d along dir %d ",component, dir);
	// first find the start and end of layers along this direction
	for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
		elem = *iter;
		getGridIndicesOfElem(elem,gridIndices);
		layerStart = MIN(layerStart,gridIndices[dir]);
		layerEnd = MAX(layerEnd,gridIndices[dir]);
	}
	// group elements into layers
	for (layer = layerStart; layer <= layerEnd;layer++) // only need to initialzie the relevant intermediate lists
		layerList[layer].clear();

	for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
		elem = *iter;
		getGridIndicesOfElem(elem,gridIndices);
		layer = gridIndices[dir];
		layerList[layer].push_front(elem);
	}
	
	nElemsInComponent = (int) elemComponentList[component].size();
	referenceLayer = layerStart;
	layerToCompare = referenceLayer+1;
    nElementsMaxInCurrentLayer =layerList[layerToCompare].size();
	*nContours = 1;

	while( referenceLayer < layerEnd) {
		nDifferences = 0;
        if (layerList[layerToCompare].size() > nElementsMaxInCurrentLayer){
            nElementsMaxInCurrentLayer = layerList[layerToCompare].size();
        }
		for( iter=layerList[referenceLayer].begin(); iter != layerList[referenceLayer].end(); ++iter) {
			elemReference = *iter;
            getElemCenter(elemReference,elemCenterRef);// fetch each time

			// move the center to the new layer
			if (dir == 0)
				elemCenterRef[dir] += (layerToCompare-referenceLayer)*h_dx;
			else if (dir == 1)
				elemCenterRef[dir] += (layerToCompare-referenceLayer)*h_dy;
			else
				elemCenterRef[dir] += (layerToCompare-referenceLayer)*h_dz;
			// find the element
			elemToCompare = findElemWithCenter(elemCenterRef[0],elemCenterRef[1],elemCenterRef[2]);

            if (disregardEmptySpace){
                if ((elemToCompare >= 0) && (h_elemComponent[elemToCompare] != component)) //element is a different component
                    nDifferences++;
            }
            else {
                if ((elemToCompare < 0) || (h_elemComponent[elemToCompare] != component) )  //element does not exist or a different component
                    nDifferences++;
            }
		}

		// reverse
		for( iter=layerList[layerToCompare].begin(); iter != layerList[layerToCompare].end(); ++iter) {
			elemReference = *iter;
			getElemCenter(elemReference,elemCenterRef);

			// move the center to the new layer
			if (dir == 0)
				elemCenterRef[dir] -= (layerToCompare-referenceLayer)*h_dx;
			else if (dir == 1)
				elemCenterRef[dir] -= (layerToCompare-referenceLayer)*h_dy;
			else
				elemCenterRef[dir] -= (layerToCompare-referenceLayer)*h_dz;
			// find the element
			elemToCompare = findElemWithCenter(elemCenterRef[0],elemCenterRef[1],elemCenterRef[2]);

            if (disregardEmptySpace){
                if ((elemToCompare >= 0) && (h_elemComponent[elemToCompare] != component)) //element is a different component
                    nDifferences++;
            }
            else {
                if ((elemToCompare < 0) || (h_elemComponent[elemToCompare] != component) )  //element does not exist or a different component
                    nDifferences++;
            }
		}
		percentError = 1.0*nDifferences/(nElemsInComponent);
        if (percentError < ce_layerError*ce_layerErrorScaling) {// use the same contour
			layerToCompare++;
		}
		else {
			*nContours += 1;
			referenceLayer = layerToCompare;
			layerToCompare = referenceLayer+1;
			if (*nContours >= MAX_CONTOURS_PER_COMPONENT-1) {
				break;
			}
		}
		if (layerToCompare > layerEnd)
			break;
	}
    *extrudeThickness = layerEnd - layerStart;
	//clear the memory
	for (layer = layerStart; layer <= layerEnd;layer++) 
		layerList[layer].clear();

}
/**********************************************************************/
bool ContourExtraction::findBoundaryElementOfMeshComponent(int component, int contour, int sweepDirection, int boundaryElementIndices[3])
{
    int squareValue,contourLayer,elem;
    list<int>::iterator iter;

    contourLayer = ce_contourLayer[component][contour];
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("CountourExtraction: Contour layer is %d  ",contourLayer);

    // find a boundary voxel in this component, on the given layer
	for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
        elem = *iter;// walk thru every element in component
        getGridIndicesOfElem(elem,boundaryElementIndices);//get its grid location
        if (boundaryElementIndices[sweepDirection] != contourLayer) continue;// if the element does not belong to the layer of interest, continue;
        if (sweepDirection == 0)
			squareValue = getPixelValueYZ(component,boundaryElementIndices[0],boundaryElementIndices[1],boundaryElementIndices[2]);
		else if (sweepDirection == 1)
			squareValue = getPixelValueZX(component,boundaryElementIndices[0],boundaryElementIndices[1],boundaryElementIndices[2]);
		else 
			squareValue = getPixelValueXY(component,boundaryElementIndices[0],boundaryElementIndices[1],boundaryElementIndices[2]);

        if (squareValue >=1 && squareValue < 15 && squareValue != 9 && squareValue != 8){// check if it is a boundary element
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Found boundary element ");

            return true;
		}
	}
    if(ce_contourExtraction_verbose) writeMessageToParetoLog("CountourExtraction: Unable to find boundary element ");

	return false;
}
/**********************************************************************/
void ContourExtraction::findMeshExtents(int component, double extents[2])
{
	list<int>::iterator iter;
	int i,elem,direction;
	int xyzGridIndexMin[3],xyzGridIndexMax[3],gridIndices[3];
    double rndValue1,rndValue2;

	for (i = 0; i < 3; i++){
		xyzGridIndexMin[i] = 1000000;
		xyzGridIndexMax[i] = -1000000;
	}
	for( iter=elemComponentList[component].begin(); iter != elemComponentList[component].end(); ++iter) {
		elem = *iter;
		getGridIndicesOfElem(elem,gridIndices);
		for (i = 0; i < 3; i++){
			xyzGridIndexMin[i] = MIN(xyzGridIndexMin[i],gridIndices[i]);
			xyzGridIndexMax[i] = MAX(xyzGridIndexMax[i],gridIndices[i]);
		}
	}

    rndValue1 = 1.1;
    rndValue2 = 2.2;
    direction = ce_sweepDirection[component];
	if (direction  == 0) {
        if (xyzGridIndexMin[0] == 0)
            extents[0] = h_xMin + (xyzGridIndexMin[0]+0.5)*h_dx-rndValue2*h_dx;// go beyond solid
        else
            extents[0] = h_xMin + (xyzGridIndexMin[0]+0.5)*h_dx-rndValue1*h_dx;//offset slightly to avoid slivers

        if (xyzGridIndexMax[0] == (h_nelx-1))
            extents[1] = h_xMin + (xyzGridIndexMax[0]+0.5)*h_dx+rndValue2*h_dx;
        else
           extents[1] = h_xMin + (xyzGridIndexMax[0]+0.5)*h_dx+rndValue1*h_dx;

	}
	else if (direction  == 1) {
        if (xyzGridIndexMin[1] == 0)
            extents[0] = h_yMin + (xyzGridIndexMin[1]+0.5)*h_dy-rndValue2*h_dy;
        else
            extents[0] = h_yMin + (xyzGridIndexMin[1]+0.5)*h_dy-rndValue1*h_dy;

        if (xyzGridIndexMax[1] == (h_nely-1))
            extents[1] = h_yMin + (xyzGridIndexMax[1]+0.5)*h_dy+rndValue2*h_dy;
        else
            extents[1] = h_yMin + (xyzGridIndexMax[1]+0.5)*h_dy+1.05*h_dy;
	}
	else if (direction  == 2) {
        if (xyzGridIndexMin[2] == 0)
            extents[0] = h_zMin + (xyzGridIndexMin[2]+0.5)*h_dz-rndValue2*h_dz;
        else
            extents[0] = h_zMin + (xyzGridIndexMin[2]+0.5)*h_dz-rndValue1*h_dz;//offset slightly to avoid slivers

        if (xyzGridIndexMax[2] == (h_nelz-1))
            extents[1] = h_zMin + (xyzGridIndexMax[2]+0.5)*h_dz+rndValue2*h_dz;
        else
            extents[1] = h_zMin + (xyzGridIndexMax[2]+0.5)*h_dz+rndValue1*h_dz;
	}
}
/**********************************************************************/
void ContourExtraction::printComponentData(void)
{
    writeMessageToParetoLog("------------------------------------------------------------");
    for (int component = 0; component < ce_nMeshComponents;component++)
        writeMessageToParetoLog("Component: %d; Elements = %d ",component,elemComponentList[component].size());
}
/**********************************************************************/
void ContourExtraction:: copyContourWithSymmetry(int fromComponentA, int toComponentB, int symmetryIndex)
{
    // assumes that symmetry has been verified! See areTwoComponentsSymmetric()
    int nContours;
    list<point>::iterator iter;
    point pt;
    double center[3];
    bool reverse;

    ce_sweepDirection[toComponentB] = ce_sweepDirection[fromComponentA];
    ce_numContoursInComponent[toComponentB] = ce_numContoursInComponent[fromComponentA];

    nContours = ce_numContoursInComponent[toComponentB];

    getCenterOfGeometry(center);
    reverse = false;
    for (int contour = 0; contour < nContours; contour++){
        for( iter=ce_contourData[fromComponentA][contour].begin(); iter != ce_contourData[fromComponentA][contour].end(); ++iter) {
             pt = *iter;
             switch(symmetryIndex){
             case 1:// x-symmetry
                 pt.x = 2*center[0] - pt.x;
                 break;
             case 2:// y-symmetry
                 pt.y = 2*center[1] - pt.y;
                 break;
             case 4:// z-symmetry
                 pt.z = 2*center[2] - pt.z;
                 break;
             }
             ce_contourData[toComponentB][nContours - contour-1].push_front(pt);
        }
    }
}
/**********************************************************************/
void ContourExtraction:: copyContourWithAxiSymmetry(int fromComponentA, int toComponentB,  double rotationAngle)
{
    // assumes that symmetry has been verified! See areTwoComponentsSymmetric()
    int nContours;
    list<point>::iterator iter;
    point pt;
    double geomCenter[3],x0,y0;

    ce_sweepDirection[toComponentB] = ce_sweepDirection[fromComponentA];
    ce_numContoursInComponent[toComponentB] = ce_numContoursInComponent[fromComponentA];

    nContours = ce_numContoursInComponent[toComponentB];
    getCenterOfGeometry(geomCenter);
    for (int contour = 0; contour < nContours; contour++){
        for( iter=ce_contourData[fromComponentA][contour].begin(); iter != ce_contourData[fromComponentA][contour].end(); ++iter) {
             pt = *iter;

             x0 = pt.x - geomCenter[0];// with respect to local coordinate system
             y0 = pt.y - geomCenter[1];
             // z does not change

             pt.x = geomCenter[0] + x0*cos(rotationAngle) - y0*sin(rotationAngle);
             pt.y = geomCenter[1] + x0*sin(rotationAngle) + y0*cos(rotationAngle);

             ce_contourData[toComponentB][contour].push_back(pt);
        }
    }
}
/**********************************************************************/
bool ContourExtraction:: checkForAxiSymmetry(int componentA, int componentB, int axiSymmetryIndex,double *rotationAngle)
{
    // algorithm
    // 1. First estimate the rotation needed to align A and B (assuming they are axi symmetric images)
    // 2. for every element of A, find the symmetric element and check if it belongs to B
    // 3. for every element of B, find the symmetric element and check if it belongs to A

    list<int>::iterator iter;
    int elemA,elemSymmetric,errorCounter,elemDiff;
    double centerElemA[3],centerSymmetricElem[3],geomCenter[3],x0,y0,radiusA,radiusB;
    double angleOfSymmetry,angle,centerA[3],centerB[3],rotatedCenter[3];
    point centerOfA,centerOfB;

    if (axiSymmetryIndex <=1 ) return false;

    elemDiff = fabs(1.0*elemComponentList[componentA].size() - 1.0*elemComponentList[componentB].size());

    //if(ce_contourExtraction_verbose) cout << "Checking for axisymmetry between " << componentA << " and " << componentB << endl;

    if (elemDiff > ce_symmetryErrorScaling*ce_componentSizeError )
        return false;

    getCenterOfGeometry(geomCenter);
    errorCounter = 0;
    angleOfSymmetry = 2*3.14159265359/axiSymmetryIndex;


    centerOfA = centerOfComponent[componentA];
    centerOfB = centerOfComponent[componentB];

    centerA[0] = centerOfA.x-geomCenter[0];
    centerA[1] = centerOfA.y-geomCenter[1];
    centerA[2] = centerOfA.z-geomCenter[2];

    centerB[0] = centerOfB.x-geomCenter[0];
    centerB[1] = centerOfB.y-geomCenter[1];
    centerB[2] = centerOfB.z-geomCenter[2];


    radiusA = NORM3D(centerA);
    radiusB = NORM3D(centerB);
    if (fabs(radiusA - radiusB) > 1.5*h_dx) return false;

    //find the angle between the two vectors
    angle = acos(DOTPROD3D(centerA,centerB)/(radiusA*radiusB));

    // check if this angle is a multiple of the angleOfSymmetry
    int multiple;
    multiple =NEARESTINT(angle/angleOfSymmetry);

    if ( fabs(multiple - angle/angleOfSymmetry) > 0.05) return false;

    angle = multiple*angleOfSymmetry;
    // can also be -angle, need to check

    rotatedCenter[0] = centerA[0]*cos(angle) - centerA[1]*sin(angle);
    rotatedCenter[1] = centerA[0]*sin(angle) + centerA[1]*cos(angle);
    rotatedCenter[2] = centerA[2];

    if (DIST3D(rotatedCenter,centerB) > 1.5*h_dx){
        angle = -angle;//try reversing
        rotatedCenter[0] = centerA[0]*cos(angle) - centerA[1]*sin(angle);
        rotatedCenter[1] = centerA[0]*sin(angle) + centerA[1]*cos(angle);
        rotatedCenter[2] = centerA[2];

        if (DIST3D(rotatedCenter,centerB) > 1.5*h_dx)
            return false;
    }

    // now proceed with comparison of elements
    for( iter=elemComponentList[componentA].begin(); iter != elemComponentList[componentA].end(); ++iter) {
        elemA = *iter;
        getElemCenter(elemA,centerElemA);

        x0 = centerElemA[0] - geomCenter[0];// with respect to local coordinate system
        y0 = centerElemA[1] - geomCenter[1];

        centerSymmetricElem[2] = centerElemA[2];

        centerSymmetricElem[0] = geomCenter[0] + x0*cos(angle) - y0*sin(angle);
        centerSymmetricElem[1] = geomCenter[1] + x0*sin(angle) + y0*cos(angle);

        elemSymmetric = findElemWithCenter(centerSymmetricElem[0],centerSymmetricElem[1],centerSymmetricElem[2],1.5);

        if (getMeshComponentOfElem(elemSymmetric) != componentB){
            errorCounter++;
            if (errorCounter > ce_symmetryErrorScaling*ce_componentSizeError ){
                return false;
            }
        }
    }
    *rotationAngle = angle;
    return true;

}
/**********************************************************************/
bool ContourExtraction:: checkForPlanarSymmetry(int componentA, int componentB, int symmetryIndex)
{
    // algorithm
    // 0. Basic checks on size, and center, then
    // 1. for every element of A, find the symmetric element and check if it belongs to B
    // 2. for every element of B, find the symmetric element and check if it belongs to A

    list<int>::iterator iter;
    int elemA,elemSymmetric,elemB,errorCounter;
    double centerElemA[3],centerSymmetricElem[3],center[3],centerElemB[3],elemDiff;

    if (symmetryIndex <=0 ) return false;// no symmetry

    // cannot check for double symmetry here!!
    if ((symmetryIndex == 3) || (symmetryIndex == 5) || (symmetryIndex == 6) || (symmetryIndex == 7)) return false;

    elemDiff = fabs(1.0*elemComponentList[componentA].size() - 1.0*elemComponentList[componentB].size());

    //if(ce_contourExtraction_verbose) cout << "Checking for planar symmetry between " << componentA << " and " << componentB << endl;


    if (elemDiff > ce_symmetryErrorScaling*ce_componentSizeError  )
        return false;

    getCenterOfGeometry(center);
    errorCounter = 0;

    for( iter=elemComponentList[componentA].begin(); iter != elemComponentList[componentA].end(); ++iter) {
        elemA = *iter;
        getElemCenter(elemA,centerElemA);
        for (int i =0; i < 3; i++)
            centerSymmetricElem[i] = centerElemA[i];

        switch(symmetryIndex){
        case 1:// x-symmetry
            centerSymmetricElem[0] = 2*center[0] - centerSymmetricElem[0];
            break;
        case 2:// y-symmetry
            centerSymmetricElem[1] = 2*center[1] - centerSymmetricElem[1];
            break;
        case 4:// z-symmetry
            centerSymmetricElem[2] = 2*center[2] - centerSymmetricElem[2];
            break;
        }
        elemSymmetric = findElemWithCenter(centerSymmetricElem[0],centerSymmetricElem[1],centerSymmetricElem[2],0.25);
        if (getMeshComponentOfElem(elemSymmetric) != componentB){
            errorCounter++;
            if (errorCounter > ce_symmetryErrorScaling*ce_componentSizeError )
                return false;
        }
    }

    for( iter=elemComponentList[componentB].begin(); iter != elemComponentList[componentB].end(); ++iter) {
        elemB = *iter;
        getElemCenter(elemB,centerElemB);
        for (int i =0; i < 3; i++)
            centerSymmetricElem[i] = centerElemB[i];
        switch(symmetryIndex){
        case 1:// x-symmetry
            centerSymmetricElem[0] = 2*center[0] - centerSymmetricElem[0];
            break;
        case 2:// y-symmetry
            centerSymmetricElem[1] = 2*center[1] - centerSymmetricElem[1];
            break;
        case 4:// z-symmetry
            centerSymmetricElem[2] = 2*center[2] - centerSymmetricElem[2];
            break;
        }
        elemSymmetric = findElemWithCenter(centerSymmetricElem[0],centerSymmetricElem[1],centerSymmetricElem[2],0.25);
        if (getMeshComponentOfElem(elemSymmetric) != componentA) {
            errorCounter++;
            if (errorCounter > ce_symmetryErrorScaling*ce_componentSizeError )
                return false;
        }
    }

    return true;
}
/**********************************************************************/
double ContourExtraction::getAreaEnclosedByContour(int component, int contour)
{
    list<point>::iterator iter;
    point pt,prevPt;
    double polygonArea;
    int direction;

    iter = ce_contourData[component][contour].end();
    iter--;
    prevPt = *iter;//last pt

    polygonArea = 0;
    direction = ce_sweepDirection[component];
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;

        switch (direction){
            case 0: polygonArea -=  (prevPt.y*pt.z - pt.y*prevPt.z); break;
            case 1: polygonArea -=  (prevPt.z*pt.x - pt.z*prevPt.x); break;
            case 2: polygonArea -=  (prevPt.x*pt.y - pt.x*prevPt.y);break;
        }
        prevPt = pt;
    }
    polygonArea /=2;
    return polygonArea;
}

/**********************************************************************/
void ContourExtraction::matchContours(void)
{
    // move vertices so that contours match
    list<point>::iterator iter1,iter2;
    point pt1,pt2;


    for (int component1 = 0; component1 < ce_nMeshComponents;component1++) {
        for (int contour1 = 0; contour1 < ce_numContoursInComponent[component1]; contour1++) {
            for( iter1=ce_contourData[component1][contour1].begin(); iter1 != ce_contourData[component1][contour1].end(); ++iter1) {
                pt1 = *iter1;
                for (int component2 = component1+1; component2 < ce_nMeshComponents;component2++) {
                    for (int contour2 = 0; contour2 < ce_numContoursInComponent[component2]; contour2++) {
                        for( iter2 = ce_contourData[component2][contour2].begin(); iter2 != ce_contourData[component2][contour2].end(); ++iter2) {
                            pt2 = *iter2;
                            if (distanceBetweenPoints(pt1,pt2) < 0.001*getModelScale()){
                                cout << "matching points " << endl;
                                iter2->x = pt1.x;
                                iter2->y = pt1.y;
                                iter2->z = pt1.z;
                            }
                        }
                    }
                }
            }
        }
    }
}
/**********************************************************************/
void ContourExtraction::reorientContours(void)
{
    double contourArea;
    list<point>::iterator iter;
    point pt;
    list<point> tempListOfPoints;

    for (int component = 0; component < ce_nMeshComponents;component++) {
        for (int contour = 0; contour < ce_numContoursInComponent[component]; contour++) {
             contourArea = getAreaEnclosedByContour(component, contour);
             if (contourArea < 0){// reorient
                 tempListOfPoints.clear();
                 for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
                     pt = *iter;
                     tempListOfPoints.push_back(pt);
                 }
                 ce_contourData[component][contour].clear();
                 for( iter=tempListOfPoints.begin(); iter != tempListOfPoints.end(); ++iter) {
                     pt = *iter;
                     ce_contourData[component][contour].push_front(pt);
                 }
             }
        }
    }
}
/**********************************************************************/
void ContourExtraction::printContourPoints(int component, int contour)
{
    list<point>::iterator iter;
    point pt;
    cout << "Component: " << component << "; Contour: " << contour << endl;
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;
        cout << pt.x << " " << pt.y << " "  << pt.z << endl;
    }
}
/**********************************************************************/
void ContourExtraction::printBasicContourData(void)
{
    writeMessageToParetoLog("------------------------------------------------------------");
	for (int component = 0; component < ce_nMeshComponents;component++) {
        writeMessageToParetoLog("Component: %d; #Elements %d; #Contours %d; Dir %d ",
            component,elemComponentList[component].size(),ce_numContoursInComponent[component],ce_sweepDirection[component]);
	}
}
/**********************************************************************/
int       ContourExtraction:: getNumberOfPointsOnContour(int component,int contour)
{
    if ((component < 0) || (component >= ce_nMeshComponents)) return 0;
    if ((contour < 0) || (contour >= ce_numContoursInComponent[component])) return 0;

    return ce_contourData[component][contour].size();
}
/**********************************************************************/
bool      ContourExtraction:: getPointOnContour(int component,int contour,int ptIndex,double pt[3])
{
    if ((component < 0) || (component >= ce_nMeshComponents)) return false;
    if ((contour < 0) || (contour >= ce_numContoursInComponent[component])) return false;
    if ((ptIndex < 0) || (ptIndex >= ce_contourData[component][contour].size())) return false;

    std::list<point>::iterator iter = ce_contourData[component][contour].begin();

    std::advance(iter, ptIndex);
    pt[0] = (*iter).x;
    pt[1] = (*iter).y;
    pt[2] = (*iter).z;
    return true;
}

/**********************************************************************/
void ContourExtraction:: dumpContourData(char *fileName)
{
	list<point>::iterator iter;
	point pt;
	ofstream myfile;
	myfile.open (fileName);
	
	myfile << "# Contour format \n";
	myfile << "# number of mesh components \n";
	myfile << "# NumOfContoursInComponent_0 \n";
	myfile << "# NumberOfPtsInContour0_ofComponent0 \n";
	myfile << "# x y z \n";
	myfile << "# ... \n";
	myfile << "# NumberOfPtsInContour1_ofComponent0 \n";
	myfile << "# x y z \n";
	myfile << "# ...\n";
	myfile << "# NumOfContoursInComponent_1 \n";
	myfile << "# NumberOfPtsInContour0_ofComponent1 \n";
	myfile << "# ... \n";
	myfile << ce_nMeshComponents << endl;
	for (int component = 0; component < ce_nMeshComponents; component++){
        myfile  << ce_numContoursInComponent[component] << endl;
        for (int contour = 0; contour < ce_numContoursInComponent[component]; contour++) {
            myfile << ce_contourData[component][contour].size() << endl;
            for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
				pt = *iter;
				myfile << pt.x << " " << pt.y << " "  << pt.z << endl;
			}
		}
	}
	myfile.close();
}

/**********************************************************************/
void ContourExtraction::generateContourOfMeshComponents(double relativeTopologyError,
                                                        double relativeSurfaceError,
                                                        int nSmootheningCycles,
                                                        int relativeOffset)
{

    /* Given a topology optimized design, the goal is to create a simplified representation.
     *
     * The key idea is to construct a set of quasi-disjoint mesh components of the negative space, such that
     * each mesh component can be represented (approximately) by a sweep with a single external contour, or a loft with
     * multiple external contours, or a sweep with a single external contour and multiple internal contours.
     *
     * For each (negative) mesh component, the first step is to determine the ideal sweep direction.
     * If the topology optimization has a draw-direction or a through-cut direction, this is used as the sweep direction.
     * If not, we estimate the number of contours along x, y and z direction, and pick the direction with the least number
     * of contours
     *
     * Once a sweep direction is determine, we may need to further split a mesh component if
     * (a) the mesh component has branches (called the island problem)
     * (b) the mesh component cannot be accurately represented by a single sweep or loft
     *
     * If the topology has implied symmetry, then we can avoid generating contours by exploiting the contour of
     * a previously generated component that is symmetric to this component
     *
     * The user controls include
     * (1) relativeComponentTolerance:  between 1 and 10 used in discarding small mesh components and for symmetry checks
     *         Smaller values will retain small components
     * (2) relativeLayerError: between 1 and 10 used in splitting a mesh component into different layers
     *         Smaller values will create more layers
     * (3) relativeContourError: between 1 and 10 used in approximating a contour by finite line segments
     *         Smaller values will create more line segments
     * (4) nSmootheningCycles: between 1 and 10 to smoothen a contour
     *         Smaller values will lead to coarser contour
     * (5) relativeOffset: between 1 and 10 used to offset a contour near corners, to avoid slivers
     *         Smaller values will lead to smaller offset (and possibly slivers)
     * */

    int component,gridIndices[3],contour,nContoursEstimated;

	if (!h_voxelgrid) {
        writeMessageToParetoLog("ContourExtraction: generateContourOfMeshComponents only works on voxel grid");
		return;
	}

    initializeContours();

    ce_componentErrorScaling = 5*relativeTopologyError;
    ce_layerErrorScaling = 10*relativeSurfaceError;
    ce_contourErrorScaling = 5*relativeSurfaceError;

    ce_componentSizeError = NEARESTINT(ce_componentError*ce_componentErrorScaling*getCurrentVolumeFractionOfOptimizedParts()*h_nElems);

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("All components less than %d elements will be discarded",ce_componentSizeError);
	//clear all content
	for (component = 0; component < MAX_MESH_COMPONENTS;component++) {
        ce_sweepDirection[component] = -1;// by default no preferred direction
        ce_numContoursInComponent[component] = 0;// 0 contours computed

        for (contour = 0; contour < MAX_CONTOURS_PER_COMPONENT; contour++) {
            ce_contourLayer[component][contour] = 0;//erase any previous data
            ce_contourData[component][contour].clear();//erase any previous data
            ce_isContourInternal[component][contour] = false;// all are external contours by default
        }
	}
	//find connected components of mesh
    // the positive and negative components are seperated.
    // components not topologically connected are considered different components
    // assumption: only one positive component!!
    if (!findConnectedComponentsOfMesh(true))
		return;

	if(ce_contourExtraction_verbose) printComponentData();

    // Now walk through all components
	for (component = 0; component < ce_nMeshComponents ;component++) {
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("------------------------------------------------------------");
        if(ce_contourExtraction_verbose)  writeMessageToParetoLog("Component: %d out of %d; size = %d ",component,ce_nMeshComponents,elemComponentList[component].size());

        if (component == ce_positiveComponent) {
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Positive component ... skipping ");
            continue; // neglect all mesh components with  elements inside
        }
        // check if there is a previously processed component that is symmetric to this component
        if (findSymmetricComponent(component)) continue;

        ce_sweepDirection[component]  = findOptimalDirection(component, &nContoursEstimated);// optimal direction is stored in  ce_sweepDirection

        if(ce_contourExtraction_verbose)  writeMessageToParetoLog("Component: optimal direction is %d ",ce_sweepDirection[component]);

       /* if (!findSweepableSubset(component,ce_sweepDirection[component])) {//additional components were created
            if (!findConnectedComponentsOfMesh(false)) // find unassigned elements and delete small components
                return;// if failure return
            if(ce_contourExtraction_verbose) printComponentData();
            component--;
            continue;
        }*/

        // split component if there are differences in external contours in its layers
        if (splitComponentBasedOnExternalContours(component,ce_sweepDirection[component]) > 1) {//additional components were created
            if (!findConnectedComponentsOfMesh(false)) // find unassigned elements and delete small components
                return;// if failure return
            if(ce_contourExtraction_verbose) printComponentData();
            component--;
            continue;
        }

        // split component if there are differences in internal contours in its layers
        if (splitComponentBasedOnInternalContours(component,ce_sweepDirection[component]) > 1) {//additional components were created
            if (!findConnectedComponentsOfMesh(false)) // find unassigned elements and delete small components
                return;// if failure return
            if(ce_contourExtraction_verbose) printComponentData();
            component--;
            continue;
        }

        //split component based on geometric differences between layers
        if (splitComponentBasedOnGeometry(component,ce_sweepDirection[component])){
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Splitting component %d based on geometry ",component);
            if (!findConnectedComponentsOfMesh(false))// once again find unassigned elements
                return;// if failure return

            if(ce_contourExtraction_verbose) printComponentData();
            component--;
            continue;
        }

        //check if the component has become disconnected
        if (splitComponentIfDisconnected(component)){
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Splitting disconnected component %d ",component);
            if (!findConnectedComponentsOfMesh(false))// once again find unassigned elements
                return;// if failure return
            if(ce_contourExtraction_verbose) printComponentData();
        }
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("---------------------------------------------------");
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("Done splitting; attempting to create contours");
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("Component %d; Picking dir: %d with %d contours ",component,ce_sweepDirection[component],ce_numContoursInComponent[component] );

		// now create the contours
        if(ce_contourExtraction_verbose) writeMessageToParetoLog("Direction: %d, nContours = %d ",ce_sweepDirection[component] ,ce_numContoursInComponent[component]);
        for ( contour = 0; contour < ce_numContoursInComponent[component]; contour++) {
            // for each contour, find a boundary element
            if (!findBoundaryElementOfMeshComponent(component,contour,ce_sweepDirection[component],gridIndices)) {// find a boundary element to start tracing
                cout << "Boundary element not found " << endl;
				continue;
			}
            generateContour(component,contour,2*relativeOffset,gridIndices );
            for (int i = 0; i < nSmootheningCycles; i++)
                smoothenContourAreaPreserving(component,contour,ce_sweepDirection[component] );
            simplifyContour(component,contour);
            //expandOrContractContour(component,contour);
            imposeSymmetryOnContour(component,contour);
		}
        if (ce_numContoursInComponent[component] > 1) {
            // offset the initial and last layers
			offsetLoftContours(component);
            if(ce_contourExtraction_verbose) writeMessageToParetoLog("Finished Offseting Lofts");

            //find the  contour with the least number of contour pts (called the master contour)
            int masterContour,maxContourSize;
            masterContour = 0;
            maxContourSize = ce_contourData[component][masterContour].size();
            for ( contour = 1; contour < ce_numContoursInComponent[component]; contour++) {
                if (ce_contourData[component][contour].size() < maxContourSize){
                    maxContourSize = ce_contourData[component][contour].size();
                    masterContour = contour;
                }
            }
            //map the slave contour pts to master pts so that lofting is easier
            for ( contour = 0; contour < ce_numContoursInComponent[component]; contour++) {
                if (contour == masterContour) continue;
                    mapContourToMaster(component,contour,masterContour);
            }
        }
	}
    // ensure that all contours have the correct orientation
    reorientContours();

    //match nearby vertices in contours
    matchContours();

    if(ce_contourExtraction_verbose) writeMessageToParetoLog("------------------------------------------------------------");
    if(ce_contourExtraction_verbose) cout << "Finished generateContourOfMeshComponents " << endl;
    printBasicContourData();
}
/**********************************************************************/
bool  ContourExtraction::generateSolidForComponent(int component, string offFileName)
{
    // create a closed triangulated model of a component
    // model is saved as an .off File
    // see the code for swept and loft solid for details

    if(ce_contourExtraction_verbose) cout << "------------------------------------------------------------\n" << endl;
    if(ce_contourExtraction_verbose) cout << "Component:" << component << endl;
    if (ce_numContoursInComponent[component] == 0 ) {
       if(ce_contourExtraction_verbose) cout << "Component has no contours " << endl;
        return false;
    }
    if (ce_sweepDirection[component] == -1){
        writeMessageToParetoLog("No direction found for component %d ... skipping\n",component);
        return false;
    }
    bool success;

    if (ce_numContoursInComponent[component] == 1)
        success = generateSweptSolidForSingleContour(component,offFileName);
    else
        success = generateLoftSolidForMultipleContours(component, offFileName);

    return success;
}
/**********************************************************************/
bool  ContourExtraction::generateSweptSolidForSingleContour(int component, string offFileName)
{
    // create a closed triangulated model of the swept component assuming it is a single contour.
    // model is saved as an .off File

    // Algorithm
    // 1. Create a 2d mesh with the single contour as boundary (no additional points are introduced)
    // 2. The 2d mesh serves as the top and botton face of swept solid
    // 3. Next, using the boundary points of the contour construct the side faces
    // 4. The resulting solid is saved in .off format


    if(ce_contourExtraction_verbose) writeMessageToParetoLog("Creating swept solid");

    list<point>::iterator iter;
    point pt;
    int nVerticesInContour, nModelVertices,nModelTriangles,nodes[3],direction;
    double componentMinLoc,componentMaxLoc,extents[2];
    int contour = 0;// assume only one contour


    if (ce_contourData[component][contour].size() < 3) {// less than 3 points on the contour
        writeMessageToParetoLog("Component has too few points \n");
        return false;
    }
    direction = ce_sweepDirection[component];

    findMeshExtents( component, extents);
    componentMinLoc = extents[0];
    componentMaxLoc = extents[1];

    if(ce_contourExtraction_verbose){
        cout << "Component " << component << endl;
        cout << "Min: " << componentMinLoc << "; max: " << componentMaxLoc << endl;
    }
    // we can calculate the number of vertices and triangles in the resulting solid
    nVerticesInContour = ce_contourData[component][contour].size();
    nModelVertices = 2*nVerticesInContour;// top and bottom

    // The number of triangles on the side is twice the number of vertices
    // Ex: If there are 4 vertices in a contour, we get 8 triangles on the side (cube)
    nModelTriangles = 2*nVerticesInContour; // side triangles

     // the number of triangles in bottom and top face is the number of vertices minus 2
    nModelTriangles += 2*(nVerticesInContour-2); // top and bottom triangles

    // We will first create a triangulation (mesh) with the contour as boundary
    // Note that no additional vertices are created in the interior of the contour

    using Point = std::array<double, 2>;
    std::vector<std::vector<Point>> polygon(1);// assume only one polygon loop

    Point p;
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;
        switch (direction){
            case 0: p[0] = pt.z; p[1] = pt.y; break;
            case 1: p[0] = pt.z; p[1] = pt.x; break;
            case 2: p[0] = pt.x; p[1] = pt.y; break;
        }
        polygon[0].push_back(p);
    }

    // Run meshing
    // Returns array of indices that refer to the vertices of the input polygon.
    // Three subsequent indices form a triangle.
    // No additional vertices are introduced to create the mesh
    std::vector<uint32_t> triangleIndices = mapbox::earcut<uint32_t>(polygon);

    // now create the off file
    FILE *fptr = fopen(offFileName.c_str(),"w");

    if (!fptr) return false;
    fprintf(fptr,"OFF \n");
    fprintf(fptr,"%d %d 0 \n",nModelVertices,nModelTriangles);

    // save all the points on one face
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;
        switch (direction){
            case 0:  pt.x = componentMaxLoc; break;
            case 1:  pt.y = componentMaxLoc; break;
            case 2:  pt.z = componentMaxLoc; break;
        }
        fprintf(fptr,"%.8g %.8g %.8g \n",pt.x,pt.y,pt.z);
    }

    // save all the points on the other face
    for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
        pt = *iter;
        switch (direction){
        case 0:  pt.x = componentMinLoc; break;
        case 1:  pt.y = componentMinLoc; break;
        case 2:  pt.z = componentMinLoc; break;
        }
        fprintf(fptr,"%.8g %.8g %.8g \n",pt.x,pt.y,pt.z);
    }

    // create triangles on bottom face
    for (int i = 0; i < triangleIndices.size(); i += 3){
        nodes[0] = triangleIndices.at(i);
        nodes[1] = triangleIndices.at(i+1);
        nodes[2] = triangleIndices.at(i+2);
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);
    }

    // create triangles on top face
    for (int i = 0; i < triangleIndices.size(); i += 3){
        nodes[0] = triangleIndices.at(i) + nVerticesInContour;
        nodes[1] = triangleIndices.at(i+1) + nVerticesInContour;
        nodes[2] = triangleIndices.at(i+2) + nVerticesInContour;
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[2],nodes[1]);
    }

    // create triangles on side
    for (int i = 0; i < nVerticesInContour; i++){
        // first triangle
        nodes[0] = i;
        if (i == nVerticesInContour-1)
            nodes[1] = 0;
        else
            nodes[1] = i+1;

        nodes[2] = nodes[1] + nVerticesInContour;
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);

        // second triangle
        nodes[0] = nodes[0];// no change
        nodes[1] = nodes[1] + nVerticesInContour;
        nodes[2] = nodes[0] + nVerticesInContour;
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);

    }

    fclose(fptr);
    return true;
}
/**********************************************************************/
bool  ContourExtraction::generateLoftSolidForMultipleContours(int component, string offFileName)
{
    // create a closed triangulated model of a loft component
    // Assumes that every contour has the same number of points (there is check here)
    // It also assumes that the contour points are geometrically aligned (not checked ... see mapContourToMaster)
    // Model is saved as an .off File

    // Algorithm
    // 1. Check if the number of points in each contour is the same, if not return false
    // 2. Create a 2d mesh with the first contour as boundary (no additional points are introduced)
    // 3. The 2d mesh serves as the top and botton face of loft solid
    // 4. Next, using the boundary points of consecutive contours construct the side faces
    // 5. The resulting solid is saved in .off format


    writeMessageToParetoLog("Creating loft solid");
    list<point>::iterator iter;
    point pt;
    int nVerticesInContour, nModelVertices,nModelTriangles,nodes[3],direction,nContours;

    nContours = ce_numContoursInComponent[component];

    if (nContours <= 1) {
        writeMessageToParetoLog("Component has too few contours for lofting");
        return false;
    }

    nVerticesInContour = ce_contourData[component][0].size();
    if (nVerticesInContour < 3) {// less than 3 points on the contour
        writeMessageToParetoLog("Contour 0 has too few points");
        return false;
    }
    for (int contour = 1; contour < nContours; contour++){
        if (ce_contourData[component][contour].size() != nVerticesInContour) {
            writeMessageToParetoLog("Contour 0 and contour %d have different number of points", contour);
            return false;
        }
    }

    direction = ce_sweepDirection[component];

    writeMessageToParetoLog("#VerticesInContour = %d, nContours = %d",nVerticesInContour,nContours);

    // we calculate the number of vertices and triangles in the resulting loft
    nModelVertices = nContours*nVerticesInContour;// vertices on intermediate contours

    // the number of triangles in bottom and top face is the number of vertices minus 2
    nModelTriangles = 2*(nVerticesInContour-2); // top and bottom triangles

    // For every pair of contours, the number of triangles on each side face is twice the number of vertices
    // Ex: If there are 4 vertices in a contour, we get 8 triangles on the side (cube)
    nModelTriangles += (nContours-1)*2*nVerticesInContour; // side triangles


    // We will first create a triangulation (mesh) with the first contour as boundary
    // Note that no additional vertices are created in the interior of the contour

    using Point = std::array<double, 2>;
    std::vector<std::vector<Point>> polygon(1);// assume only one polygon loop

    Point p;

    for( iter=ce_contourData[component][0].begin(); iter != ce_contourData[component][0].end(); ++iter) {
        pt = *iter;
        switch (direction){
            case 0: p[0] = pt.z; p[1] = pt.y; break;
            case 1: p[0] = pt.z; p[1] = pt.x; break;
            case 2: p[0] = pt.x; p[1] = pt.y; break;
        }
        polygon[0].push_back(p);
    }

    // Run tessellation
    // Returns array of indices that refer to the vertices of the input polygon.
    // Three subsequent indices form a triangle.
    // No additional vertices are introduced to create the mesh
    std::vector<uint32_t> triangleIndices = mapbox::earcut<uint32_t>(polygon);

    // now create the off file
    FILE *fptr = fopen(offFileName.c_str(),"w");

    if (!fptr) return false;
    fprintf(fptr,"OFF \n");
    fprintf(fptr,"%d %d 0 \n",nModelVertices,nModelTriangles);// computed previously

    // save all the points on  first contour to last
    for (int contour = 0; contour < nContours; contour++){
        for( iter=ce_contourData[component][contour].begin(); iter != ce_contourData[component][contour].end(); ++iter) {
            pt = *iter;
            fprintf(fptr,"%.8g %.8g %.8g \n",pt.x,pt.y,pt.z);
        }
    }

    // create triangles on bottom face
    for (int i = 0; i < triangleIndices.size(); i += 3){
        nodes[0] = triangleIndices.at(i);
        nodes[1] = triangleIndices.at(i+1);
        nodes[2] = triangleIndices.at(i+2);
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[2],nodes[1]);
    }

    // create triangles on top face
    int nVerticesOffset;
    nVerticesOffset = (nContours-1)*nVerticesInContour;

    for (int i = 0; i < triangleIndices.size(); i += 3){
        nodes[0] = triangleIndices.at(i) + nVerticesOffset;
        nodes[1] = triangleIndices.at(i+1) + nVerticesOffset;
        nodes[2] = triangleIndices.at(i+2) + nVerticesOffset;
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);
    }

    // create triangles on side
    for (int contour = 0; contour < nContours-1; contour++){
        nVerticesOffset = contour*nVerticesInContour;
        for (int i = 0; i < nVerticesInContour; i++){
            // first triangle
            nodes[0] = i+nVerticesOffset;
            if (i == nVerticesInContour-1)
                nodes[1] = 0+nVerticesOffset;
            else
                nodes[1] = i+1+nVerticesOffset;

            nodes[2] = nodes[1] + nVerticesInContour;
            fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[2],nodes[1]);

            // second triangle
            nodes[0] = nodes[0];// no change
            nodes[1] = nodes[1] + nVerticesInContour;
            nodes[2] = nodes[0] + nVerticesInContour;
            fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[2],nodes[1]);

        }
    }

    fclose(fptr);
    return true;
}
/**********************************************************************/
ContourExtraction::~ContourExtraction(void)
{
    ;
}

