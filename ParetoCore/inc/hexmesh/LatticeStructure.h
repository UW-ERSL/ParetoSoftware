#ifndef LATTICE_STRUCTURE_H
#define LATTICE_STRUCTURE_H
#include "ContourExtraction.h"
#include <list>
#include <stdio.h>
#include <algorithm>    // std::sort
#include <vector>
#include <array>
#include <iostream>

#ifdef _WIN32
#include <FinalMesh/finalMesh.h>
#endif
typedef std::array<int,2> LineSegment;

struct TrussStructure {
	vector<Point3D>  nodes;       // Node Coordinates
	vector<LineSegment>  trussElements;      // Nodal connectivity
};

enum LATTICE_TYPE
{
	BOX =0,
    BCC, 
    BCCz, 
	BOX_BCC,
	FCC,
	FCCz,
	FCCBCC,
	OCTET,
	C11,// maximizes stiffness along X
	C12,// maximizes Poisson effect in XY plane
	C44// maximizes shear stiffness in XY plane
};

enum LATTICE_SURFACE_RETAIN
{
	LATTICE_SURFACE_NONE =0,
    LATTICE_SURFACE_CRITICAL, 
	LATTICE_SURFACE_ALL
};


class LatticeStructure: public ContourExtraction {
private:
	LATTICE_TYPE				ls_latticeType;
	LATTICE_SURFACE_RETAIN		ls_surfaceRetainType;
	int							ls_resolution;// how many voxel divisions per edge of lattice unit cell
	double						ls_fillRatio;// between 0 to 1
	double						ls_unitCellSizeInModelUnits;//
	double						ls_geomBoundingBox[3][2];
	double						ls_geometryVolume;

    TrussStructure              ls_TrussStructure;



	void		constructBoxLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructBCCzLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructBoxBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructFCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructFCCBCCLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructFCCzLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructOctetLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructC11LatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructC12LatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructC44LatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
	void		constructLatticeTruss(TrussStructure &trussStructure,double Lx,double Ly,double Lz);
#ifdef _WIN32
    void        RemoveDuplicatePointsFromIVBool(ivbool3d::Object *master, ivbool3d::Object *slave);
    bool        constructCylinderTriangulation(Point3D startPt, Point3D endPt, double radius, double scale, int nDivisions, ivbool3d::Object*obj);
#endif
	void		shrinkTrussStructure( TrussStructure &trussStructure,const double unitCellSize, const double radius);
	void		shrinkLatticeBoundingBox(double radius);
	double		getRadiusOfTrussElement(double cubeLength);



public:
			LatticeStructure();

    int     getLatticeNumberOfTrussEdges(void);
    int     getLatticeNumberOfTrussNodes(void);
    bool    getLatticeNodesOfTrussEdge(int edge,int nodes[2]);
    bool    getLatticeXYZOfTrussNode(int node,double xyz[3]);
	void	getLatticeSpecsAsString(char *latticeSpecs);
	int		getLatticeNumberOfVoxelElementsNeeded(void);
	void	printTrussStructure( TrussStructure &trussStructure);
	void	setLatticeType(LATTICE_TYPE latticeType);
	void	setLatticeSurfaceRetainType(LATTICE_SURFACE_RETAIN latticeSurfaceRetainType);
	void	setLatticeResolution(int latticeResolution);
	void	setLatticeFillRatio(double fillRatio);
	void	setLatticeUnitCellSizeInModelUnits(double unitCellSizeInModelUnits);
	void	setLatticeBoundingBox(double geometryVolume,double boundingBox[3][2]);


    void    computeLatticeStructure(void);
    bool    computeLatticeUnitCellAsOffFile(void);
    void	computeLatticeStructureAsVoxelMesh(bool(*callBackFunction)(double param));



    //saving and reading
    bool    SaveLatticeGraphToOffFile(const char*offFile);
#ifdef _WIN32
    bool    readAndLoadOffFileToIVBool(const char *fileName, ivbool3d::Object*obj);
    bool    SaveIVBoolToOffFile(ivbool3d::Object*obj, double scale,const char*offFile);
    void    FilterIVBoolMesh(ivbool3d::Object* mesh, ivbool3d::Object* obj);
#endif
			~LatticeStructure();
};

#endif
