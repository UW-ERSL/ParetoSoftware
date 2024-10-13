#ifndef LATTICE_GENERATOR_H_
#define LATTICE_GENERATOR_H_

#include <algorithm>
#include<iostream>
#include<math.h>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<string.h>
#include<time.h>
#include<limits>
#include <stdio.h>
#include <sstream>
#include "TopOptIsoSurface.h" // import Pareto functions
using namespace std;

typedef std::array<int,2> LineSegment;

struct LatticeGraph {
    vector<Point3D>     nodes;       // Node Coordinates
    vector<LineSegment> struts;      // Nodal connectivity
    Point3D             boxMin;      // bounding Box min
    Point3D             boxMax;      // bounding Box max
    double              modelScale;  //overall length
    vector<int>         parentBody;     // the body within which a node lies (for multiple bodies)
    vector<bool>        strutExists;    // useful for optimization
    vector<double>      radiusOfTrunkStrut;
    vector<double>      lengthOfStrut;  // Length of each strut, useful for FEA

    Point3D     xAxis;             // local axis along strut
    Point3D     yAxis;             // random axis perpendicular to x
    Point3D     zAxis;             //
    // vector<double>      cosXOfStrut;      // Orientation of each strut, useful for FEA
    // vector<double>      cosYOfStrut;      // Orientation of each strut, useful for FEA
    // vector<double>      cosZOfStrut;      // Orientation of each strut, useful for FEA
};

enum LATTICETYPE
{
    LATTICE_BOX =0,
    LATTICE_BCC,
    LATTICE_BCCz,
    LATTICE_BOX_BCC,
    LATTICE_FCC,
    LATTICE_FCCz,
    LATTICE_FCCBCC,
    LATTICE_OCTET,
    LATTICE_PILLARS,
    LATTICE_NO_OVERHANG,//a lattice with no overhangs for Z build
    LATTICE_VORONOI,
};

enum TPMSTYPE
{
    TPMS_BOX = 0,
    P_TYPE,
    D_TYPE,
    G_TYPE,
};


enum LatticeSource
{
    VOXEL =0,
    GRAPH,
    TETMESH,
    TETMESHDUAL,
    HEXMESH
};


struct LatticeParameters{
    LatticeSource   latticeSource;// for constructing; voxel, graph, tetmesh, tetmeshdual, hexmesh
    LATTICETYPE     latticeType;// BOX, BCC, ...
    TPMSTYPE        tpmsType;
    TPMSTYPE        structureType;

    double          latticeSpacingInMM;// for repeated cells
    double          latticeRadiusInMM;
    int             circularDivision;// for approximating a cylinder
    int             tpmsNumber;
    int             tpmsTotal;
    double          tpmsSmoothing;
    double          tpmsThickness;
    int             tpmsEdgeSharpness;
};

class LatticeGenerator : public virtual TopOptIsoSurface
{

private:
    void  createLatticeLocalCoordinateSystem(void);

protected:
    LATTICETYPE                 lg_latticeType;
    LatticeGraph                lg_LatticeGraph;

public:
    LatticeGenerator();


    bool   setLatticeGeneratorXYZOfTrussNode(int node,double xyz[3]);
    bool   setLatticeGeneratorParentBodyOfTrussNode(int node,int parentBody);
    bool   setLatticeGeneratorRadiusOfAllStruts(double radius);
    bool   setLatticeGeneratorradiusOfTrunkStrut(int strut,double radius);
    bool   suppressLatticeGeneratorStrut(int strut);
    bool   unSuppressLatticeGeneratorStrut(int strut);
    bool   isStrutSuppressed(int strut);

    int    getLatticeType(void);
    double getLatticeModelScale();
    int    getLatticeGeneratorNumberOfStruts(void);
    int    getLatticeGeneratorNumberOfNodes(void);
    bool   getLatticeGeneratorNodesOfStrut(int strut,int nodes[2]);
    bool   getLatticeGeneratorXYZOfTrussNode(int node,double xyz[3]);
    int    getLatticeGeneratorParentBodyOfTrussNode(int node);
    double getLatticeGeneratorRadiusOfTrunkStrut(int strut);
    double computeLatticeVolume(void);

    bool   scaleLatticeGraph(double xScale,double yScale,double zScale);

    bool   generateTriangleSoupFromLatticeGraph(const double halfWidth, const char*stlFile);
    bool   computeLatticeGraphFromVoxelMesh(LATTICETYPE latticeType);
    bool   readLatticeGraphFromOffFile(const char*offFile);
    bool   saveLatticeGraphToOffFile(const char*offFile);
};

#endif
