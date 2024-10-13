#ifndef CONTOUR_EXTRACTION_H
#define CONTOUR_EXTRACTION_H
#include <SIMP.h>
#include <list>


#define MAX_MESH_COMPONENTS 100
#define MAX_CONTOURS_PER_COMPONENT 20

typedef struct  {
	double x;
	double y;
	double z;
} point;// an actual point in the mesh


class ContourExtraction: public  SIMP   {
	// for tracing contours after topology optimization
private:




        int         ce_sweepDirection[MAX_MESH_COMPONENTS];// which direction to sweep/loft, 0,1 or 2 for a good direction, else -1
        int         ce_numContoursInComponent[MAX_MESH_COMPONENTS];// 1 for sweep, greater than 1 for loft
        list<point> ce_contourData[MAX_MESH_COMPONENTS][MAX_CONTOURS_PER_COMPONENT];// x,y,z location of each contour
        list<point> ce_guideCurve[MAX_MESH_COMPONENTS];//  the guide curve (as a collection of of points)

        list<int>   elemComponentList[MAX_MESH_COMPONENTS];// for each component, the list of elements
        point       centerOfComponent[MAX_MESH_COMPONENTS];


        bool        ce_contourExtraction_verbose;
        int         ce_nMeshComponents;
        double      ce_allowableErrorBetweenLayersForSweeping;
        double      ce_discardComponentsCutoff;
        bool        ce_exploitSymmetry;



		// functions
		double	  distanceBetweenPoints(point pt1,point pt2);

		int		  getPixelValueXY(int currentComponent,int pX,int pY,int nz);
		int		  getPixelValueYZ(int currentComponent,int nX,int pY,int pZ);
		int		  getPixelValueZX(int currentComponent,int pX,int ny,int pZ);

		
		void	  generateContourXY(int component,int contour,int startX, int startY, int nz );
		void	  generateContourYZ(int component,int contour,int nx, int startY, int startZ );
		void	  generateContourZX(int component,int contour,int startX, int ny, int startZ );

		void	  offsetContourXY(int component,int contour,int relativeOffset);
		void	  offsetContourYZ(int component,int contour,int relativeOffset);
		void	  offsetContourZX(int component,int contour,int relativeOffset);

		

        void      computeMassPropertiesOfContour(int component, int contour, double center[3], double principalAxis[2][2]);
		void	  simplifyContour(int component,int contour,double tolerance);
        bool	  imposeSymmetryOnContour(int component,int contour,double tolerance);
		void	  shiftSingleContourToCenter(int component);
		
		void	  snapToLinesArcs(int component,int contour);

		void	  smoothenContour(int component,int contour);
		void	  smoothenContourAreaPreserving(int component,int contour,int projectionDir);
        void	  expandOrContractContour(int component,int contour,double relativeValue = 0.25);
		double	  areaOfTriangle(int projectionDir,point p1,point p2,point p3);
		int       getComponentOfGridElement(int pX,int pY,int nz);

		//For lofts
		void	  checkDuplicateContours(int component);
		void	  offsetLoftContours(int component);
		void	  createGuideCurve(int component);

		void	 computeMeanStdDev(int *x,int start,int end,double *mean,double *stddev);
		void	 computeMeanStdDev(double *x,int start,int end,double *mean,double *stddev);
		bool	 areMeshLayersConnectedAlongDir(int component,int dir);
        bool	 splitIslands(int component,int dir);
        bool	 splitMeshComponent(int component,int dir);
        int      getDepthOfComponentAlongDir(int component,int dir);
        void	 estimateNumberOfContoursInMeshComponent(int component, int dir, int *contoursLayerIndex, int *nContours, int *extrudeThickness);
		bool	 analyzeMeshComponent(int component,int *numberOfContours,int *contoursStartIndex, int *preferredDirection);
		bool	 splitMeshComponentUsingLayers(int component,int preferredDirection);
		bool	 splitMeshComponentUsingConnectivity(int component);
		bool	 findBoundaryElementOfMeshComponent(int component,int sweepDirection,int contourStartLocation,int gridIndices[3]);
		
		void	 findMeshExtents(int component, double extents[2]);
		
        void     initializeContours(void);
        void     printComponentData(void);
        void     reorientContours(void);
        void     printBasicContourData(void);
        double   getAreaEnclosedByContour(int component, int contour);
        void     printContourPoints(int component, int contour);
        void	 dumpContourData(char *fileName);
        bool	 findConnectedComponentsOfMesh(bool reset);
        bool	 findConnectedSingleLargestComponentOfMesh(bool reset,double cutoff = 0.5);
        bool	 findConnectedComponentsOfMesh_Material(bool reset);


        void    copyContourWithSymmetry(int fromComponentA, int toComponentB, int symmetryIndex);
        void    copyContourWithAxiSymmetry(int fromComponentA, int toComponentB, double rotationAngle);
        bool    checkForPlanarSymmetry(int componentA, int componentB, int symmetryIndex);
        bool    checkForAxiSymmetry(int componentA, int componentB, int axiSymmetryIndex, double *rotationAngle);
        bool    mapContourToMaster(int component,int slaveContour,int masterContour);
        double  findClosestPtOnContour(int component, int contour, point inputPt, point &closestPt);
        double  distanceOfPointFromLine3D(point inputPt,point linePt1,point linePt2,point &closestPt);

protected:

public:
			 ContourExtraction();
    int		 getNumberOfMeshComponents(void);
    int		 getNumberOfContoursInComponent(int component);
    int		 getMeshComponentOfElem(int elem);

    bool     generateSweptSolidForSingleContour(int component, string offFileName);
    void	 generateContourOfMeshComponents(int generationTol = 2, int relativeSimplificationTol = 2, int nSmootheningCycles = 5, int relativeOffset = 5);

		     ~ContourExtraction();
};


#endif
