#ifndef COMMON_DATA_STRUCTURES_H
#define COMMON_DATA_STRUCTURES_H
#include <vector>

typedef struct {
	double i;
	double j;
	double k;
}Vec;

typedef struct {
	double x;
	double y;
	double z;
}Pt;

typedef struct{
	Pt v1;
	Pt v2;
	Pt v3;
}Triangle;

typedef struct{
	Vec f1;
	Vec f2;
	Vec f3;
}TriVecField;

typedef struct{
	double f1;
	double f2;
	double f3;
}TriScalarField;

// rVec = end - start
inline Vec Subtract3D_Points(const Pt &start,const Pt &end){
	Vec d;
	d.i  = end.x-start.x;
	d.j  = end.y-start.y;
	d.k  = end.z-start.z;
	return d;
}
// rVec = end -start
inline Vec Subtract3D_Vectors(const Vec &start,const Vec &end){
	Vec d;
	d.i  = end.i-start.i;
	d.i  = end.j-start.j;
	d.i  = end.k-start.k;
	return d;
}
// rPt = start + v
inline Pt Translate3D_Points(const Pt &start,const Vec &v){
	Pt end;
	end.x  = start.x + v.i;
	end.y  = start.y + v.j;
	end.z  = start.z + v.k;
	return end;
}
// dist = sqrt((end.x - start.x)^2 + (end.y - start.y)^2 + (end.z - start.z)^2)
inline double Distance3D_Points(const Pt &start,const Pt &end){
	double dx = end.x - start.x;
	double dy = end.y - start.y;
	double dz = end.z - start.z;
	return(sqrt(dx*dx + dy*dy + dz*dz));
}
// dotprod = v1*v2'
inline double DotProd_Vectors(const Vec &v1,const Vec &v2){
	return (v1.i*v2.i + v1.j*v2.j + v1.k*v2.k);
}
// dotprod = v1 x v2
inline Vec CrossProd_Vectors(const Vec &v1,const Vec &v2){
	Vec dest;
	dest.i=v1.j*v2.k-v1.k*v2.j; 
	dest.j=v1.k*v2.i-v1.i*v2.k; 
	dest.k=v1.i*v2.j-v1.j*v2.i;
	return dest;
}
// sqrt(v*v')
inline double Norm3D_Vectors(const Vec &v){
	return(sqrt(v.i*v.i + v.j*v.j + v.k*v.k));
}
// v/||v||
inline Vec Normalize3D_Vectors(const Vec &v){
	Vec v2 = v;
	double nrmV =  Norm3D_Vectors(v2);
	v2.i /= nrmV; v2.j /= nrmV;v2.k /= nrmV;
	return v2;
}
// rVec = alpha*v
inline Vec MultiplyAlpha3D_Vectors(const Vec &v,const double alpha){
	Vec v2 = v;
	v2.i *= alpha;
	v2.j *= alpha;
	v2.k *= alpha;
	return v2;
}
/**********************************************************************/
inline void convertToPoint(double XYZ[3],Pt &p)
{
	p.x = XYZ[0];
	p.y = XYZ[1];
	p.z = XYZ[2];
}
/**********************************************************************/
inline void convertToVec(double IJK[3],Vec &v)
{
	v.i = IJK[0];
	v.j = IJK[1];
	v.k = IJK[2];
}
/**********************************************************************/
typedef struct{
	double sxx;
	double syy;
	double szz;
	double sxy;
	double sxz;
	double syz;
}StressTensor;

typedef struct{
	double compression_x;
	double compression_y;
	double compression_z;
	double tension_x;
	double tension_y;
	double tension_z;
	double shear_yz;
	double shear_xz;
	double shear_xy;
}StrengthData;

typedef struct{
	double c1,c2,c3,c4,c5,
		c6,c7,c8,c9,c10,c11,c12;
}StressCoefficients;

enum TOPOPT_OBJECTIVES
{
	STIFFNESS, 
	STRENGTH
};
enum TOPOPT_STRESSMODELS
{
	VON_MISES,
	TSAI_WU, 
	HOFFMAN,
	TSAI_HILL
};
enum TOPOPT_PLOTMODES
{
	OBJECTIVE, // compliance or inverse of strength-ratio p-norm
	STRAIN_ENERGY, // compliance
	STRESS, // FailureIndex or vonMises
	OBJECTIVE_HIST,
	STRESS_HIST,
	SENSITIVITY_HIST,
	STRESS_VONMISES
};
/**********************************************************************/
inline void convertToStressTensor(double s[6],StressTensor &s_tensor)
{
	s_tensor.sxx = s[0];
	s_tensor.syy = s[1];
	s_tensor.szz = s[2];
	s_tensor.sxy = s[3];
	s_tensor.sxz = s[4];
	s_tensor.syz = s[5];
}
/**********************************************************************/
inline double PNormVector(vector<double> &vec,double p)
{
	double nrm = 0;
	for (size_t i=0;i<vec.size();i++) nrm += pow(vec[i],p);
	return pow(nrm,1.0/p);
}
/**********************************************************************/
#endif
