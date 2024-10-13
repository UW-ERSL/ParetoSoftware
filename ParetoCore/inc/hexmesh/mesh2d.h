#ifndef _MESH2D_
#define _MESH2D_

/*
   if maximum number of nodes is exceeded or you have more memory 
   you can increase both MAXPSIZE and/or MAXESIZE 
*/

#define MAXPSIZE 5001
#define MAXESIZE 10001


typedef struct mesh2d_data {
	int nLoops;
	int *loopStart;
	int *loopEnd;
	int nPoints;
	double *xPt;
	double *yPt;
	double elementSize;
} MESH2D_DATA;


typedef struct mesh3d_data {
	int nLoops;
	int *loopStart;
	int *loopEnd;
	int nPoints;
	double *xPt;
	double *yPt;
	double *zPt;
	double elementSize;
} MESH3D_DATA;


typedef struct mesh2d_mesh {
	int nNodes;
	int nTriangles;
	double nodeXY[MAXPSIZE][2];
	int nodeMarker[MAXPSIZE]; // marks the boundary number on which node lies
	int triangles[MAXESIZE][3];

} MESH2D_MESH;

typedef struct surf_mesh {
	int nNodes;
	int nTriangles;
	double nodeXYZ[MAXPSIZE][3];
	int nodeMarker[MAXPSIZE]; // marks the boundary number on which node lies
	int triangles[MAXESIZE][3];

} SURF_MESH;


int mesh2D(MESH2D_DATA input,MESH2D_MESH *output);

#endif