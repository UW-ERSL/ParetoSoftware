#ifndef VOXELIZER_H
#define VOXELIZER_H

#include "STLGeom.h"

#define MAX_VOXELS  (145237418*2)


class Voxelizer: public STLGeom {
private:

    // Voxel Data
    bool vox_verbose;

    bool    vox_includeNodesCloseToBoundary;
    bool    vox_meshThinStructures;

    bool    vox_userDefinedGrid;// the user specifies grid before calling voxelize

    bool    vox_forceEqualSizeGrid;//
    int		vox_maxVoxelsAllowed;
    int		vox_nels[3];
    double	vox_dsize[3];
    int		vox_nElemsInside;
    int		vox_nNodesInside;// includes boundary nodes
    int		*vox_elemMapping; // since grid elems may be removed, this provides a mapping from a grid elem index to a valid elem number
    int		*vox_nodeMapping; // since grid nodes may be removed, this provides a mapping from a grid node index to a valid node number
    int		*nElemsConnectedToNode;// between 0 to 8 for a uniform grid
    int		*vox_partIdOfGridElem; // which part each element belongs to
    int		*vox_closestTriangleToGridNode;// Computed only for boundary nodes
    double  *vox_nodeValues;// can be used for implicit surface isosurface extraction

    void	 pointInTetrahedron(double x[4],double y[4],double z[4],double p[3],int *state);
    void	 pointMembership(double p[3], int *isInside);
    int		 find_word(FILE *filein,char *word);
    int		 getSTLSizeSolidWorks(char *fileName);

    void	 allocateMemory(int nTotalGridNodes, int nTotalGridElems);
    void	 MarkNodesRecursive(double x[3],double y[3],double z[3],double dMin,double edgeLengthDesired,int currentDepth, int *isGridNodeInside);
    void	 closestGridNodeToPoint(double x,double y,double z,double edgeLengthDesired, int *gridNode);
    void	 markGridNodeCloseToBoundary(int part,double edgeLengthDesired,int *isGridNodeInside,bool considerAllTriangles);
    void     includeNodesCloseToTriangles(int part, double dMin, int *gridNodesInside);
    void	 imposeSymmetry(int symmetryIndex,int *gridNodesInside);
    bool	 setGridSize(int nElemsDesired,double maxAspectRatio);
    void	 swap(double *v, int *index, int i, int j);
    void	 quickSort(double *v, int *sortIndex,int s, int n);
    bool     getGridNodeIndices(int gridNode,int gridNodeIndices[3]);
    bool     getLocationOfGridNode(int gridNode, double gridPt[3]);
public:
    Voxelizer();
    void	 enableVoxVerbose(void);
    void	 disableVoxVerbose(void);
    void	 copySTLGeometry(Voxelizer &voxIn);
    void	 transferClosestTriGroups(Voxelizer &voxIn);
    void	 getNumGridElems(int grid[3]);
    void	 getVoxelSize(double size[3]);
    int		 getElemFromGridElem(int gridElem);

    void	 markBoundaryConditions(double LModel);
    int		 findAllNonAccessibleTriangles(int dir);
    bool	 voxelize(int nElemsDesired,double maxAspectRatio,int  symmetryIndex,bool (*callBackFunction)(double fractionCompleted));
    double	 distanceOfGridNodeToSTL(int nx,int ny,int nz,double ptClosest[3]);
    double	 distanceOfPointToSTLTriGroup(double pt[3],int group,double ptClosest[3]);
    double	 distanceOfPointFromLine3D(double *Pt,double *Pt1,double *Pt2, double *PtClosest);
    double	 distanceOfPointToSTL(double pt[3],double ptClosest[3], int *closestTri);
    double	 closestVoxelElementToPoint(double x,double y,double z,int *closestElement,double xietamu[3]);
    double	 closestVoxelNodeToPoint(double x,double y,double z,int *voxelNode);
    bool	 getClosestTriangleToPt(double *pt, int *tri);
    bool     assignValueAtVoxNode(int nx, int ny, int nz, double value);
    bool     assignImplicitFunctionAtGridNodes(double (*function)(double pt[3]));
    bool     assignValueAtAllVoxelBoundaryNodes(double value);
    bool     isElemInside(int elx,int ely,int elz);

    void	 dumpVoxelData(const char *fileName);
    bool	 dumpVoxelDataBinary(const char *fileName);
    bool     createAndDumpIsoSurfaceFromNodalValues(const char *fileName,double tau);
    double	 getVoxelVolume(void);
    void     setVoxelGrid(int nelx,int nely,int nelz);
    void     unsetVoxelGrid();
    void	 setMaxVoxels(int maxVoxels);
    int		 getMaxVoxels(void);
    void     set_includeNodesCloseToBoundary(bool value);
    void     set_ThinStructure(bool value);
    void     setEqualGridSize(void);
    void     unsetEqualGridSize(void);
    ~Voxelizer();
};	

#endif
