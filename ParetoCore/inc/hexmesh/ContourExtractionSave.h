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
        bool        ce_contourExtraction_verbose;

        int         ce_sweepDirection[MAX_MESH_COMPONENTS];// which direction to sweep/loft, 0,1 or 2 for a good direction, else -1
        int         ce_numContoursInComponent[MAX_MESH_COMPONENTS];// 1, 2 or more contours (with possibly internal)
        bool        ce_isContourInternal[MAX_MESH_COMPONENTS][MAX_CONTOURS_PER_COMPONENT];// default: false, for each contour, external or internal
        list<point> ce_contourData[MAX_MESH_COMPONENTS][MAX_CONTOURS_PER_COMPONENT];// x,y,z location of each contour

        list<int>   elemComponentList[MAX_MESH_COMPONENTS];// for each component, the list of elements
        point       centerOfComponent[MAX_MESH_COMPONENTS];
        int         ce_nMeshComponents;

        // the following are used in discarding small mesh components, and for comparing symmetry of components
        double      ce_componentError;// a small value, typically around 0.002
        int         ce_componentErrorScaling;// the above can be scaled by the user
        int         ce_componentSizeError;// number of elements in component that can be discarded, determined from above, and  mesh size, etc


          // the two variable is used for symmetry detection
        int         ce_symmetryErrorScaling; //used in conjunction with ce_componentSizeError

        // the following is used in determining whether to merge or split consecutive layers for sweeping/lofting
        double      ce_layerError;// a small value, typically around 0.002
        int         ce_layerErrorScaling;// the above can be scaled by the user

        // the following is used to simplify and smoothen contours
        double      ce_contourError;// a small value, typically around 0.05, the mesh edge length is used as reference
        int         ce_contourErrorScaling;// the above can be scaled by the user

        bool        ce_exploitSymmetry;



		// functions
		double	  distanceBetweenPoints(point pt1,point pt2);

		int		  getPixelValueXY(int currentComponent,int pX,int pY,int nz);
		int		  getPixelValueYZ(int currentComponent,int nX,int pY,int pZ);
		int		  getPixelValueZX(int currentComponent,int pX,int ny,int pZ);

        void      generateContour(int component, int contour, int relativeOffset, int gridIndices[3]);
		
		void	  generateContourXY(int component,int contour,int startX, int startY, int nz );
		void	  generateContourYZ(int component,int contour,int nx, int startY, int startZ );
		void	  generateContourZX(int component,int contour,int startX, int ny, int startZ );

		void	  offsetContourXY(int component,int contour,int relativeOffset);
		void	  offsetContourYZ(int component,int contour,int relativeOffset);
		void	  offsetContourZX(int component,int contour,int relativeOffset);

        bool      findSymmetricComponent(int component);

        void      computeMassPropertiesOfContour(int component, int contour, double center[3], double principalAxis[2][2]);
        void	  simplifyContour(int component,int contour);
        bool	  imposeSymmetryOnContour(int component, int contour);
		
		void	  smoothenContour(int component,int contour);
		void	  smoothenContourAreaPreserving(int component,int contour,int projectionDir);
        void	  expandOrContractContour(int component,int contour,double relativeValue = 0.25);
		double	  areaOfTriangle(int projectionDir,point p1,point p2,point p3);
		int       getComponentOfGridElement(int pX,int pY,int nz);

		//For lofts
		void	  offsetLoftContours(int component);

        void	 computeMeanStdDev(int *x,int start,int end,double *mean,double *stddev);
		void	 computeMeanStdDev(double *x,int start,int end,double *mean,double *stddev);
		bool	 areMeshLayersConnectedAlongDir(int component,int dir);
        bool	 splitIslands(int component,int dir);
        bool	 splitIntoLayers(int component,int dir);
        int      getDepthOfComponentAlongDir(int component,int dir);
        void	 estimateNumberOfContoursInMeshComponent(int component, int dir, int *nContours, int *extrudeThickness);
        bool	 analyzeMeshComponent(int component,int *numberOfContours,int *contoursStartIndex, int *preferredDirection);
        bool	 findBoundaryElementOfMeshComponent(int component, int sweepDirection, int gridIndices[3]);
		
		void	 findMeshExtents(int component, double extents[2]);
		
        void     initializeContours(void);
        void     printComponentData(void);
        void     reorientContours(void);
        void     printBasicContourData(void);
        double   getAreaEnclosedByContour(int component, int contour);
        void     printContourPoints(int component, int contour);
        void	 dumpContourData(char *fileName);
        bool	 findConnectedComponentsOfMesh(bool reset);
        bool	 findConnectedComponentsOfMesh_Material(bool reset);
        int      findOptimalDirection(int component, int *nContoursEstimated);
        void     findTopologyOfComponent(int component, int sweepDirection);


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
    void	 generateContourOfMeshComponents(int relativeComponentError = 4,
                                             int relativeLayerError = 4,
                                             int relativeContourError = 4,
                                             int nSmootheningCycles = 5,
                                             int relativeOffset = 5);

		     ~ContourExtraction();
};


#endif
