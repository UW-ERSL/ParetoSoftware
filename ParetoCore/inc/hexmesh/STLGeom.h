#ifndef STLGEOMROUTINES_H
#define STLGEOMROUTINES_H
#include "IntegerSet.h"
#include <vector>
#include <Eigen/Dense>
#ifdef _WIN32
#include <FinalMesh/finalMesh.h>
#endif

using namespace std;
#define MAX(a,b) (((a) > (b))? (a):(b))
#define MIN(a,b) (((a) < (b))? (a):(b))


#define MAX_TRIANGLES 5000000
#define MAX_TRI_PTS 5000000
#define	MAX_EDGES 20000
#define	MAX_CORNERS 2000
#define	STL_MAX_GROUPS 1000

#define MAX_DEPTH_RECURSIVE_SELECT 5000
 
// MAX intersections of a ray with STLGeom object
#define MAXINTERSECTIONS 10000
#define MAX_STL_PARTS 1000

#define CUTOFF_TRIANGLE_FOR_EDGE_MARKING 10000

#define TOL 1e-12

#define CROSS3D(dest,v1,v2)                      \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOTPROD3D(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define COPYVEC3D(v1,v2) v2[0]=v1[0]; v2[1]=v1[1]; v2[2]=v1[2]; 

#define SUM3D(v1) (v1[0]+v1[1]+v1[2])

#define SUBTRACT3D(dest,v1,v2) dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2]; 

#define PRINT3D(v) cout<<v[0]<<","<<v[1] << ", " << v[2] << endl;

#define MEAN3D(dest,v1,v2) dest[0]=(v1[0]+v2[0])/2.; dest[1]=(v1[1]+v2[1])/2.; dest[2]=(v1[2]+v2[2])/2.; 

#define ADD3D(dest,v1,v2) dest[0]=v1[0]+v2[0]; dest[1]=v1[1]+v2[1]; dest[2]=v1[2]+v2[2]; 

#define NORM3D(v1) (sqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]))

#define MULTIPLY3D(dest,v,alpha) dest[0]= alpha*v[0]; dest[1]= alpha*v[1]; dest[2]= alpha*v[2];

#define SCALE(v,scale) v[0] /=scale; v[1] /=scale; v[2]/=scale;


#define DIST3D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+(p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])))



#define DIST6D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+ (p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])+(p2[3]-p1[3])*(p2[3]-p1[3])+(p2[4]-p1[4])*(p2[4]-p1[4])+  (p2[5]-p1[5])*(p2[5]-p1[5])))

enum SURFACE_TYPE
{
    PLANAR, 
	CYLINDER,
	OTHER
};

typedef struct {double x,y,z;} Point3d;


class STLGeom {

private:
		unsigned int stl_nTesselatedTriangles;

		void	 dumpTriangleTesselationRecursive(double xTri[3],double yTri[3],double zTri[3],FILE *fptr,int maxDepth,double edgeLength);
		void	 dumpTriangle(double xTri[3],double yTri[3],double zTri[3],FILE *fptr);

protected:// inherited classes can access these variables
		bool	stl_verbose;



		// Triangle data
		int		stl_ntriPoints;
		int		stl_nTriangles;
		int		stl_nTriGroups;// for boundary conditions
		int		stl_nTriFaces;// for display
		int		stl_nSharpEdges; // sharp edges of STLGeom, using a specified cutoff angle
		int		stl_nSharpCorners;//deduce from sharp edges
		int		stl_nConcaveEdges;//
		int		stl_nParts;//for assemblies
		bool	stl_symmetry[3];
        bool    stl_segmentModel; // should we segment the model (sometimes not needed)
		

