#ifndef QUADMESHER_H
#define QUADMESHER_H

#define QUADMESHER_VERBOSE 0
#define REAL double 

#define MAX(a,b) ((a) > (b))? (a):(b)
#define MIN(a,b) ((a) < (b))? (a):(b)

#define MAX_BREP_VERTICES 100
#define MAX_BREP_SEGMENTS 100

#define BREP_LINE 0
#define BREP_ARC 1
#define BREP_CIRCLE 2

#define BREP_CW 0
#define BREP_CCW 1

// following is used for incremental memory allocation
#define NODEBLOCK 1000
#define QUADBLOCK 1000


class QuadMesher {
private:
		// Brep data
		/* 2-D geometry Structure with vertices and segments
		brep_vertices = (2,N) 
		includes end points, centers of arcs and circles, and a single point on a circle

		brep_type = (M)
		0: LINE
		1: ARC
		2: CIRCLE

		brep_segments = (4,M) 
		[startVertex endVertex centerVertex orientation]';

		A. For line segments
		[startVertex endVertex 0 0 0]';

		B. For partial arcs 
		[startVertex endVertex 1 centerVertex orientation]';
		Orientation is 0 if clockwise and 1 if ccw

		C. Circles (note: start and end vertices are same)
		[startVertex startVertex 2 centerVertex orientation]';*/

		int		nBrepVertices;
		REAL	brepVertices[MAX_BREP_VERTICES][2];
		int		nBrepSegments;
		int		brepSegType[MAX_BREP_SEGMENTS];
		int		brepSegments[MAX_BREP_SEGMENTS][4];
		REAL	modelScale;


		int		nNodes;
		REAL	*nodeXY;
		int		nQuads;
		int		*quads;

		int		currentNodeBlockMemory;
		int		currentQuadBlockMemory;

		void		QuadMesher::meshBoundary(REAL meshSize);


public:
				QuadMesher::QuadMesher();
		void	QuadMesher::initializeBrep(void);
		int		QuadMesher::getNumVertices(void);
		int		QuadMesher::getNumSegments(void);
		void	QuadMesher::getVertex(int vertex,REAL pt[2]);
		int		QuadMesher::getSegType(int seg);
		void	QuadMesher::getSegment(int seg,int *type,REAL start[2],REAL end[2],REAL center[2],int *orientation);
		int		QuadMesher::readBrep(FILE *fptr);
		REAL	QuadMesher::lengthOfSegment(int seg);
		REAL	QuadMesher::getModelScale(void);
		int		QuadMesher::mesh(int nElems);
		int		QuadMesher::createLBracket(REAL L, REAL H, REAL R);

		void	QuadMesher::initializeQuadMesh(void);
				QuadMesher::~QuadMesher();

};

#endif