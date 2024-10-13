#ifndef ISOSURFACE_H
#define ISOSURFACE_H

typedef struct {
   double x,y,z;
} XYZ;

typedef struct {
   XYZ p[8];// the 3 xyz locations at 8 nodes
   XYZ n[8]; //the 3 components of field gradient at 8 nodes
   double val[8];
} GRIDCELL;

typedef struct {
   XYZ p[3];         /* Vertices */
   XYZ n[3]; // xyz component of normals
} TRIANGLE;

#define ABS(x) (x < 0 ? -(x) : (x))

#define CROSS3D(dest,v1,v2)                      \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

// Prototypes
extern void printXYZ(XYZ p);
extern void printTriangle(TRIANGLE t);
extern int PolygoniseCube(GRIDCELL,double,TRIANGLE tri[]);
extern void VertexInterp(double,XYZ,XYZ,double,double, XYZ *xyz);
extern double volumeContributionOfTriangle(TRIANGLE T);



#endif