        double	triXYZ[3*MAX_TRI_PTS];// for each STLGeom point, x y z coords (stored as x0,y0,z0,x1,y1,z1,...)
		int		triNodes[3*MAX_TRIANGLES];// for each STLGeom triangle, the 3 nodes
		int		triCorners[MAX_CORNERS];// deduced from STLGeom through heuristics
        int		triEdges[2*MAX_EDGES];// the two nodes that make up the edge, deduced from STLGeom through heuristics
        int		triEdgesParents[2*MAX_EDGES];// the two triangles that make up the edge, deduced from STLGeom through heuristics
		bool	edgeConcavity[MAX_EDGES];// for stress raisers from STLGeom through heuristics
		int		triNeighbors[3*MAX_TRIANGLES];// deduced from STLGeom through connectivity
		bool	triHighlight[MAX_TRIANGLES];// for selecting and highlighting triangles
        int		triGroup[MAX_TRIANGLES];// grouping of triangles,  for example, into planes and triangles
		int		triFace[MAX_TRIANGLES];// grouping of triangles for display		
		int		triPart[MAX_TRIANGLES]; // different parts in assembly
		int		triStartEnd[MAX_STL_PARTS][2];// by default o to stl_nTriangles-1, but can be narrowed down if they are presorted. 

		double	 stl_partArea[MAX_STL_PARTS];
		double	 stl_partVolume[MAX_STL_PARTS];
		double	 stl_partCenter[MAX_STL_PARTS][3];
		double	 stl_smallestCylinderRadius;// keep track of cylinders selected and smallest radius for mesh size estimation


		double	stl_modelScale;
		double	stl_XYZMin[3],stl_XYZMax[3],stl_STLGeomCenter[3];

        int		 find_word(FILE *filein,const char *word);
		int		 findNumCommonElements(int n,int *v1, int *v2);
		
		double	 computeAreaOfTriangle(double x[3],double y[3],double z[3]);
		void	 bubbleSortIncreasing(double *numbers, int *sortIndex,int array_size);
		void	 computeContributionOfTriangle(double x[3],double y[3],double z[3],double *vol,double *area,double center[3]);
		int		 findDuplicatePt(double pt[3],int partNum);
		void	 centerSTLGeomModel(void);
		bool	 readSTLGeomAscii(const char *fileName, int partNum,bool (*callbackFunction)(double fractionCompleted),bool eliminateDuplicateNodes = true);
		bool     readSTLGeomBinary(const char *fileName, int partNum,bool (*callbackFunction)(double fractionCompleted),bool eliminateDuplicateNodes = true);


		void	 findSharpCorners(void);
		bool	 isPointIn2DTriangle(double *A,double *B,double *C,double *P, double *u, double *v);
        void	 assignTrianglesToGroupRecursive(int seedTriangle,double triNormal[3],double cutoffAngleDegrees,int group,int depth);
		bool	 checkForHollowBodies(void);
		int		 findIntersectionPointsWithSTLGeom_ZDirRay(int part,double x,double y,double **intersectionPts, int *intersectingTriangles);
		int		 findIntersectionPointsWithTriangleSet_ZDirRay(int part,double x,double y,IntegerSet &triangles,double **intersectionPts, int *intersectingTriangles);

		

		double	 findRadiusOfCylindricalFace(int faceGroup,double axisDirection[3]);

		int		 rayIntersectTri(double orig[3], double dir[3],double vert0[3], double vert1[3], double vert2[3],
								double *t, double *u, double *v);
		double	 distanceOfPointFromLine3D(double *Pt,double *Pt1,double *Pt2, double *PtClosest);

		
		void	 findLargestPlaneNormal(double planeNormal[3]);
		void	 unhighlightTriangle(int triangle);
		
		void	 dumpTriangleSegmentation(char *fileName);


		void	 findSharpEdgesAndCorners(double creaseAngleInDegrees);
		int		 createLocallyConformingQuadMesh(double pt[3], double meshSize, int numQuadsPerSide,char *fileName);
		
		double	 computeTriGroupArea(int group);
		
