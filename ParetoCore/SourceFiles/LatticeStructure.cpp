#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string> 
#include <fstream> 
#include <iostream> 
#include <unordered_set>
#include <unordered_map>
#ifndef __APPLE__
#include <omp.h>
#endif
#include "Timer.h"
#include "LatticeStructure.h"


using namespace std; 
typedef unsigned char byte; 
#pragma warning(disable:4996)

/**********************************************************************/
LatticeStructure::LatticeStructure() 
{
    ls_latticeType = BOX;
    ls_surfaceRetainType = LATTICE_SURFACE_CRITICAL;
    ls_resolution = 10;// how many voxel divisions per edge of unit cell
    ls_fillRatio = 0.5;
    ls_unitCellSizeInModelUnits = 0.005;//5 mm in default meters (unless otherwise overridden)


}
/************************************************************/
void LatticeStructure::setLatticeType(LATTICE_TYPE latticeType)
{
    ls_latticeType = latticeType;
}
/************************************************************/
void	LatticeStructure::setLatticeSurfaceRetainType(LATTICE_SURFACE_RETAIN latticeSurfaceRetainType)
{
    ls_surfaceRetainType = latticeSurfaceRetainType;
}
/************************************************************/
void	LatticeStructure::setLatticeResolution(int latticeResolution)
{
    ls_resolution = latticeResolution;
}
/************************************************************/
void	LatticeStructure::setLatticeFillRatio(double fillRatio)
{
    ls_fillRatio = fillRatio;
}
/************************************************************/
void	LatticeStructure::setLatticeUnitCellSizeInModelUnits(double unitCellSizeInModelUnits)
{
    ls_unitCellSizeInModelUnits = unitCellSizeInModelUnits;
}
/************************************************************/
void	LatticeStructure::setLatticeBoundingBox(double geometryVolume,double boundingBox[3][2])
{
    ls_geometryVolume = geometryVolume;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            ls_geomBoundingBox[i][j] = boundingBox[i][j];

}
/************************************************************/
void	LatticeStructure::shrinkLatticeBoundingBox(double radius)
{
    double center[3],vec[3],nrm;

    // shrink the bounding box to make sure that the corner and edge trusses are entirely inside the box
    for (int i = 0; i < 3; i++)
        center[i] = (ls_geomBoundingBox[i][0] + ls_geomBoundingBox[i][1])/2;

    // move the min points
    for (int i = 0; i < 3; i++)
        vec[i] = center[i] - ls_geomBoundingBox[i][0] ;
    nrm = NORM3D(vec);
    for (int i = 0; i < 3; i++)
        ls_geomBoundingBox[i][0] += radius*vec[i]/nrm;

    // move the max points
    for (int i = 0; i < 3; i++)
        vec[i] = center[i] - ls_geomBoundingBox[i][1] ;//max points
    nrm = NORM3D(vec);
    for (int i = 0; i < 3; i++)
        ls_geomBoundingBox[i][1] += radius*vec[i]/nrm;

}
/************************************************************/
void LatticeStructure::constructBoxLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;


    //nNodes = 8;
    //nTrussElements = 12;

    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    edge[0] = 0; edge[1] = 1;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 2;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 3;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 0;
    trussStructure.trussElements.push_back(edge);

    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

    edge[0] = 4; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 5; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 6; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 7; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
}
/************************************************************/
void LatticeStructure::constructBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 9;
    //nTrussElements = 8;

    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    pt[0] = Lx/2;pt[1] = Ly/2;pt[2] = Ly/2;
    trussStructure.nodes.push_back(pt);

    for (int i = 0; i < 8; i++){
        edge[0] = i; edge[1] = 8;
        trussStructure.trussElements.push_back(edge);
    }
}
/************************************************************/
void LatticeStructure::constructFCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 8 ;
    //nTrussElements = 12;

    // 8 corners
    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    //left face
    edge[0] = 0; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);

    //front face
    edge[0] = 1; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);

    //right face
    edge[0] = 3; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

    //back face
    edge[0] = 0; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);


    //bottom face
    edge[0] = 0; edge[1] = 2;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 3;
    trussStructure.trussElements.push_back(edge);

    //top face
    edge[0] = 4; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 5; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

}
/************************************************************/
void LatticeStructure::constructFCCzLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 8 ;
    //nTrussElements = 8;

    // 8 corners
    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    //left face
    edge[0] = 0; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);

    //front face
    edge[0] = 1; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);

    //right face
    edge[0] = 3; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

    //back face
    edge[0] = 0; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);

    //bottom face
    edge[0] = 0; edge[1] = 2;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 3;
    trussStructure.trussElements.push_back(edge);

    //top face
    edge[0] = 4; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 5; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

    //four straight
    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

}
/************************************************************/
void LatticeStructure::constructFCCBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 8 ;
    //nTrussElements = 12;

    // 8 corners
    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    //left face
    edge[0] = 0; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);

    //front face
    edge[0] = 1; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);

    //right face
    edge[0] = 3; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);

    //back face
    edge[0] = 0; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);


    // four cross diagonals
    edge[0] = 0; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);

}
/************************************************************/
void LatticeStructure::constructBCCzLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 9;
    //nTrussElements = 8 + 4;

    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    pt[0] = Lx/2;pt[1] = Ly/2;pt[2] = Ly/2;
    trussStructure.nodes.push_back(pt);

    for (int i = 0; i < 8; i++){
        edge[0] = i; edge[1] = 8;
        trussStructure.trussElements.push_back(edge);
    }

    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
}
/************************************************************/
void LatticeStructure::constructBoxBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 9;
    //nTrussElements = 12 + 8;

    // 8 corners
    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    pt[0] = Lx/2;pt[1] = Ly/2;pt[2] = Ly/2;
    trussStructure.nodes.push_back(pt);

    edge[0] = 0; edge[1] = 1;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 2;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 3;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 0;
    trussStructure.trussElements.push_back(edge);

    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 4; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 5; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 6; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 7; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);

    for (int i = 0; i < 8; i++){
        edge[0] = i; edge[1] = 8;
        trussStructure.trussElements.push_back(edge);
    }
}
/************************************************************/
void LatticeStructure::constructOctetLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;

    //nNodes = 8 + 6;
    //nTrussElements = 4*6 + 12;

    // 8 corners
    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);


    // 6 face centers
    pt[0] = Lx/2;pt[1] = Ly/2;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx/2;pt[1] = 0;pt[2] = Lz/2;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly/2;pt[2] = Lz/2;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx/2;pt[1] = Ly;pt[2] = Lz/2;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly/2;pt[2] = Lz/2;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx/2;pt[1] = Ly/2;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    int nodes[4];
    nodes[0] = 0; nodes[1] = 1; nodes[2] = 2; nodes[3] = 3;
    for (int i = 0; i < 4; i++){//bottom face
        edge[0] = nodes[i]; edge[1] = 8;
        trussStructure.trussElements.push_back(edge);
    }
    nodes[0] = 0; nodes[1] = 1; nodes[2] = 4; nodes[3] = 5;
    for (int i = 0; i < 4; i++){//left face
        edge[0] = nodes[i]; edge[1] = 9;
        trussStructure.trussElements.push_back(edge);
    }
    nodes[0] = 1; nodes[1] = 2; nodes[2] = 6; nodes[3] = 5;
    for (int i = 0; i < 4; i++){//front face
        edge[0] = nodes[i]; edge[1] = 10;
        trussStructure.trussElements.push_back(edge);
    }
    nodes[0] = 2; nodes[1] = 3; nodes[2] = 6; nodes[3] = 7;
    for (int i = 0; i < 4; i++){//right face
        edge[0] = nodes[i]; edge[1] = 11;
        trussStructure.trussElements.push_back(edge);
    }
    nodes[0] = 0; nodes[1] = 3; nodes[2] = 4; nodes[3] = 7;
    for (int i = 0; i < 4; i++){//back face
        edge[0] = nodes[i]; edge[1] = 12;
        trussStructure.trussElements.push_back(edge);
    }
    nodes[0] = 4; nodes[1] = 5; nodes[2] = 6; nodes[3] = 7;
    for (int i = 0; i < 4; i++){//top face
        edge[0] = nodes[i]; edge[1] = 13;
        trussStructure.trussElements.push_back(edge);
    }

    // center trusses
    nodes[0] = 9; nodes[1] = 10; nodes[2] = 11; nodes[3] = 12;
    for (int i = 0; i < 4; i++){//top face
        edge[0] = nodes[i]; edge[1] = 8;
        trussStructure.trussElements.push_back(edge);
    }

    nodes[0] = 9; nodes[1] = 10; nodes[2] = 11; nodes[3] = 12;
    for (int i = 0; i < 4; i++){//top face
        edge[0] = nodes[i]; edge[1] = 13;
        trussStructure.trussElements.push_back(edge);
    }

    edge[0] = 9; edge[1] = 10;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 10; edge[1] = 11;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 11; edge[1] = 12;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 12; edge[1] = 9;
    trussStructure.trussElements.push_back(edge);
}
/************************************************************/
void LatticeStructure::constructC11LatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;


    //nNodes = 8;
    //nTrussElements = 4;

    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
}
/************************************************************/
void LatticeStructure::constructC12LatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    Point3D pt;
    LineSegment edge;


    //nNodes = 8;
    //nTrussElements = 4;

    pt[0] = 0;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = 0;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = 0;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = Lx;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);
    pt[0] = 0;pt[1] = Ly;pt[2] = Lz;
    trussStructure.nodes.push_back(pt);

    edge[0] = 0; edge[1] = 4;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 1; edge[1] = 5;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 2; edge[1] = 6;
    trussStructure.trussElements.push_back(edge);
    edge[0] = 3; edge[1] = 7;
    trussStructure.trussElements.push_back(edge);
}
/************************************************************/
void LatticeStructure::constructLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz)
{
    double cellLength = ls_unitCellSizeInModelUnits;

    if (ls_latticeType == BOX) {
        constructBoxLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == BCC){
        constructBCCLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == BCCz){
        constructBCCzLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == BOX_BCC) {
        constructBoxBCCLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == FCC) {
        constructFCCLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == FCCBCC) {
        constructFCCBCCLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == FCCz) {
        constructFCCzLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == OCTET){
        constructOctetLatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == C11){
        constructC11LatticeTruss(trussStructure,Lx,Ly,Lz);
    }
    else if (ls_latticeType == C12){
        constructC11LatticeTruss(trussStructure,Lx,Ly,Lz);
    }
}
/************************************************************/
void LatticeStructure::printTrussStructure( TrussStructure &trussStructure)
{
    Point3D pt;
    LineSegment edge;

    for (int truss = 0; truss < trussStructure.trussElements.size(); truss++) {
        edge = trussStructure.trussElements[truss];
        printf("%d: %d %d \n",truss,edge[0],edge[1]);
    }

    for (int node = 0; node < trussStructure.nodes.size(); node++) {
        pt = trussStructure.nodes[node];
        printf("%d: %.2g %.2g %.2g \n",node,pt[0],pt[1],pt[2]);
    }

}
/************************************************************/
void LatticeStructure::shrinkTrussStructure( TrussStructure &trussStructure,const double unitCellSize, const double radius)
{
    Point3D pt;
    double center[3],vec[3],nrm;

    center[0] = center[1] = center[2] = unitCellSize/2;

    for (int node = 0; node < trussStructure.nodes.size(); node++) {
        pt = trussStructure.nodes[node];
        for (int i = 0; i < 3; i++)
            vec[i] = center[i] - pt[i];
        nrm = NORM3D(vec);
        for (int i = 0; i < 3; i++)
            trussStructure.nodes[node][i] += radius*vec[i]/nrm;
    }
}
/************************************************************/
int		LatticeStructure::getLatticeNumberOfVoxelElementsNeeded(void)
{
    int numberOfLatticeVoxElems,numberOfLattices;
    double unitCellVolume;

    unitCellVolume = pow(ls_unitCellSizeInModelUnits,3.0);
    numberOfLattices = NEARESTINT(ls_geometryVolume/unitCellVolume);
    numberOfLatticeVoxElems = pow(ls_resolution,3)*numberOfLattices;// this is for fine resolution

    return numberOfLatticeVoxElems;
}
/************************************************************/
double LatticeStructure::getRadiusOfTrussElement(double cubeLength)
{
    double L,latticeVolume,LCubeHalfDiag,LFaceDiag,radius;

    L =cubeLength;
    latticeVolume = pow(L,3.0);
    LCubeHalfDiag = sqrt(3*L*L/4);// half the distance from corner to center
    LFaceDiag = sqrt(2)*L;// diag of a face

    radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(12*L))); // default
    // based on the fill ratio and lattice type, figure out the radius of truss
    switch (ls_latticeType) {
    case BOX:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(12*L)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.45;// heuristic
        break;
    case BCC:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*8*LCubeHalfDiag));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.20;// heuristic
        break;
    case BCCz:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(8*LCubeHalfDiag+4*L)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.25;// heuristic
        break;
    case BOX_BCC:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(12*L + 8*LCubeHalfDiag)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.3;// heuristic
        break;
    case FCC:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(8*LFaceDiag)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.25;// heuristic
        break;
    case FCCBCC:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(8*LFaceDiag + 8*LCubeHalfDiag)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.25;// heuristic
        break;
    case FCCz:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(8*LFaceDiag  + 4*L)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.3;// heuristic
        break;
    case OCTET:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(12*LFaceDiag+4*LFaceDiag)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.25;// heuristic
        break;
    case C11:
        radius = sqrt(ls_fillRatio*latticeVolume/(3.1415*(4*L)));
        // the radius estimate above is smaller than needed since the length L is an overestimate
        radius *= 1.20;// heuristic
        break;
    }

    return radius;
}
/************************************************************/
void LatticeStructure::getLatticeSpecsAsString(char *latticeSpecs)
{
    char tempStr[256];
    int Nx,Ny,Nz,numberOfLatticeVoxElems,numberOfLattices;
    double unitCellVolume,radius;

    Nx = int NEARESTINT( (ls_geomBoundingBox[0][1] - ls_geomBoundingBox[0][0])/ls_unitCellSizeInModelUnits) ;
    Ny = int NEARESTINT( (ls_geomBoundingBox[1][1] - ls_geomBoundingBox[1][0])/ls_unitCellSizeInModelUnits) ;
    Nz = int NEARESTINT( (ls_geomBoundingBox[2][1] - ls_geomBoundingBox[2][0])/ls_unitCellSizeInModelUnits) ;


    unitCellVolume = pow(ls_unitCellSizeInModelUnits,3.0);
    numberOfLattices = NEARESTINT(ls_geometryVolume/unitCellVolume);
    numberOfLatticeVoxElems = pow(ls_resolution,3)*numberOfLattices;// how many voxel elements per lattice

    switch (ls_latticeType){
    case BOX: sprintf(latticeSpecs,"Lattice type: Box \n");break;
    case BCC: sprintf(latticeSpecs,"Lattice type: BCC \n");break;
    case BCCz: sprintf(latticeSpecs,"Lattice type: BCCz \n");break;
    case BOX_BCC: sprintf(latticeSpecs,"Lattice type: Box-BCC \n");break;
    case FCC: sprintf(latticeSpecs,"Lattice type: FCC \n");break;
    case FCCz: sprintf(latticeSpecs,"Lattice type: FCCz \n");break;
    case FCCBCC: sprintf(latticeSpecs,"Lattice type: FCCBCC \n");break;
    case OCTET: sprintf(latticeSpecs,"Lattice type: OCTET \n");break;
    case C11: sprintf(latticeSpecs,"Lattice type: C11 \n");break;
    case C12: sprintf(latticeSpecs,"Lattice type: C12\n");break;
    case C44: sprintf(latticeSpecs,"Lattice type: C44 \n");break;
    default: sprintf(latticeSpecs,"Lattice type: Unknown \n");break;
    }

    sprintf(tempStr,"Cell size (modelUnits): %.2g \n",ls_unitCellSizeInModelUnits);
    strcat(latticeSpecs,tempStr);
    sprintf(tempStr,"Cell edge division: %d \n",ls_resolution);
    strcat(latticeSpecs,tempStr);
    radius = getRadiusOfTrussElement(ls_unitCellSizeInModelUnits);
    if (radius < ls_unitCellSizeInModelUnits/ls_resolution){
        sprintf(tempStr,"**Insufficient edge division for fill ratio!\n**Suggested division: %d\n",NEARESTINT(ls_unitCellSizeInModelUnits/radius)+1);
        strcat(latticeSpecs,tempStr);
    }
    sprintf(tempStr,"Desired fill ratio: %.2f \n",ls_fillRatio);
    strcat(latticeSpecs,tempStr);

    switch (ls_surfaceRetainType){
    case LATTICE_SURFACE_NONE: sprintf(tempStr,"Surfaces retained: None \n");break;
    case LATTICE_SURFACE_CRITICAL: sprintf(tempStr,"Surfaces retained: Critical \n");break;
    case LATTICE_SURFACE_ALL: sprintf(tempStr,"Surfaces retained: All \n");break;
    }
    strcat(latticeSpecs,tempStr);
    sprintf(tempStr,"#Unit Cells: %d \n",Nx*Ny*Nz);
    strcat(latticeSpecs,tempStr);
    sprintf(tempStr,"#VoxelElements: %d \n",numberOfLatticeVoxElems);
    strcat(latticeSpecs,tempStr);

    strcat(latticeSpecs,"\nProceed?");
}


