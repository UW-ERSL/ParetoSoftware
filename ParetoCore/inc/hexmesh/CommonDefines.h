#ifndef COMMONDEFINES_H
#define COMMONDEFINES_H

#define PARETO_VERSION 2024.02
#define PARETO_BUILD_DATE 4.21

#define MAX_MATERIALS  100
#define MAX_EIGEN_MODES  20
	
#define SUCCESS 1
#define FAILURE 0

#define TOL 1e-12

#define NEARESTINT(x)  ( int(x+0.5) )

//#define COMPILE_GPUCODE
// DEFLATION_METHOD Variables

// Don't change the numbers since project files use this convention
#define DEFLATION_NONE 0
#define DEFLATION_TRANSLATION 1
#define DEFLATION_RIGID 2
#define DEFLATION_STRAIN 3

#define CROSS3D(dest,v1,v2)                      \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOTPROD3D(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define COPYVEC3D(v1,v2) v2[0]=v1[0]; v2[1]=v1[1]; v2[2]=v1[2]; 

#define SUM3D(v1) (v1[0]+v1[1]+v1[2])

#define SUBTRACT3D(dest,v1,v2) dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2]; 

#define PRINT3D(v) printf("%g %g %g\n",v[0],v[1],v[2]); 

#define MEAN3D(dest,v1,v2) dest[0]=(v1[0]+v2[0])/2.; dest[1]=(v1[1]+v2[1])/2.; dest[2]=(v1[2]+v2[2])/2.; 

#define ADD3D(dest,v1,v2) dest[0]=v1[0]+v2[0]; dest[1]=v1[1]+v2[1]; dest[2]=v1[2]+v2[2]; 

#define NORM3D(v1) (sqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]))

#define ABSSUM3D(v1) (fabs(v1[0]) + fabs(v1[1]) + fabs(v1[2]) )

#define MULTIPLY3D(dest,v,alpha) dest[0]= alpha*v[0]; dest[1]= alpha*v[1]; dest[2]= alpha*v[2];


#define DIST3D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+(p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])))





#ifndef _WIN32
inline bool _isnan( double var)
{
	 volatile double x = var;
	 return x!=x;
}
#endif

#endif