		void	 findSymmetry(void);


		
		void	 extractCrossSection(char *fileName,double hSize);
		bool	 doBothNodesBelongToTriangle(int nodes[2],int tri );
		int		 findCommonElements(int n,int *v1, int *v2, int *vCommon);
		
public:
				 STLGeom();

//enabling verbose
		void	 enableSTLVerbose(void);
		void	 disableSTLVerbose(void);
		

// reading and writing an STL or OFF file
        bool	 readSTLGeom(const char *fileName,int useGPU = false,bool assemblyMode = false,bool (*callbackFunction)(double fractionCompleted) = NULL,bool eliminateDuplicateNodes = true);
        bool	 readOffFile(const char *fileName, int partNum);
        bool	 readObjFile(const char *fileName);
        void	 deleteCurrentModel(void);
        bool	 saveAsSTL(const char *stlFileName);
        void	 saveAsObj(char *objFileName);
        bool	 saveAsOff(const char *fileName);
        bool	 savePartsAsOff(const char *fileName,std::vector<int> parts);

//set operations
        int	setTriangleData(int nPoints, int nTri, int nGroups, double *xyz,int *nodes);
        void	 setTriGroup(int tri,int group);
        void     setNumberOfPartsToZero(void);
        void     setLogFilePtr(FILE *logFilePtr);
        void    setModelScale(double scale);
		
//get operations
		int		 getNumTriangles();
		double	 getZHeightOfSTLGeom(void);
		void	 getTriNode(int node,double pt[3]);
		void	 getRandomPtInTri(int tri,double pt[3]);
		double	 getModelScale(void);
		int		 getNumParts();
		int		 getNumTriPoints();
		int		 getTriGroup(int tri);
		void	 getMinAndMaxLengths(double*LMin,double *LMax);
		void	 getBoundingBox(double xyzMin[3],double xyzMax[3]);
		void	 getBoundingBox(double boundingBox[3][2]);
		void	 getBoundingBoxOfPart(int partNo,double boundingBox[3][2]);
		void	 getTriNodes(int tri,int nodes[3]);
		void	 getTriCenter(int tri,double center[3]);
		void	 getTriPts(int tri,double x[3],double y[3],double z[3]);
		int		 getParentPartOfTriangle(int tri);
		int		 getNumTriGroups(void);
		void	 getSTLSymmetry(bool symmetry[3]);
		double	 getAreaOfSTLPart(int part);
		double	 getVolumeOfSTLPart(int part);
		void	 getCenterOfSTLPart(int part,double center[3]);
		double	 getMinimumThicknessOfPart(int part);
		double	 getVolumeOfSTLGeom(double *area);
		void	 getActiveGroups(int activeGroups[STL_MAX_GROUPS]);
		void	 getSTLGeomCenter(double center[3]);
		int		 getRecommendedNumberOfElements(void);
		int		 getNumSharpEdges(void);
        void     getNeighboringTrianglesOfTriangle(int tri,int neighTri[3]);
        bool     getNormalAtPoint(double pt[3],double normal[3]);
		void	 getSharpEdgeNodes(int edge,int nodes[2]);
		void	 getSharpEdgePoints(int edge,double x[],double y[],double z[]);
        void     getPtOnTriangleAt(int tri,double baryCentricCoords[3],double pt[3]);
		void	 getTriangleNormal(double x[3],double y[3],double z[3],double normal[3]);
		void	 getNormalOfTriangle(int tri,double normal[3]);
		double	 getAreaOfTriangle(double pt1[3],double pt2[3],double pt3[3]);
		double	 getAreaOfTriangle(int tri);
		void	 getCenterOfTriangle(int tri,double center[3]);
		int		 getNumberOfConcaveEdges(void);
        int		 findIntersectionPointsWithSTLGeom(int part, double origin[3],double dir[3],double **intersectionPts, int *intersectionTriangles);
        int		 findRayIntersectionSTL( double origin[3],double dir[3],double *tParams);


//geometry operations
        void	 applyRotationMatrix(double R[3][3]);
		void	 rotateSTLGeomAboutX(double angleInDeg);
		void	 rotateSTLGeomAboutY(double angleInDeg);
		void	 rotateSTLGeomAboutZ(double angleInDeg);
		void	 translateSTLGeombyZ(double distance);
		void	 translateSTLGeom(double offset[3]);
		void	 recomputeSTLGeomLimits(void);
		void	 scaleSTLGeomAboutOrigin(double scale);
		void	 scaleSTLGeomAboutOriginAlongZ(double scale);
		void	 scaleSTLGeomAboutOriginAlongY(double scale);
		void	 scaleSTLGeomAboutOriginAlongX(double scale);	
		bool	 createBoundingBoxSTLFile(double boundingBox[3][2],char *fileName);