#ifdef _WIN32

//*********************************************
void LatticeStructure:: RemoveDuplicatePointsFromIVBool(ivbool3d::Object *master, ivbool3d::Object *slave)
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

// https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys
template <int d,class T_int>
struct ArrayUtilities {
    // hash
    unsigned operator()(const std::array<T_int, d>& a) const {
        T_int h = 0;
        for (auto e : a) {
            h ^= std::hash<T_int>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2);}
        return h;}
    // equal to
    bool operator()(const std::array<T_int, d>& a, const std::array<T_int, d>& b) const {
        for (int i=0;i<d;i++) {
            if (a[i]!=b[i]) return false;
        }
        return true;
    }
};


double Distance_To_Line(const std::array<unsigned,2>& edge, const ivbool3d::Point3d *points, const std::array<unsigned,3>& triangle_indices) {
    // find the nonedge vertex - x[0] // edge[0] - x[1] // edge[1] - x[2]
    int indices[3];
    for (int i=1;i<3;i++) {
        auto index=std::find(triangle_indices.begin(), triangle_indices.end(), edge[i-1]);
        if (index != triangle_indices.end()) {
            indices[i]=index-triangle_indices.begin();
        }
        else {
            std::cout<<"ERROR ::: Triangle doesn't contain vertex " <<edge[i]<<std::endl;
            exit(1);}
    }
    indices[0]=3-indices[1]-indices[2];
    // coordinates
    std::array<Point3D,3> x;
    for (int i=0;i<3;i++) {
        x[i][0]=points[triangle_indices[indices[i]]].x;
        x[i][1]=points[triangle_indices[indices[i]]].y;
        x[i][2]=points[triangle_indices[indices[i]]].z;}
    // math http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    Point3D x1_x0, x2_x1;
    for (int i=0;i<3;i++) {
        x1_x0[i]=x[1][i]-x[0][i];
        x2_x1[i]=x[2][i]-x[1][i];}
    double length_x1_x0 = x1_x0[0]*x1_x0[0]+x1_x0[1]*x1_x0[1]+x1_x0[2]*x1_x0[2];
    double length_x2_x1 = x2_x1[0]*x2_x1[0]+x2_x1[1]*x2_x1[1]+x2_x1[2]*x2_x1[2];
    double x1x0_x2x1_dot_product = x1_x0[0]*x2_x1[0]+x1_x0[1]*x2_x1[1]+x1_x0[2]*x2_x1[2];
    return (length_x1_x0*length_x2_x1 - x1x0_x2x1_dot_product*x1x0_x2x1_dot_product) / length_x2_x1;
}


