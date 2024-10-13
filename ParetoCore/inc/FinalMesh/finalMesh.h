#ifndef FINAL_MESH
#define FINAL_MESH
#include "ivBoolean.h"
#include "boolVector.h"

struct point3df { float x, y, z; };
struct point3dd { double x, y, z; };
struct point2df { float x, y; };


struct ObjectInfo64
{
	unsigned numPoints;
	unsigned numUVPoints;
	unsigned numNormals;
	unsigned numFaces;
	const point3dd*v;
	const point3df*n;
	const point2df*uv;
	const unsigned *sizes;
	const unsigned *fv;
	const unsigned *fuv;
	const unsigned *fn;
	const double *tm;
};

#endif