		void	 precomputePartProperties(void);
        bool     findTriNeighbors(int useGPU);
		bool	 assignPartNumberToTriangles(int startTri, int endTri, int partId);
		void	 findStartEndTrianglesOfParts(void);
		void	 autoAssignPartNumberToTriangles(void);
		void	 incrementTriGroups(void);
		void	 decrementTriGroups(void);
		void	 setNumTriGroups(int nGroups);
		bool	 isGeometrySweepable(double dir[3]);
		bool	 isEdgeConcave(int edge);
        bool     isPointInsidePart(double pt[3],int part);
        void	 snapPointToTriangle(double *pt,int tri,double tolerance);
        void	 snapPointToTriangulation(double *pt,double tolerance);
        void	 snapPointToCorner(double *pt,double tolerance);
        void	 snapPointToEdge(double *pt,double tolerance);
        bool	 isPointCloseToAnSTLEdge(double *pt,double tolerance);
        void	 snapPointToGeometry(double *pt,double tolerance);

// computation support
		int		 findClosestTriangleIntesectingRay(double origin[3],double dir[3],double *tMin);
        int		 findClosestTriangleIntesectingRaySkippingParts(vector<int> partToSkip,double origin[3],double dir[3],double *tMin);
		double	 distanceOfPointToSTLGeomTriGroup(double pt[3],int group,double ptClosest[3]);
		double	 distanceOfPointToSTLGeom(double pt[3],double ptClosest[3],int *triClosest);
		double	 distanceOfPointFromTriangle3D(double *Pt,double *Pt1,double *Pt2,double *Pt3, double *Pt0, double *stu);
        double	 distanceOfPointToTri(double *Pt,int tri);
        double	 computeVolumeOfSTLPart(int part, double *area,double center[3]);

        bool     findExtremeVerticesAlongDirection(double dir[3], double minPt[3],double maxPt[3],double minMaxHeights[2]);


#ifdef _WIN32
        void    RemoveDuplicatePointsFromIVBool(ivbool3d::Object *master, ivbool3d::Object *slave);
        bool    SaveIVBoolToOffFile(ivbool3d::Object*obj, const char*offFile);
        bool    SaveIVBoolToObj(ivbool3d::Object*obj, const char*objFile);
        bool    readAndLoadOffFileToIVBool(const char *fileName, ivbool3d::Object*obj);
        bool    Boolean(const char *offFileA, const char *offFileB, ivbool3d::boolean_mode booleanOperations, const char *resultOffFile);
#endif
// group selection and assignment
		void	 highlightTrianglesRecursive(int seedTriangle,int depth,double triNormal[3],double cutoffAngle,double *cumulativeArea);
		void	 unHighlightTrianglesRecursive(int seedTriangle, int depth, double triNormal[3], double cutoffAngle, double *cumulativeArea);
		int		 highlightTriangleGroup(int group,double *area);
        void     highlightTriangle(int tri);
		void	 resetHighlight(void);
        bool     findCanonicalDirectionOfGroup(int group, Point3d &direction);
		void     computeSmallestRadius(int group);
        bool     isTriangleHighlighted(int tri);
		void	 segmentModel(double creaseAngleInDegrees = 15);
        int      segmentModelAndFindCanonicalDirections(double creaseAngleInDegrees, double directionTol, std::vector<Point3d> &directions);
		void	 dumpTriGroups(void);
		void	 resetGroups(void);

		SURFACE_TYPE   assignHighlightedTrianglesToGroup(int group,double averageNormal[3], double *area,double cylinderAxis[3],double axisPoint[3],double *cylinderRadius);

        bool     generatePointsOnSharpEdges(double spacing, std::vector<Point3d> &points);
        bool     dumpPointCloudOnSTL(const char *fileName, double edgeLength, int nPointsDesired);
		bool	 dumpSTLTesselation(const char *fileName,int maxDepth,double edgeLength);

				 ~STLGeom();
};	

#endif