/*
void LatticeStructure::FilterIVBoolMesh(ivbool3d::Object* mesh, ivbool3d::Object* obj)
{
    std::cout<<"Filtering Mesh"<<std::endl;

    // get ivbool data
    std::cout<<"Collecting ivbool data"<<std::endl;
    unsigned int numPoints;
    const ivbool3d::Point3d *points;
    if(!mesh->GetPointsD(&points, numPoints)) std::cout<<"Failed to get mesh points"<<std::endl;
    unsigned int numFaces=mesh->GetNumFaces();
    std::vector<std::array<unsigned,3> >triangles(numFaces);
    ivbool3d::Face face;
    for (int i=0;i<numFaces;i++) {
        if(!mesh->GetFace(i,&face)) std::cout<<"Failed to get face "<<i<<std::endl;
        for (int j=0;j<3;j++) {
            triangles[i][j]=face.v[j];}}

    // remove duplicate triangles - keep one copy
    std::cout<<"Removing duplicate triangles"<<std::endl;
    std::unordered_map<array<unsigned,3>,vector<unsigned>, ArrayUtilities<3> > triangle_map(triangles.size());
    for (unsigned i=0;i<triangles.size();i++) {
        array<unsigned,3> face=triangles[i]; // make a copy
        sort(face.begin(),face.end());
        triangle_map[face].push_back(i);}
    std::vector<unsigned> deletion_list;
    for (auto e : triangle_map) {
        if (e.second.size()>1) {
            for (int i=1;i<e.second.size();i++) {
                std::cout<<"    duplicate triangle found : "<<e.second[i]<<std::endl;
                deletion_list.push_back(e.second[i]);}}}
    // delete triangles
    sort(deletion_list.begin(), deletion_list.end());
    std::cout<<"  sorted deletion_list ";for (auto x : deletion_list) std::cout<<x<<" "; std::cout<<std::endl;
    while (deletion_list.size()) {
        auto index=deletion_list.back();
        std::cout<<"    Removing duplicate triangle "<<index<<std::endl;
        triangles[index]=triangles.back();
        triangles.pop_back();
        while (deletion_list.back()==index) {
            deletion_list.pop_back();}}

    // deal with nonmanifold edges
    std::unordered_map<array<unsigned,2>,std::vector<unsigned>, ArrayUtilities<2>, ArrayUtilities<2> > edge_to_faces;
    std::unordered_map<int, std::array<array<unsigned,2>,3> > face_to_edges;
    bool do_nonmanifold_filter=true;
    while (do_nonmanifold_filter) {
        do_nonmanifold_filter=false;
        bool nonmanifold_edge_found=false;

        // when remove dangling triangles from nonmanifold edges
        std::cout<<"Removing dangling triangles from nonmanifold edges"<<std::endl;
        // fill data structures
        deletion_list.clear();
        edge_to_faces.clear(); edge_to_faces.reserve(triangles.size()*4);
        face_to_edges.clear(); face_to_edges.reserve(triangles.size());
        for (int i=0;i<triangles.size();i++) {
            for (int j=0;j<3;j++) {
                std::array<unsigned,2> edge{triangles[i][j],triangles[i][(j+1)%3]};
                edge_to_faces[edge].push_back(i);
                face_to_edges[i][j]=edge;}}
        // examine edges
        for (auto edge : edge_to_faces) {
            if (edge.second.size() > 1) { // nonmanifold directed edge
                std::cout<<" dangling triangle test : triangles on nonmanifold edge "; for (auto t : edge.second) std::cout<<" "<<t; std::cout<<endl;
                nonmanifold_edge_found=true;
                for (auto face : edge.second) { // examine all faces using that edge
                    for (auto face_edge : face_to_edges[face]) { // check to see if any edges are boundary edges
                        std::array<unsigned,2> reversed_face_edge={face_edge[1],face_edge[0]};
                        if (edge_to_faces.count(reversed_face_edge)==0) {
                            deletion_list.push_back(face);}}}}}
        // delete triangles
        sort(deletion_list.begin(), deletion_list.end());
        std::cout<<"  sorted deletion_list ";for (auto x : deletion_list) std::cout<<x<<" "; std::cout<<std::endl;
        while (deletion_list.size()) {
            auto index=deletion_list.back();
            std::cout<<"    Removing triangle "<<index<<std::endl;
            triangles[index]=triangles.back();
            triangles.pop_back();
            while (deletion_list.back()==index) {
                deletion_list.pop_back();}}
        // end removing dangling triangles

        if (nonmanifold_edge_found) {
            // keep the largest triangle if more than one triangle uses an edge
            // this will likely leave a dangling triangle so repeat removing dangling triangles if one is found
            std::cout<<"Removing small triangles from nonmanifold edges"<<std::endl;
            deletion_list.clear();
            // triangles were renumbered if any were deleted so rebuild the data structures
            edge_to_faces.clear(); edge_to_faces.reserve(triangles.size()*4);
            face_to_edges.clear(); face_to_edges.reserve(triangles.size());
            for (int i=0;i<triangles.size();i++) {
                for (int j=0;j<3;j++) {
                    std::array<unsigned,2> edge{triangles[i][j],triangles[i][(j+1)%3]};
                    edge_to_faces[edge].push_back(i);
                    face_to_edges[i][j]=edge;}}
            for (auto edge : edge_to_faces) {
                if (edge.second.size() > 1) { // nonmanifold edge
                    do_nonmanifold_filter=true;
                    std::cout<<" Removing small triangles : triangles on nonmanifold edge "; for (auto t : edge.second) std::cout<<" "<<t; std::cout<<endl;
                    unsigned largest_triangle=0;
                    double largest_distance=Distance_To_Line(edge.first, points, triangles[edge.second[0]]);
                    for (int i=1;i<edge.second.size();i++) {
                        double candidate_distance=Distance_To_Line(edge.first, points, triangles[edge.second[i]]);
                        if (candidate_distance>largest_distance) {
                            largest_distance=candidate_distance;
                            largest_triangle=i;}}
                    for (int i=0;i<edge.second.size();i++) {
                        if (i!=largest_triangle) {deletion_list.push_back(edge.second[i]);}}
                    break; // just correct one edge - this will leave dangling triangles
                }}
            // delete triangles
            sort(deletion_list.begin(), deletion_list.end());
            std::cout<<"  sorted deletion_list ";for (auto x : deletion_list) std::cout<<x<<" "; std::cout<<std::endl;
            while (deletion_list.size()) {
                auto index=deletion_list.back();
                std::cout<<"    Removing triangle "<<index<<std::endl;
                triangles[index]=triangles.back();
                triangles.pop_back();
                while (deletion_list.back()==index) {
                    deletion_list.pop_back();}}
        } // end removing small triangles from nonmanifold edges
    } // end addressing nonmanifold edges


    // count boundary edges
    std::cout<<"counting boundary triangles"<<std::endl;
    // fill data structures
    edge_to_faces.clear(); edge_to_faces.reserve(triangles.size()*4);
    std::vector<unsigned> boundary_triangles;
    for (int i=0;i<triangles.size();i++) {
        for (int j=0;j<3;j++) {
            std::array<unsigned,2> edge{triangles[i][j],triangles[i][(j+1)%3]};
            edge_to_faces[edge].push_back(i);
        }}
    // examine edges
    for (auto edge : edge_to_faces) {
        std::array<unsigned,2> reversed_edge={edge.first[1],edge.first[0]};
        if (edge_to_faces.count(reversed_edge)==0) {
            boundary_triangles.push_back(edge.second[0]);
        }}
    sort(boundary_triangles.begin(),boundary_triangles.end());
    std::cout<<"Boundary_Triangles ";for (auto e:boundary_triangles) std::cout<<" "<<e; std::cout<<std::endl;

    // regenerate ivbool::mesh object
    std::cout<<"regenerating the ivbool mesh object"<<std::endl;
    numFaces=triangles.size();
    unsigned *faces = (unsigned *)malloc(numFaces * 3 * sizeof(unsigned));
    for (int i=0;i<numFaces;i++) {
        faces[3 * i]     = triangles[i][0];
        faces[3 * i + 1] = triangles[i][1];
        faces[3 * i + 2] = triangles[i][2];}
    obj->SetNumFaces(numFaces);
    unsigned size = 3;
    for (unsigned i = 0; i<numFaces; i++){
        obj->NewFace(size, faces);
        faces += size;
    }
    obj->SetPointsD((ivbool3d::Point3d*)points, numPoints);
    mesh=obj;
    std::cout<<"End Filtering"<<std::endl;
}
*/
/************************************************************/
bool LatticeStructure::computeLatticeUnitCellAsOffFile(void)
{
    double radius;
    int Nx,Ny,Nz;
    double Lx,Ly,Lz,L;
    int nDivisions;
    int node0,node1;
    LineSegment edge;
    Point3D pt0,pt1;
    TrussStructure trussStructure;

    // There is a bug in MacOS for OpenMP

    InitializeTopOpt(3);
    computeLatticeStructure();
    L = ls_unitCellSizeInModelUnits;
    radius = 0.5*getRadiusOfTrussElement(ls_unitCellSizeInModelUnits);
    // the radius cannot be smaller than the voxel size
    radius = MAX(radius,L/ls_resolution);

    // shrink the boundingBox by the radius value to make sure the corners are captured correctly
    shrinkLatticeBoundingBox(radius);

    Nx = NEARESTINT( (ls_geomBoundingBox[0][1] - ls_geomBoundingBox[0][0])/L) ;
    Ny = NEARESTINT( (ls_geomBoundingBox[1][1] - ls_geomBoundingBox[1][0])/L) ;
    Nz = NEARESTINT( (ls_geomBoundingBox[2][1] - ls_geomBoundingBox[2][0])/L) ;

    Lx = (ls_geomBoundingBox[0][1] - ls_geomBoundingBox[0][0])/Nx;
    Ly = (ls_geomBoundingBox[1][1] - ls_geomBoundingBox[1][0])/Ny;
    Lz = (ls_geomBoundingBox[2][1] - ls_geomBoundingBox[2][0])/Nz;

    cout << "Constructing lattice cell " << endl;
    //get the template unit cell
    constructLatticeTruss(trussStructure,Lx,Ly,Lz);


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
            cout << "LatticeStructure: Could not initialize ivmodule.dll" << endl;
            return false;
        }
    }
    else{
        cout << "LatticeStructure: Could not load ivmodule.dll" << endl;
        return false;
    }
    double scalingForBoolean;

    scalingForBoolean = 1.0/Lx;//scaling to 1 helps in Boolean

    cout << "scalingForBoolean " << scalingForBoolean << endl;

    //scalingForBoolean = 1.0;
    ivbool3d::Processor*proc = iv3dCreate();
    ivbool3d::Object *lattice = proc->AddSource(0);//A channel
    ivbool3d::Object *trussMember = proc->AddSource(1);//B channel
    proc->SetReduceProps(0.01, 0.01, 0.01);//default settings
    ivbool3d::boolean_mode booleanOperation = ivbool3d::M_OR;
    nDivisions = 4;
    char fileName[256];
    double offset[3];
    offset[0] = ls_geomBoundingBox[0][0];
    //now mark all elements within a given radius to the truss elements as 'in'
    bool firstStrut;
    firstStrut = true;
    for (int ix = 0; ix <= 1; ix++) {//Nx
        cout << 100.0*ix/Nx << endl;
        offset[1] = ls_geomBoundingBox[1][0];
        for (int jy = 0; jy <= 1; jy++) {
            offset[2] = ls_geomBoundingBox[2][0];
            for (int kz = 0; kz <= 1; kz++) {
                for (int truss = 0; truss < trussStructure.trussElements.size(); truss++) {//
                    edge = trussStructure.trussElements[truss];
                    node0 = edge[0];
                    node1 = edge[1];
                    pt0 = trussStructure.nodes[node0];
                    pt1 = trussStructure.nodes[node1];

                    // shift lattice
                    for (int i = 0; i < 3; i++) {
                        pt0[i] = pt0[i] + offset[i];
                        pt1[i] = pt1[i] + offset[i];
                    }
                    sprintf(fileName,"truss_%d.off",truss);
                    if (firstStrut){
                        constructCylinderTriangulation(pt0,pt1,radius,scalingForBoolean,nDivisions,lattice);
                        SaveIVBoolToOffFile(lattice, 1.0,"lattice.off");
                        SaveIVBoolToOffFile(lattice, 1.0,fileName);
                        firstStrut = false;
                    }
                    else {
                        proc->Destroy();
                        proc = iv3dCreate();
                        proc->SetReduceProps(1e-6, 0.1, 1e-6);//default settings
                        lattice = proc->AddSource(0);
                        readAndLoadOffFileToIVBool("lattice.off",lattice);
                        trussMember = proc->AddSource(1);
                        if (!constructCylinderTriangulation(pt0,pt1,radius,scalingForBoolean,nDivisions,trussMember))
                            continue;

                        SaveIVBoolToOffFile(trussMember,1.0, fileName);
                        if (!proc->Process(booleanOperation, ivbool3d::REDUCE)) {
                            cout << "LatticeStructure: Could not perform Boolean " << endl;
                        }
                        //ivbool3d::Object *filtered_result=proc->NewObject();
                        //FilterIVBoolMesh(proc->GetResult(), filtered_result);
                        //SaveIVBoolToOffFile(filtered_result,1.0, "lattice.off");
                        SaveIVBoolToOffFile(proc->GetResult(),1.0, "lattice.off");
                    }
                }
                offset[2] += Lz;
            }
            offset[1] += Ly;
        }
        offset[0] += Lx;
    }
    SaveIVBoolToOffFile(proc->GetResult(), 1./scalingForBoolean,"finallattice.off");

    proc->Destroy();
    iv3dDone();

#endif
}
//*********************************************
bool LatticeStructure::readAndLoadOffFileToIVBool(const char *fileName, ivbool3d::Object*obj)
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
        cout << "LatticeStructure: Could not find " << fileName << endl;
        return false;
    }

    if (find_word(filein, "OFF") == 0) {
        cout << "LatticeStructure:  OFF keyword missing" << endl;
        return false;
    }

    fscanf(filein, "%d %d %d", &(nTriPoints), &(nTriangles), &(tempInt));
    if ((nTriPoints == 0) || (nTriangles == 0)) {
        cout << "  Incorrect OFF format " << endl;
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
            cout << "LatticeStructure: face in OFF file has more than 3 vertices ... not allowed " << endl;
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


    return true;
}
//*********************************************
bool  LatticeStructure::SaveLatticeGraphToOffFile(const char*offFile)
{
    FILE *fptr = fopen(offFile, "wt");
    if (fptr== NULL){
        writeMessageToParetoLog("Unable to open file %s ",offFile);
        return false;
    }

    int number_of_trusses = ls_TrussStructure.trussElements.size();
    int number_of_nodes = ls_TrussStructure.nodes.size();


    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d 0 %d\n",number_of_nodes,number_of_trusses);

    if (number_of_trusses == 0){
        fclose(fptr);
        return false;

    }
    for (int node = 0;node<number_of_nodes;node++)
        fprintf(fptr,"%.3g %.3g %.3g \n",ls_TrussStructure.nodes[node][0],ls_TrussStructure.nodes[node][1],ls_TrussStructure.nodes[node][2]);

    for (int truss = 0;truss<number_of_trusses;truss++)
        fprintf(fptr,"%d %d \n",ls_TrussStructure.trussElements[truss][0],ls_TrussStructure.trussElements[truss][1]);

    fclose(fptr);
}

//*********************************************
bool  LatticeStructure::SaveIVBoolToOffFile(ivbool3d::Object*obj,double scale, const char*offFile)
{
    // scaling is to help boolean
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
                fprintf(f, "%g %g %g\n", scale*(p32->x), scale*(p32->y), scale*(p32->z));
                p32++;
            }
        }
        else if (p64) {
            for (i = 0; i<nVertices; i++) {
                fprintf(f, "%e %e %e\n", scale*(p64->x), scale*(p64->y), scale*(p64->z));
                p64++;
            }
        }
        ivbool3d::Face face;
        for (i = 0; i<nTriangles; i++) {
            obj->GetFace(i, &face);
            fprintf(f, "3 %d %d %d \n", face.v[0],face.v[1],face.v[2]);
        }

        fclose(f);

        return true;
    }
    return false;
}
/************************************************************/
bool    LatticeStructure::constructCylinderTriangulation(Point3D startPt, Point3D endPt, double radius, double scale,int nDivisions, ivbool3d::Object*obj)
{
    // construct a triangulation of cylinder
    // points are constructed at the center of the two discs, and on the circumference
    // triangles are then constructed from these points
    // scaling is to help boolean

    int nTriPoints,nTriangles,face;
    double axis[3],zVec[3],lengthOfAxis,theta;

    nTriPoints = 2*nDivisions + 2;//
    nTriangles = 2*nDivisions +2*nDivisions; // two discs and cylinder

    point3dd *vertices;
    vertices = (point3dd *)malloc(nTriPoints * sizeof(point3dd));

    // first create the cylinder with axis along z
    for (int i = 0; i < 3; i++)
        axis[i] = endPt[i] - startPt[i];


    lengthOfAxis = NORM3D(axis);
    if (lengthOfAxis < 1e-32)
        return false;
    for (int i = 0; i < 3; i++)
        axis[i] /= lengthOfAxis;


    // startPt
    // to avoid numerical issues, we need to perturb the center vertex randomly
    vertices[0].x =0;// 0.1*radius*rand()/RAND_MAX;// avoids numerical errors during Booleans
    vertices[0].y =0;// 0.1*radius*rand()/RAND_MAX;
    vertices[0].z = -radius;
    for (int node = 1; node <= nDivisions; node++) {// points on start disc circumference
        theta = (node-1)*2*3.1415/nDivisions;//  + 0.01*rand()/RAND_MAX;// avoids numerical errors during Booleans
        vertices[node].x = radius*cos(theta);
        vertices[node].y = radius*sin(theta);
        vertices[node].z = 0;
    }

    vertices[nDivisions+1].x = 0;// 0.1*radius*rand()/RAND_MAX;
    vertices[nDivisions+1].y = 0; //0.1*radius*rand()/RAND_MAX;
    vertices[nDivisions+1].z = lengthOfAxis +radius;
    for (int node = nDivisions+2; node <= 2*nDivisions+1; node++) {// points on end disc circumference
        theta = (node-nDivisions-2)*2*3.1415/nDivisions;// + 0.01*rand()/RAND_MAX;// avoids numerical errors during Booleans
        vertices[node].x = radius*cos(theta);
        vertices[node].y = radius*sin(theta);
        vertices[node].z = lengthOfAxis;
    }


    obj->SetNumFaces(nTriangles);
    unsigned *faces = (unsigned *)malloc(nTriangles * 3 * sizeof(unsigned));

    int startNode = 1;
    //faces on the bottom
    face = 0;
    while (face < nDivisions) {// bottom faces
        faces[3 * face] = (unsigned) 0;
        if (face <nDivisions-1)
            faces[3 * face + 1] = (unsigned) startNode+1;//anti-clockwise for outward normal
        else //last face
            faces[3 * face + 1] = (unsigned)1;

        faces[3 * face + 2] = (unsigned) startNode;

        face++;
        startNode++;

    }

    //faces on the side

    startNode = 1;
    face = nDivisions;
    while (face < 3*nDivisions) {// side faces
        if (startNode < nDivisions){
            faces[3 * face] = (unsigned) startNode;
            faces[3 * face + 1] = (unsigned) startNode + 1;
            faces[3 * face + 2] = (unsigned) startNode+nDivisions+1;
            face++;

            faces[3 * face] = (unsigned) startNode+1;
            faces[3 * face + 1] = (unsigned) startNode + nDivisions + 2;
            faces[3 * face + 2] = (unsigned) startNode+nDivisions+1;
            face++;
        }
        else {//last face
            faces[3 * face] = (unsigned) startNode;// at this point startNode = nDivisions;
            faces[3 * face + 1] = (unsigned) 1;
            faces[3 * face + 2] = (unsigned) startNode+nDivisions+1;
            face++;

            faces[3 * face] = (unsigned) 1;
            faces[3 * face + 1] = (unsigned)  startNode + 2;
            faces[3 * face + 2] = (unsigned) startNode+nDivisions+1;
            face++;
        }
        startNode++;
    }

    //faces on the top
    face = 3*nDivisions;
    startNode = nDivisions+2;
    while (face < 4*nDivisions) {
        faces[3 * face] = (unsigned) nDivisions+1;// center of disc
        faces[3 * face + 1] = (unsigned) startNode;
        if (face <4*nDivisions-1)
            faces[3 * face + 2] = (unsigned)startNode + 1;
        else //last face
            faces[3 * face + 2] = (unsigned)nDivisions+2;//back to beginning

        startNode++;
        face++;
    }
    unsigned size = 3;
    for (unsigned i = 0; i<nTriangles; i++){
        unsigned uvFcae = obj->NewFace(size, faces);
        faces += size;
    }

    // now reorient all points on  the cylidner
    zVec[0] = 0;
    zVec[1] = 0;
    zVec[2] = 1;

    // see https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    double sine,cosine,vec[3];
    double rotationMatrix[3][3],vMatrix[3][3],vMatrixSquared[3][3];

    for (int i = 0;i < 3; i++){
        for (int j = 0;j < 3; j++){
            rotationMatrix[i][j] = 0;
            vMatrix[i][j] = 0;
            vMatrixSquared[i][j] = 0;
        }
    }

    CROSS3D(vec,zVec,axis);
    sine = NORM3D(vec);
    cosine = DOTPROD3D(zVec,axis);
    if (sine < 1e-32){// axis coincides with +z or -z
        if (cosine > 0){
            rotationMatrix[0][0] = 1;
            rotationMatrix[1][1] = 1;
            rotationMatrix[2][2] = 1;
        }
        else {//reverse z orientation
            rotationMatrix[0][0] = 1;
            rotationMatrix[1][1] = -1;
            rotationMatrix[2][2] = -1;
        }
    }
    else {
        vMatrix[0][1] = -vec[2];
        vMatrix[0][2] = vec[1];

        vMatrix[1][0] = vec[2];
        vMatrix[1][2] = -vec[0];

        vMatrix[2][0] = -vec[1];
        vMatrix[2][1] = vec[0];
        for (int i = 0;i < 3; i++){
            for (int j = 0;j < 3; j++){
                for (int k = 0;k < 3; k++)
                    vMatrixSquared[i][j] += vMatrix[i][k]*vMatrix[k][j];
            }
        }
        rotationMatrix[0][0] = rotationMatrix[1][1] =rotationMatrix[2][2] = 1.0;

        for (int i = 0;i < 3; i++){
            for (int j = 0;j < 3; j++){
                rotationMatrix[i][j] += vMatrix[i][j] + vMatrixSquared[i][j]*(1-cosine)/(sine*sine);
            }
        }
    }
    /*
    for (int i = 0;i < 3; i++){
        for (int j = 0;j < 3; j++)
            cout << rotationMatrix[i][j] << " ";
        cout << endl;
    }
    */
    // apply the rotation matrix
    double pt[3];

    for (int node = 0; node < nTriPoints; node++) {
        pt[0] = vertices[node].x;
        pt[1] = vertices[node].y;
        pt[2] = vertices[node].z;

        vertices[node].x = vertices[node].y = vertices[node].z = 0;

        for (int i = 0; i < 3; i++){
            vertices[node].x +=  rotationMatrix[0][i]*pt[i];
            vertices[node].y +=  rotationMatrix[1][i]*pt[i];
            vertices[node].z +=  rotationMatrix[2][i]*pt[i];
        }
        vertices[node].x += startPt[0];
        vertices[node].y += startPt[1];
        vertices[node].z += startPt[2];

    }

    for (int node = 0; node < nTriPoints; node++) {
        vertices[node].x *= scale;
        vertices[node].y *= scale;
        vertices[node].z *= scale;
    }

    obj->SetPointsD((ivbool3d::Point3d*)vertices, nTriPoints);

    return true;
}

#endif


/************************************************************/
void LatticeStructure::computeLatticeStructure(void)
{
    // prologue
#ifdef _WIN32
    int Nx,Ny,Nz;
    double Lx,Ly,Lz,L;
    double offset[3];
    TrussStructure trussStructure;

    InitializeTopOpt(3);
    L = ls_unitCellSizeInModelUnits;
    double radius = getRadiusOfTrussElement(ls_unitCellSizeInModelUnits);
    radius = MAX(radius,L/ls_resolution); // the radius cannot be smaller than the voxel size


    // shrink the boundingBox by the radius value to make sure the corners are captured correctly
    shrinkLatticeBoundingBox(radius);

    Nx = NEARESTINT( (ls_geomBoundingBox[0][1] - ls_geomBoundingBox[0][0])/L) ;
    Ny = NEARESTINT( (ls_geomBoundingBox[1][1] - ls_geomBoundingBox[1][0])/L) ;
    Nz = NEARESTINT( (ls_geomBoundingBox[2][1] - ls_geomBoundingBox[2][0])/L) ;

    Lx = (ls_geomBoundingBox[0][1] - ls_geomBoundingBox[0][0])/Nx;
    Ly = (ls_geomBoundingBox[1][1] - ls_geomBoundingBox[1][0])/Ny;
    Lz = (ls_geomBoundingBox[2][1] - ls_geomBoundingBox[2][0])/Nz;

    //get the template unit cell
    constructLatticeTruss(trussStructure,Lx,Ly,Lz);

    // build  truss structure within bounding box
    TrussStructure completeTrussStructure;
    offset[0] = ls_geomBoundingBox[0][0];
    for (int ix = 0; ix <= Nx; ix++) {
        offset[1] = ls_geomBoundingBox[1][0];
        for (int jy = 0; jy <= Ny; jy++) {
            offset[2] = ls_geomBoundingBox[2][0];
            for (int kz = 0; kz <= Nz; kz++) {
                int node_index_offset=completeTrussStructure.nodes.size();
                for (int i=0;i<trussStructure.nodes.size();i++) {
                    Point3D pt;
                    for (int j=0;j<3;j++) {pt[j]=trussStructure.nodes[i][j]+offset[j];}
                    completeTrussStructure.nodes.push_back(pt);
                }
                for (int i=0;i<trussStructure.trussElements.size();i++) {
                    LineSegment edge;
                    for (int j=0;j<2;j++) {edge[j]=trussStructure.trussElements[i][j]+node_index_offset;}
                    completeTrussStructure.trussElements.push_back(edge);
                }
                offset[2] += Lz;}
            offset[1] += Ly;}
        offset[0] += Lx;
    }

    // compute minimum distance between any two points in unit cell
    // std::cout<<" compute minimum distance between any two points in unit cell "<<std::endl;
    double dx=1e32;
    int number_of_nodes=trussStructure.nodes.size();
    for (int i=0;i<number_of_nodes;i++) {
        for (int j=i+1;j<number_of_nodes;j++) {
            const Point3D& pt1=trussStructure.nodes[i], &pt2=trussStructure.nodes[j];
            double p[3]={pt1[0]-pt2[0],
                         pt1[1]-pt2[1],
                         pt1[2]-pt2[2]};
            double candidate_dx=NORM3D(p);
            if (candidate_dx<dx) {
                dx=candidate_dx;
            }
        }
    }
    dx*=0.5; // different points are guaranteed to be farther apart than this distance

    // 'union find' the nodes that are actually the same point
    // std::cout<<" union find the nodes that are actually the same point "<<std::endl;
    number_of_nodes=completeTrussStructure.nodes.size();
    int *node_map = new int[number_of_nodes]; for (int i=0;i<number_of_nodes;i++) {node_map[i]=i;}
    for (int i=1;i<number_of_nodes;i++) {
        for (int j=0;j<i;j++) {
            const Point3D& pt1=completeTrussStructure.nodes[i], &pt2=completeTrussStructure.nodes[j];
            double p[3]={pt1[0]-pt2[0],
                         pt1[1]-pt2[1],
                         pt1[2]-pt2[2]};
            double distance=NORM3D(p);
            if (distance<dx) {
                node_map[i]=j;
                break;
            }
        }
    }

    // update edges of the complete truss structure with the renumbered nodes
    // std::cout<<" update edges of the complete truss structure with the renumbered nodes "<<std::endl;
    int number_of_trusses=completeTrussStructure.trussElements.size();
    for (int i=0;i<number_of_trusses;i++) {
        LineSegment& edge=completeTrussStructure.trussElements[i];
        for (int j=0;j<2;j++) {
            edge[j]=node_map[edge[j]];
        }
    }
    delete node_map;

    // build a set of edges that are actually used - to remove duplicate edges
    // std::cout<<" build a set of edges that are actually used - to remove duplicate edges "<<std::endl;
    std::unordered_set<std::array<int, 2>,ArrayUtilities<2, int> > truss_edges;
    for (auto edge : completeTrussStructure.trussElements) {
        sort(edge.begin(),edge.end());
        truss_edges.insert(edge);
    }

    // adjacency list for the points of the truss structure - don't include unused nodes
    // std::cout<<" adjacency list for the points of the truss structure - don't include unused nodes "<<std::endl;
    std::unordered_map<int, std::vector<int> > adjacency_list;
    for (auto e : truss_edges) {
        adjacency_list[e[0]].push_back(e[1]);
        adjacency_list[e[1]].push_back(e[0]);}

    // identify closest voxel element for all nodes - skip nodes that are unused
    // std::cout<<" identify closest voxel element for all nodes - skip nodes that are unused "<<std::endl;
    std::unordered_map<int, int> closest_voxel;
    for (auto map_entry : adjacency_list) {
        int p = map_entry.first;
        const Point3D& pt=completeTrussStructure.nodes[p];
        closest_voxel[p]=findElemWithCenter(pt[0],pt[1],pt[2],1.01);
    }

    // identify points that need to move and map them to their interior neighbors
    // std::cout<<" identify points that need to move and map them to their interior neighbors "<<std::endl;
    std::unordered_map<int, std::vector<int> > points_moved;
    for (auto map_entry : adjacency_list) {
        int p = map_entry.first; // p -- point
        if (closest_voxel[p]==-1) {
            for (auto n : adjacency_list[p]) { // n -- neighbor
                if (closest_voxel[n]!=-1) {
                    points_moved[p].push_back(n);
                }
            }
        }
    }

    // relocate points outside so they are inside the voxel mesh
    // std::cout<<" relocate points outside so they are inside the voxel mesh "<<std::endl;
    std::unordered_map<int, std::vector<int> > parent_to_points;
    for (auto map_entry : points_moved) {
        auto p = map_entry.first;
        for (auto n : points_moved[p]) {
            Point3D outside = completeTrussStructure.nodes[p];
            Point3D inside  = completeTrussStructure.nodes[n];
            Point3D midpoint;
            int inside_voxel = closest_voxel[n];
            int midpoint_voxel = -1;

            // binary line search for edge of voxel mesh
            int enough = 10; // just in case
            while (enough && inside_voxel != midpoint_voxel) {
                midpoint={(outside[0]+inside[0])/2, (outside[1]+inside[1])/2, (outside[2]+inside[2])/2};
                midpoint_voxel=findElemWithCenter(midpoint[0],midpoint[1],midpoint[2],2.01);
                if (midpoint_voxel == inside_voxel) {break;}
                if (midpoint_voxel==-1) {
                    outside[0]=midpoint[0];outside[1]=midpoint[1];outside[2]=midpoint[2];}
                else {
                    inside[0]=midpoint[0];inside[1]=midpoint[1];inside[2]=midpoint[2];
                    inside_voxel=midpoint_voxel;
                    midpoint_voxel=-1;
                }
            }

            // determine location of the new point
            // std::cout<<" determine location of the new point "<<std::endl;
            inside = completeTrussStructure.nodes[n];
            double direction[3]={inside[0]-midpoint[0],inside[1]-midpoint[1],inside[2]-midpoint[2]}; // use midpoint and inside point
            double distance = sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
            int new_point_index = n;
            for (int i=0;i<3;i++) {midpoint[i] += radius*direction[i]/distance;}

            // remove the old edge
            // std::cout<<" remove the old edge "<<std::endl;
            std::array<int,2> edge = {n, p};
            sort(edge.begin(),edge.end());
            truss_edges.erase(edge);

            // update truss structure
            // std::cout<<" update truss structure "<<std::endl;
            // push the midpoint to the back of the point list
            completeTrussStructure.nodes.push_back(midpoint);
            new_point_index = completeTrussStructure.nodes.size()-1;
            // record the closest voxel for the newly added point
            closest_voxel[new_point_index] = inside_voxel;
            // reconnect the inside point to the new midpoint
            edge = {n, new_point_index};
            sort(edge.begin(),edge.end());
            truss_edges.insert(edge);
            // add new point to the parent to new point data structure
            parent_to_points[p].push_back(new_point_index);
        }
    }

    // if an outside point was connected to more than one inside point - connect the new points
    // std::cout<<" if an outside point was connected to more than one inside point - connect the new points "<<std::endl;
    for (auto map_entry : parent_to_points) {
        for (int i = 0; i<map_entry.second.size();i++) {
            for (int j = i+1; j<map_entry.second.size();j++) {
                std::array<int,2> edge = {map_entry.second[i], map_entry.second[j]};
                sort(edge.begin(),edge.end());
                // std::cout<<" connecting children of single parent node ("<<edge[0]<<","<<edge[1]<<")"<<std::endl;
                truss_edges.insert(edge);
            }
        }
    }

    // check to see if any two outside points that moved were previously connected - add a truss
    // std::cout<<" check to see if any two outside points that moved were previously connected - add a truss "<<std::endl;
    for (auto map_entry : parent_to_points) {
        auto p = map_entry.first;
        for (auto n : adjacency_list[p]) {
            if (parent_to_points.count(n)) { // connect these points - use the closest pair of child points
                double smallest_distance = DBL_MAX;
                int e1=-1,e2=-1;
                // find the closest two children
                for (auto c1 : parent_to_points[p]) {
                    for (auto c2 : parent_to_points[n]) {
                        const Point3D& pt1=completeTrussStructure.nodes[c1], &pt2=completeTrussStructure.nodes[c2];
                        double p[3]={pt1[0]-pt2[0],pt1[1]-pt2[1],pt1[2]-pt2[2]};
                        double distance=NORM3D(p);
                        if (distance<smallest_distance) {
                            smallest_distance=distance;
                            e1=c1;e2=c2;
                        }
                    }
                }
                std::array<int,2> edge = {e1,e2};
                sort(edge.begin(),edge.end());
                // std::cout<<" connecting children of adjacent parent nodes ("<<edge[0]<<","<<edge[1]<<")"<<std::endl;
                truss_edges.insert(edge);
            }
        }
    }

    // outside points connected to two points that moved (e.g. corners)
    std::unordered_map<int, std::vector<int> > outside_points_to_moved_points;
    for (auto map_entry : adjacency_list) {
        const auto &p = map_entry.first;   // point
        const auto &ns = map_entry.second; // neighbors
        if (parent_to_points.count(p)) continue;
        if (closest_voxel[p] >= 0) continue;
        for (auto n : ns) { // iterate through neighbors
            if (parent_to_points.count(n)) { // did they move
                outside_points_to_moved_points[p].push_back(n);
            }
        }
    }
    for (auto map_entry : outside_points_to_moved_points) {
        const auto &ps = map_entry.second;
        if (ps.size()>1) {
            for (int i=0;i<ps.size();i++) {
                for (int j=i+1;j<ps.size();j++) {
                    // for each pair find the closest two children
                    double smallest_distance = DBL_MAX;
                    int e1=-1,e2=-1;
                    for (auto c1 : parent_to_points[ps[i]]) {
                        for (auto c2 : parent_to_points[ps[j]]) {
                            const Point3D& pt1=completeTrussStructure.nodes[c1], &pt2=completeTrussStructure.nodes[c2];
                            double p[3]={pt1[0]-pt2[0],pt1[1]-pt2[1],pt1[2]-pt2[2]};
                            double distance=NORM3D(p);
                            if (distance<smallest_distance) {
                                smallest_distance=distance;
                                e1=c1;e2=c2;}}}
                    std::array<int,2> edge = {e1,e2};
                    sort(edge.begin(),edge.end());
                    // std::cout<<" connecting children of moved nodes adjacent to a corner ("<<edge[0]<<","<<edge[1]<<")"<<std::endl;
                    truss_edges.insert(edge);
                }
            }
        }
    }

    // rebuild truss structure - remove unused points
    // std::cout<<" rebuild truss structure - remove unused points "<<std::endl;
    ls_TrussStructure.nodes.clear();
    ls_TrussStructure.trussElements.clear();
    std::unordered_map<int, int> old_point_to_new_point;
    std::array<int,2> new_edge;
    for (auto e : truss_edges) {
        for (int i=0;i<2;i++) {
            if (old_point_to_new_point.count(e[i])==0) {
                ls_TrussStructure.nodes.push_back(completeTrussStructure.nodes[e[i]]);
                old_point_to_new_point[e[i]]=ls_TrussStructure.nodes.size()-1;}
            new_edge[i]=old_point_to_new_point[e[i]];
        }
        ls_TrussStructure.trussElements.push_back(new_edge);
    }

#endif
}
/*****************************************************************/
int     LatticeStructure::getLatticeNumberOfTrussEdges(void)
{
    return ls_TrussStructure.trussElements.size();
}
/*****************************************************************/
int     LatticeStructure::getLatticeNumberOfTrussNodes(void)
{
    return ls_TrussStructure.nodes.size();
}
/*****************************************************************/
bool    LatticeStructure::getLatticeNodesOfTrussEdge(int edge,int nodes[2])
{
    if ( (edge < 0) || (edge >= ls_TrussStructure.trussElements.size()))
        return false;

    nodes[0] = ls_TrussStructure.trussElements[edge][0];
    nodes[1] = ls_TrussStructure.trussElements[edge][1];
    return true;
}
/*****************************************************************/
bool    LatticeStructure::getLatticeXYZOfTrussNode(int node,double xyz[3])
{
    if ( (node < 0) || (node >= ls_TrussStructure.nodes.size()))
        return false;

    for (int i =0; i < 3; i++)
        xyz[i] = ls_TrussStructure.nodes[node][i];
    return true;
}
/*****************************************************************/
void LatticeStructure::computeLatticeStructureAsVoxelMesh(bool(*callBackFunction)(double param))
{
    double *pseudoDensity;// update voxel mesh with truss structure

    pseudoDensity = (double *)malloc(getNumElems()*sizeof(double));
    memset(pseudoDensity,0,getNumElems()*sizeof(double));
    auto max_number_of_elements = ls_resolution*ls_resolution*ls_resolution; // max vox elements per truss member, worst case
    int *elements=new int[max_number_of_elements];

    double radius = getRadiusOfTrussElement(ls_unitCellSizeInModelUnits);
    radius = MAX(radius,ls_unitCellSizeInModelUnits/ls_resolution); // the radius cannot be smaller than the voxel size

    computeLatticeStructure();

    // now mark all elements within each truss as 'in'
    int number_of_trusses=ls_TrussStructure.trussElements.size();
    for (int truss = 0;truss<number_of_trusses;truss++) {
        if (!(truss%(number_of_trusses/20))) {
            printf("Percent completed: %.1f%% \n",100.0);
            if (!callBackFunction(1.0*truss/number_of_trusses)) break;
        }
        const LineSegment& edge=ls_TrussStructure.trussElements[truss];
        double p[2][3];
        for (int i=0;i<2;i++) {
            for (int j=0;j<3;j++) {
                p[i][j]=ls_TrussStructure.nodes[edge[i]][j];
            }
        }
        int number_of_elements = getElemsInsideCylinder(p[0],p[1],radius,max_number_of_elements,elements);
        for (int i=0;i<number_of_elements;i++) {
            pseudoDensity[elements[i]]=1.;
        }
    }
    delete elements;

    // deal with surface
    switch (ls_surfaceRetainType) {
    case LATTICE_SURFACE_NONE:
        break;// nothing to do
    case LATTICE_SURFACE_CRITICAL: //critical surfaces
        for (int elem = 0; elem < getNumElems(); elem++) {
            if (getKeepElem(elem)){
                pseudoDensity[elem] = 1;}}
        break;
    case LATTICE_SURFACE_ALL: // all surfaces
        for (int elem = 0; elem < getNumElems(); elem++) {
            if (!isElemInInterior(elem))
                pseudoDensity[elem] = 1;}
        break;
    }

    // epilogue
    setPseudoDensity(pseudoDensity);
    free(pseudoDensity);
    saveTopOptField(1.0,0.5,true);


}
/**********************************************************************/
LatticeStructure::~LatticeStructure() 
{
    ;
}


